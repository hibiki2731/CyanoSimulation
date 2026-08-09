#include "pch.h"
#include "CyanoSimulator.h"
#include "Math.h"
#include "SpriteComponent.h"
#include "input.h"
#include "Random.h"
#include "timer.h"
#include "RWStructuredBuffer.h"
#include "Scene.h"
#include "Game.h"
#include "Graphic.h"
#include "AssetManager.h"
#include "DescriptorHeap.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "CyanoCalculator.h"

const int CyanoSimulator::CELL_SIZE = 10;
const float CyanoSimulator::PIXEL_AREA_WIDTH = Graphic::ClientWidth * 0.5f;
const float CyanoSimulator::PIXEL_AREA_HEIGHT = Graphic::ClientWidth * 0.5f;

const int CyanoSimulator::GRID_WIDTH =  PIXEL_AREA_WIDTH / CELL_SIZE;
const int CyanoSimulator::GRID_HEIGHT = PIXEL_AREA_HEIGHT / CELL_SIZE;



std::vector<float> vertices = {
	0.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.0f, 1.0f,
	1.0f, 0.0f, 1.0f, 0.0f,
	1.0f, 1.0f, 1.0f, 1.0f,
};

std::vector<UINT16> indices = {
	0, 1, 2,
	2, 1, 3
};

CyanoSimulator::CyanoSimulator(Scene& scene):
	Actor(scene),
	mGraphic(scene.getGame().getGraphic()),
	mCommandList(*mGraphic.getCommandList()),
	mAssetManager(scene.getGame().getAssetManager()),
	mDescriptorHeap(mGraphic.getDescriptorHeap())
{
	//パラメータの初期化
	mInteractionIntensity = -4.0f;
	mInteractionRange = 0.05f;
	mPecletNumber = 5.0f;
	mCyanoSpeed = 1.0f;
	mCyanoLength = 1.0f;
	mDeltaT = 0.001f;
	mDeltaTSqrt = sqrtf(mDeltaT);
	mAreaWidth = 2.0f;
	mAreaHeight = 10.0f;
	mPixelParamRatio = PIXEL_AREA_WIDTH / mAreaWidth;

	//セルの数だけ確保
	mCellHeads.resize(GRID_WIDTH * GRID_HEIGHT);
	for (auto& head : mCellHeads) head = -1;

	//バッファーの初期化
	//initBuffer(*scene.getGame().getGraphic().getDevice());
	mCalculator = std::make_unique<CyanoCalculator>(scene.getGame().getGraphic(), MaxPointNum);


}

void CyanoSimulator::inputActor()
{
	if (isKeyJustPressed('I'))
		addCyano(XMFLOAT4(PIXEL_AREA_WIDTH / 2.0f, PIXEL_AREA_HEIGHT / 2.0f, 0.0f, 1.0f), mCyanoLength, mCyanoSpeed);

	if (isKeyJustPressed('O'))
		addCyanos(100);

	if (isKeyJustPressed('P'))
		addCyanos(1000);

	if (isKeyJustPressed('@'))
		addCyanos(1440000);
}

void CyanoSimulator::endProcessActor()
{
	mDescriptorHeap.deleteRange(*mDescRange);
}

void CyanoSimulator::updateActor()
{
	if (!adjustUpdateRate()) return;

	mCalculator->startCalculation(mPoints_pos);
	//updateAngle();
	//createHead();
	//copyPointsToGPU();
}

void CyanoSimulator::draw() {
	return;

	mGraphic.setRenderType(Graphic::RENDER_CYANO);

	//頂点をセット
	mCommandList.IASetVertexBuffers(0, 1, &mVertexBuffer->getView());

	//ディスクリプタヒープをディスクリプタテーブルにセット
	mCommandList.SetGraphicsRoot32BitConstants(0, 3, &mRenderDesc, 0);
	mCommandList.SetGraphicsRootDescriptorTable(1, mDescriptorHeap.getGPUHandle(mDescRange->getIndex(0)));
	mCommandList.SetGraphicsRootDescriptorTable(2, mDescriptorHeap.getGPUHandle(mDescRange->getIndex(1)));
	//描画。インデックスを使用
	mCommandList.IASetIndexBuffer(&mIndexBuffer->getView());
	mCommandList.DrawIndexedInstanced(indices.size(), mPoints_pos.size(), 0, 0, 0);

}

void CyanoSimulator::addCyano(const XMFLOAT4& headPos, float length, float speed)
{
	//個体のサイズ
	const int size = static_cast<int>(length / (speed * mDeltaT));
	//個体の点配列の先頭インデックス
	int beginIdx = mPoints_pos.size();
	mIndivisual_beginPointIdx.push_back(beginIdx);
	mIndivisual_headPointIdx.push_back(beginIdx + size - 1);
	mIndivisual_size.push_back(size);
	//個体の先頭の角度
	mIndivisual_angularVelocity.push_back(0.0f);
	mIndivisual_speed.push_back(speed);

	//点配列への追加
	//仮に真横へ伸びるようにする
	mPoints_pos.resize(mPoints_pos.size() + size);
	mPoints_angle.resize(mPoints_angle.size() + size);
	mPoints_sprites.resize(mPoints_sprites.size() + size);
	mCellNext.resize(mCellNext.size() + size);
	mCellPrev.resize(mCellPrev.size() + size);
	const float angle = Random::normalDist(0.0f, 6.283184f);
	for (int i = 0; i < size; i++) {
		//点の位置を算出
		auto pos = headPos;
		mPoints_pos[beginIdx + i] = pos;
		mPoints_angle[beginIdx + i] = angle;

		//グリッドに追加
		addCell(pos, beginIdx + i);

	}

}

//何秒に一回更新するのか
bool CyanoSimulator::adjustUpdateRate()
{
	mUpdateTimer += deltaTime;
	if (mUpdateTimer > mUpdateInterval) {
		mUpdateTimer = mUpdateTimer - mUpdateInterval;
		return true;
	}

	return false;
}

void CyanoSimulator::addCell(const XMFLOAT4& pos, int idx)
{
	//セル番号を計算
	const int cellIdx = calcCellIdx(pos);

	int oldHead = -1;
	int it = mCellHeads[cellIdx];
	if (it != -1) oldHead = it;

	mCellNext[idx] = oldHead;
	mCellPrev[idx] = -1;
	if (oldHead != -1) mCellPrev[oldHead] = idx;

	mCellHeads[cellIdx] = idx;
}

void CyanoSimulator::deleteCell(int idx)
{
	const auto pos = mPoints_pos[idx];
	const int cellIdx = calcCellIdx(pos);

	int prev = mCellPrev[idx];
	int next = mCellNext[idx];

	//自身が先頭かどうか
	if (prev != -1) mCellNext[prev] = next;	//先頭じゃない場合
	else mCellHeads[cellIdx] = next;		//先頭の場合

	//自身が最後尾かどうか
	if (next != -1) mCellPrev[next] = prev;	//最後尾じゃない場合


}

void CyanoSimulator::createHead()
{
	for (int indivisualIdx = 0; indivisualIdx < mIndivisual_headPointIdx.size(); indivisualIdx++) {
		const int preHeadIdx = mIndivisual_headPointIdx[indivisualIdx];
		const int newHeadIdx = preHeadIdx + 1 >= mIndivisual_beginPointIdx[indivisualIdx] + mIndivisual_size[indivisualIdx] ? mIndivisual_beginPointIdx[indivisualIdx] : preHeadIdx + 1;
		const XMFLOAT4& preHeadPos = mPoints_pos[preHeadIdx];
		const XMVECTOR preHeadVec = XMLoadFloat4(&preHeadPos);
		const float& speed = mIndivisual_speed[indivisualIdx];
		const float& angle = mPoints_angle[newHeadIdx];

		//角度方向へ移動
		XMMATRIX rotX = XMMatrixRotationZ(angle);
		XMVECTOR xVec = XMVectorSet(1.0f, 0.0f, 0.0f, 1.0f);
		XMVECTOR newHeadVec = XMVectorAdd(XMVectorScale(XMVector2TransformNormal(xVec, rotX), speed * mDeltaT * mPixelParamRatio), preHeadVec);

		//壁との衝突反転
		XMVECTOR mod = XMVectorSet(PIXEL_AREA_WIDTH, PIXEL_AREA_HEIGHT, 1.0f, 1.0f);
		XMVECTOR newHeadVecForMod = XMVectorAdd(newHeadVec, XMVectorSet(PIXEL_AREA_WIDTH, PIXEL_AREA_HEIGHT, 0.0f, 0.0f));
		XMVECTOR correctedHeadVec = XMVectorMod(newHeadVecForMod, mod);


		//点の更新
		//セルの変更

		//新たな角度の算出
		const float newAngle = calcDeltaHeadAngle(preHeadVec, correctedHeadVec, angle);
		mPoints_angle[newHeadIdx] = newAngle;

		//最後尾をセルから削除
		deleteCell(newHeadIdx);
		//点の更新
		XMFLOAT4 completedNewHeadPos;
		XMStoreFloat4(&completedNewHeadPos, correctedHeadVec);
		mIndivisual_headPointIdx[indivisualIdx] = newHeadIdx;
		mPoints_pos[newHeadIdx] = completedNewHeadPos;

		//新たな点をセルに追加
		addCell(completedNewHeadPos, newHeadIdx);

	}
	
}

void CyanoSimulator::copyPointsToGPU()
{
	auto upload = mUploadBuffer->getBufferOnCPU();
	memcpy(upload, mPoints_pos.data(), mPoints_pos.size() * sizeof(XMFLOAT4));
}

int CyanoSimulator::calcCellIdx(const XMFLOAT4& pos)
{
	int cellIdx = static_cast<int>(pos.x) / CELL_SIZE + static_cast<int>(pos.y) / CELL_SIZE * GRID_WIDTH;

	return cellIdx;
}

int CyanoSimulator::wrapCellIdx(const int cellX, const int cellY)
{
	const int wrappedCellX = (cellX % GRID_WIDTH + GRID_WIDTH) % GRID_WIDTH;
	const int wrappedCellY = (cellY % GRID_HEIGHT + GRID_HEIGHT) % GRID_HEIGHT;
	return wrappedCellX + wrappedCellY * GRID_WIDTH;
}

void CyanoSimulator::applyParamaterToPicselScale()
{
}

void CyanoSimulator::addCyanos(const int num)
{
	for (int i = 0; i < num; i++)
		addCyano(XMFLOAT4(PIXEL_AREA_WIDTH * 0.5f, PIXEL_AREA_HEIGHT * 0.5f , 0.0f, 1.0f), mCyanoLength, mCyanoSpeed);
}

bool CyanoSimulator::isNearWall(const int cellIdx)
{
	//画面上端のセルに所属している場合
	if (cellIdx < GRID_WIDTH) return true;

	//画面下端のセルに所属している場合
	if (cellIdx >= (GRID_WIDTH) * (GRID_HEIGHT - 1)) return true;

	//画面の右端のセルに所属している場合
	if (cellIdx % GRID_WIDTH == GRID_WIDTH - 1) return true;

	//画面の左端のセルに所属している場合
	if (cellIdx % GRID_WIDTH == 0) return true;

	return false;
}

XMVECTOR CyanoSimulator::calcWallHit(const XMFLOAT4& preHeadPos, FXMVECTOR newHeadVec, const float speed)
{

	//点が画面端にあるかを判定
	const int cellIdx = calcCellIdx(preHeadPos);
	if (!isNearWall(cellIdx)) return newHeadVec;

	//clampで画面外に以下ないよう調整
	XMVECTOR min = XMVectorSet(speed, speed, 0.0f, 0.0f);
	XMVECTOR max = XMVectorSet(PIXEL_AREA_WIDTH - speed, PIXEL_AREA_HEIGHT - speed, 0.0f, 0.0f);

	XMVECTOR correctedNewVec = XMVectorClamp(newHeadVec, min, max);

	return correctedNewVec;

}

//角度の更新
constexpr float ROOT2 = 1.41421356;

void CyanoSimulator::updateAngle()
{
	const float noiseIntensity = ROOT2 / mPecletNumber * mDeltaTSqrt;
	for (int indivisualIdx = 0; indivisualIdx < mIndivisual_headPointIdx.size(); indivisualIdx++) {
		const int preHeadIdx = mIndivisual_headPointIdx[indivisualIdx];
		const int newHeadIdx = preHeadIdx + 1 >= mIndivisual_beginPointIdx[indivisualIdx] + mIndivisual_size[indivisualIdx] ? mIndivisual_beginPointIdx[indivisualIdx] : preHeadIdx + 1;
		//角度の変位を計算
		const float preTheta = mPoints_angle[preHeadIdx];
		const float preOmega = mIndivisual_angularVelocity[indivisualIdx];
		const float deltaTheta = mDeltaT * (preOmega - mInteractionIntensity * calcInteractionValue(indivisualIdx, mPoints_pos[preHeadIdx], preTheta));

		//各速度の変位を計算
		const float noise = noiseIntensity * Random::normalDist(0.0f, 1.0f);
		const float deltaOmega = -deltaTheta + noise;
		
		//角度、角速度を更新
		mPoints_angle[newHeadIdx] = preTheta + deltaTheta;
		mIndivisual_angularVelocity[indivisualIdx] = preOmega + deltaOmega;

	}
}

float CyanoSimulator::calcInteractionValue(const int indivisualIdx, const XMFLOAT4& basePos, const float baseAngle)
{
	const int cellX = static_cast<int>(basePos.x / CELL_SIZE);
	const int cellY = static_cast<int>(basePos.y / CELL_SIZE);
	const int selfBeginIdx = mIndivisual_beginPointIdx[indivisualIdx];
	const int selfSize = mIndivisual_size[indivisualIdx];
	InteractParamater interactParam;
	//自身のセルを中心とした3×3の範囲に存在する点の影響を計算
	for (int dy = -1; dy <= 1; dy++) {
		for (int dx = -1; dx <= 1; dx++) {
			const int neighberIdx = wrapCellIdx(cellX + dx, cellY + dy);
			interactParam = calcInteractInCell(selfBeginIdx, selfSize, neighberIdx, interactParam, basePos, baseAngle);
		}
	}

	if (interactParam.interactNum == 0) return 0.0f;

	return interactParam.interactValue / interactParam.interactNum;
}

CyanoSimulator::InteractParamater CyanoSimulator::calcInteractInCell(const int selfBeginIdx, const int selfSize, const int cellIdx, const CyanoSimulator::InteractParamater& refParam, const XMFLOAT4& basePos, const float baseAngle)
{
	int pointIdx = mCellHeads[cellIdx];
	InteractParamater newParam = refParam;
	while (pointIdx >= 0) {
		if (pointIdx >= selfBeginIdx && pointIdx < selfBeginIdx + selfSize) {
			pointIdx = mCellPrev[pointIdx];
			continue;
		}

		const XMFLOAT4& otherPos = mPoints_pos[pointIdx];

		const float distance = Math::distance(basePos, otherPos);

		if (distance < mInteractionRange * mPixelParamRatio) {
			const float otherAngle = mPoints_angle[pointIdx];

			newParam.interactValue += -sinf(baseAngle - otherAngle);
			newParam.interactNum++;
		}

		//点のインデックスを進める
		pointIdx = mCellPrev[pointIdx];
	}

	return newParam;
}

float CyanoSimulator::calcDeltaHeadAngle(FXMVECTOR preHeadVec, FXMVECTOR newHeadVec, float preAngle)
{
	//移動方向ベクトルを算出
	XMVECTOR moveVec = XMVectorSubtract(newHeadVec, preHeadVec);

	//moveVecの大きさが0に近い場合、マスク処理を行う
	XMVECTOR lenSq = XMVector2LengthSq(moveVec);
	//非常に小さな値epsilonを定義
	XMVECTOR epsilon = XMVectorReplicate(0.0001f);
	//lenSq < epsilonの場合、maskを全ビット1にする
	XMVECTOR mask = XMVectorLess(lenSq, epsilon);

	//現在の角度
	XMVECTOR currentAngle = XMVectorSet(preAngle, 0.0f, 0.0f, 0.0f);

	//moveVecの大きさが0に小さい場合の角度を取得
	XMVECTOR rotatedAngle = XMVectorSet(preAngle + XM_PIDIV2, 0.0f, 0.0f, 0.0f);

	//マスクの値から、出力する角度を選択
	XMVECTOR result = XMVectorSelect(currentAngle, rotatedAngle, mask);

	return XMVectorGetX(result);


}

void CyanoSimulator::initBuffer(ID3D12Device& device)
{
	//GPUで更新する用のバッファ
	mUploadBuffer = std::make_unique<RWStructuredBuffer>(device, sizeof(RenderData), MaxPointNum);

	//スプライト用バッファ
	VertexBufferDescription vertexDesc = { 4, 4 };
	mVertexBuffer = std::make_unique<VertexBuffer>(device, vertexDesc, vertices);

	mIndexBuffer = std::make_unique<IndexBuffer>(device, indices);

	//テクスチャの取得
	mTexture = mAssetManager.getShaderResource("assets/picture/white.png");

	//ディスクリプタヒープに登録
	mDescRange = mDescriptorHeap.allocate(NumSlots(2));
	mDescriptorHeap.addUAV(*mUploadBuffer.get(), mDescRange->getIndex(0));
	mDescriptorHeap.addTextureView(*mTexture, mDescRange->getIndex(1));

	mRenderDesc.cyanoSize = mCyanoSpeed * mDeltaT * mPixelParamRatio;
	mRenderDesc.WindowSize = {Graphic::ClientWidth, Graphic::ClientHeight};

}

