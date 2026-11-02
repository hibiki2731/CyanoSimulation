
#include "CyanoSimulator.h"
#include "Utility/Input/input.h"
#include "Utility/Math/Random.h"
#include "Utility/Math/Math.h"
#include "Utility/Time/timer.h"
#include "Memory/StructuredBuffer.h"
#include "Scene/Scene.h"
#include "GameLoopCore/Game.h"
#include "Graphic/Core/Graphic.h"
#include "Memory/AssetManager.h"
#include "Memory/DescriptorHeap.h"
#include "Memory/VertexBuffer.h"
#include "Memory/IndexBuffer.h"
#include "CyanoCalculator.h"
#include "Memory/IRWStructuredBuffer.h"

#include "Builder/EngineResourceFactory.h"
const UINT CyanoSimulator::CELL_SIZE = 1;
const float CyanoSimulator::PIXEL_AREA_WIDTH = Graphic::ClientHeight;
const float CyanoSimulator::PIXEL_AREA_HEIGHT = Graphic::ClientHeight;

const UINT CyanoSimulator::GRID_WIDTH =  static_cast<int>(PIXEL_AREA_WIDTH / CELL_SIZE);
const UINT CyanoSimulator::GRID_HEIGHT = static_cast<int>(PIXEL_AREA_HEIGHT / CELL_SIZE);



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
	mCommandList(*mGraphic.getGraphicsCommandList()),
	mAssetManager(scene.getGame().getAssetManager()),
	mDescriptorHeap(mGraphic.getDescriptorHeap())
{
	//パラメータの初期化
	mInteractionIntensity = 4.0f;
	mPecletNumber = 1.0f;
	mCyanoSpeed = 1.0f;
	mCyanoLength = 1.0f;
	//mDeltaT = 0.001f;
	mDeltaT = 0.1f;
	mDeltaTSqrt = sqrtf(mDeltaT);
	mAreaWidth = 10.0f;
	mAreaHeight = 1.0f;
	mPixelParamRatio = PIXEL_AREA_WIDTH / mAreaWidth;
	mInteractionRange = mCyanoSpeed * mDeltaT * 2.0f;

	mRenderDesc.cyanoSize = mCyanoSpeed * mDeltaT * mPixelParamRatio;
	mRenderDesc.WindowSize = {Graphic::ClientWidth, Graphic::ClientHeight};

	//セルの数だけ確保
	mCellHeads.resize(GRID_WIDTH * GRID_HEIGHT);
	for (auto& head : mCellHeads) head = -1;

	//バッファーの初期化
	initBuffer(*scene.getGame().getGraphic().getDevice());

	mComputeParam = {
		0,
		mInteractionIntensity,
		mInteractionRange,
		mPecletNumber,
		mDeltaT,
		mDeltaTSqrt,
		mPixelParamRatio,
		PIXEL_AREA_WIDTH,
		PIXEL_AREA_HEIGHT,
		GRID_WIDTH,
		GRID_HEIGHT,
		CELL_SIZE,
		0
	};

	const UINT size = static_cast<int>(mCyanoLength / (mCyanoSpeed * mDeltaT));
	UINT maxNumCyanos = MaxPointNum / size;
	mCalculator = std::make_unique<CyanoCalculator>(scene.getGame().getGraphic(), MaxPointNum, maxNumCyanos, mComputeBuffer, mPointsIsInteractionBuffer, mComputeParam);


}


void CyanoSimulator::inputActor()
{
	if (isKeyJustPressed('I')) {
		addCyanos(1);
	}

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

	mCalculator->startCalculation(static_cast<UINT>(mPoints_pos.size()), mIndivisual_headPointIdx.size());
	//updateAngle();
	//createHead();
	//copyPointsToGPU();
}

void CyanoSimulator::draw() {

	//頂点をセット
	mCommandList.IASetVertexBuffers(0, 1, &mVertexBuffer->getView());

	//ディスクリプタヒープをディスクリプタテーブルにセット
	mCommandList.SetGraphicsRoot32BitConstants(0, 3, &mRenderDesc, 0);
	mCommandList.SetGraphicsRootDescriptorTable(1, mDescriptorHeap.getGPUHandle(mDescRange->getIndex(0)));
	mCommandList.SetGraphicsRootDescriptorTable(2, mDescriptorHeap.getGPUHandle(mDescRange->getIndex(1)));
	//描画。インデックスを使用
	mCommandList.IASetIndexBuffer(&mIndexBuffer->getView());
	mCommandList.DrawIndexedInstanced(static_cast<UINT>(indices.size()), static_cast<UINT>(mPoints_pos.size()), 0, 0, 0);
	
	

}

void CyanoSimulator::addCyano(const XMFLOAT4& headPos, float length, float speed)
{
	//個体のサイズ
	const int size = static_cast<int>(length / (speed * mDeltaT));
	//個体の点配列の先頭インデックス
	int beginIdx = static_cast<UINT>(mPoints_pos.size());
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
	mCellNext.resize(mCellNext.size() + size);
	mCellPrev.resize(mCellPrev.size() + size);
	const float angle = Random::normalDist(0.0f, 6.283184f);
	for (int i = 0; i < size; i++) {
		//点の位置を算出
		auto pos = headPos;
		mPoints_pos[beginIdx + i] = pos;
		mPoints_angle[beginIdx + i] = angle;

		//グリッドに追加
		//addCell(pos, beginIdx + i);

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

		//PBCラップ
		XMVECTOR mod = XMVectorSet(PIXEL_AREA_WIDTH, PIXEL_AREA_HEIGHT, 1.0f, 1.0f);
		XMVECTOR newHeadVecForMod = XMVectorAdd(newHeadVec, XMVectorSet(PIXEL_AREA_WIDTH, PIXEL_AREA_HEIGHT, 0.0f, 0.0f));
		XMVECTOR correctedHeadVec = XMVectorMod(newHeadVecForMod, mod);
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
	mUploadBuffer->uploadToCPU(mPoints_pos.data(), 0, 0, mPoints_pos.size() * sizeof(XMFLOAT4));
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

void CyanoSimulator::addCyanos(const int num)
{
	const int maxX = static_cast<int>(sqrtf(static_cast<float>(num) * PIXEL_AREA_WIDTH / PIXEL_AREA_HEIGHT));
	const int maxY = num / maxX;
	const float xDistance = PIXEL_AREA_WIDTH / static_cast<float>(maxX);
	const float yDistance = PIXEL_AREA_HEIGHT / static_cast<float>(maxY);
	const UINT pointsBeginIndex = mPoints_pos.size();
	const UINT individualBeginIndex = mIndivisual_size.size();
	for (int i = 0; i < maxX; i++) {
		for (int j = 0; j < maxY; j++) {
			addCyano(XMFLOAT4(i * xDistance, j * yDistance, 0.0f, 1.0f), mCyanoLength, mCyanoSpeed);
		}
	}

	//GPUメモリへ追加したシアノを送る
	mComputeBuffer->uploadToGPU(mPoints_pos.data(), pointsBeginIndex * sizeof(XMFLOAT4), pointsBeginIndex * sizeof(XMFLOAT4), (mPoints_pos.size() - pointsBeginIndex) * sizeof(XMFLOAT4));
	mCalculator->uploadIndividualBegin(mIndivisual_beginPointIdx, individualBeginIndex);
	mCalculator->uploadIndividualSize(mIndivisual_size, individualBeginIndex);
	mCalculator->uploadIndividualSpeed(mIndivisual_speed, individualBeginIndex);
	mCalculator->uploadIndividualHeadIdx(mIndivisual_headPointIdx, individualBeginIndex);
	mCalculator->uploadIndividualAngularVelocity(mIndivisual_angularVelocity, individualBeginIndex);
	mCalculator->uploadAngle(mPoints_angle, pointsBeginIndex);

	
}

//角度の更新
constexpr float ROOT2 = 1.41421356f;

void CyanoSimulator::updateAngle()
{
	const float noiseIntensity = ROOT2 / mPecletNumber;
	for (int indivisualIdx = 0; indivisualIdx < mIndivisual_headPointIdx.size(); indivisualIdx++) {
		const int preHeadIdx = mIndivisual_headPointIdx[indivisualIdx];
		const int newHeadIdx = preHeadIdx + 1 >= mIndivisual_beginPointIdx[indivisualIdx] + mIndivisual_size[indivisualIdx] ? mIndivisual_beginPointIdx[indivisualIdx] : preHeadIdx + 1;
		//角度の変位を計算
		const float preTheta = mPoints_angle[preHeadIdx];
		const float preOmega = mIndivisual_angularVelocity[indivisualIdx];

		//各速度の変位を計算
		const float noise = noiseIntensity * Random::normalDist(0.0f, 1.0f);
		//const float deltaOmega = -preOmega + noise;
		//const float newOmega = preOmega + deltaOmega * mDeltaT;

		//const float deltaTheta = newOmega - mInteractionIntensity * calcInteractionValue(indivisualIdx, mPoints_pos[preHeadIdx], preTheta);

		const float d_Theta = preOmega - mInteractionIntensity * calcInteractionValue(indivisualIdx, mPoints_pos[preHeadIdx], preTheta);
		const float deltaTheta = d_Theta * mDeltaT;
		const float deterministic_term = -deltaTheta;
		const float stochastic_term = noise * mDeltaTSqrt;
		const float deltaOmega = deterministic_term + stochastic_term;
		
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

			newParam.interactValue += sinf(2.0f * (baseAngle - otherAngle));
			newParam.interactNum++;
		}

		//点のインデックスを進める
		pointIdx = mCellPrev[pointIdx];
	}

	return newParam;
}

void CyanoSimulator::initBuffer(ID3D12Device& device)
{
	//GPUで更新する用のバッファ
	auto& factory = GetEngineResourceFactory();
	//mUploadBuffer = factory.createStructuredBuffer(MaxPointNum, static_cast<int>(sizeof(RenderData)));
	mComputeBuffer = factory.createRWStructuredBuffer(MaxPointNum, static_cast<int>(sizeof(RenderData)));
	mPointsIsInteractionBuffer = factory.createRWStructuredBuffer(MaxPointNum, sizeof(float));

	//スプライト用バッファ
	VertexBufferDescription vertexDesc = { 4, 4 };
	mVertexBuffer = std::make_unique<VertexBuffer>(device, vertexDesc, vertices);

	mIndexBuffer = std::make_unique<IndexBuffer>(device, indices);

	//テクスチャの取得
	mTexture = mAssetManager.getShaderResource("Content/picture/white.png");

	//ディスクリプタヒープに登録
	mDescRange = mDescriptorHeap.allocate(NumSlots(3));
	mDescriptorHeap.addUAV(*mComputeBuffer, mDescRange->getIndex(0));
	mDescriptorHeap.addUAV(*mPointsIsInteractionBuffer, mDescRange->getIndex(1));
	//mDescriptorHeap.addUploadSRV (*mUploadBuffer.get(), mDescRange->getIndex(0));
	mDescriptorHeap.addTextureView(*mTexture, mDescRange->getIndex(2));


}

