#include "CyanoSimulator.h"
#include "Math.h"
#include "SpriteComponent.h"
#include "input.h"
#include "Random.h"
#include "timer.h"

const int CyanoSimulator::CELL_SIZE = 40;
const int CyanoSimulator::GRID_WIDTH = Graphic::ClientWidth / CELL_SIZE;
const int CyanoSimulator::GRID_HEIGHT = Graphic::ClientHeight / CELL_SIZE;

CyanoSimulator::CyanoSimulator(Scene& scene):
	Actor(scene)
{
	//セルの数だけ確保
	mCellHeads.resize(GRID_WIDTH * GRID_HEIGHT);
	for (auto& head : mCellHeads) head = -1;

}

void CyanoSimulator::inputActor()
{
	if (isKeyJustPressed('I'))
		addCyano(XMFLOAT2(500.0f, 300.0f), 100, 5);

	if (isKeyJustPressed('O'))
		add100Cyano();
}

void CyanoSimulator::updateActor()
{
	if (!adjustUpdateRate()) return;

	updateAngle();
	createHead();
}

void CyanoSimulator::addCyano(const XMFLOAT2& headPos, float length, float speed)
{
	//個体のサイズ
	const int size = static_cast<int>(length / speed);
	//個体の点配列の先頭インデックス
	int beginIdx = mPoints_pos.size();
	mIndivisual_beginPointIdx.push_back(beginIdx);
	mIndivisual_headPointIdx.push_back(beginIdx + size - 1);
	mIndivisual_size.push_back(size);
	//個体の先頭の角度
	mIndivisual_angle.push_back(0.0f);
	mIndivisual_angularVelocity.push_back(0.0f);
	mIndivisual_speed.push_back(speed);

	//点配列への追加
	//仮に真横へ伸びるようにする
	mPoints_pos.resize(mPoints_pos.size() + size);
	mPoints_sprites.resize(mPoints_sprites.size() + size);
	mCellNext.resize(mCellNext.size() + size);
	mCellPrev.resize(mCellPrev.size() + size);
	for (int i = 0; i < size; i++) {
		//点の位置を算出
		auto pos = headPos + XMFLOAT2(speed * i, 0);
		mPoints_pos[beginIdx + i] = pos;

		//グリッドに追加
		addCell(pos, beginIdx + i);

		//スプライトの生成
		auto sprite = std::make_unique<SpriteComponent>(*this);
		sprite->create("assets/picture/UI2/PNG/Default/checkbox_grey_empty.png");
		sprite->setPosition(pos);
		sprite->setSpriteSize(XMFLOAT2(speed, speed));
		mPoints_sprites[beginIdx + i] = sprite.get();

		addComponent(std::move(sprite));

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

void CyanoSimulator::addCell(const XMFLOAT2& pos, int idx)
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
		const XMFLOAT2& preHeadPos = mPoints_pos[preHeadIdx];
		const XMVECTOR preHeadVec = XMLoadFloat2(&preHeadPos);
		const float& speed = mIndivisual_speed[indivisualIdx];
		const float& angle = mIndivisual_angle[indivisualIdx];

		//角度方向へ移動
		XMMATRIX rotX = XMMatrixRotationZ(angle);
		XMVECTOR xVec = XMVectorSet(1.0f, 0.0f, 0.0f, 1.0f);
		XMVECTOR newHeadVec = XMVectorAdd(XMVectorScale(XMVector2TransformNormal(xVec, rotX), speed), preHeadVec);

		//壁との衝突反転
		XMVECTOR correctedHeadVec = calcWallHit(preHeadPos, newHeadVec, speed);

		//点の更新
		//セルの変更

		//新たな角度の算出
		const float newAngle = calcDeltaHeadAngle(preHeadVec, correctedHeadVec, angle);
		mIndivisual_angle[indivisualIdx] = newAngle;

		//新たな頭のインデックスを計算
		const int newHeadIdx = preHeadIdx + 1 >= mIndivisual_beginPointIdx[indivisualIdx] + mIndivisual_size[indivisualIdx] ? mIndivisual_beginPointIdx[indivisualIdx] : preHeadIdx + 1;
		//最後尾をセルから削除
		deleteCell(newHeadIdx);
		//点の更新
		XMFLOAT2 completedNewHeadPos;
		XMStoreFloat2(&completedNewHeadPos, correctedHeadVec);
		mIndivisual_headPointIdx[indivisualIdx] = newHeadIdx;
		mPoints_pos[newHeadIdx] = completedNewHeadPos;
		mPoints_sprites[newHeadIdx]->setPosition(completedNewHeadPos);

		//新たな点をセルに追加
		addCell(completedNewHeadPos, newHeadIdx);

	}
	
}

int CyanoSimulator::calcCellIdx(const XMFLOAT2& pos)
{
	int cellIdx = static_cast<int>(pos.x) / CELL_SIZE + static_cast<int>(pos.y) / CELL_SIZE * GRID_WIDTH;

	return cellIdx;
}

void CyanoSimulator::add100Cyano()
{
	for(int i = 0; i < 100; i++) 
		addCyano(XMFLOAT2(500.0f, 300.0f), 100, 5);
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

XMVECTOR CyanoSimulator::calcWallHit(const XMFLOAT2& preHeadPos, FXMVECTOR newHeadVec, const float speed)
{
	//点が画面端にあるかを判定
	const int cellIdx = calcCellIdx(preHeadPos);
	if (!isNearWall(cellIdx)) return newHeadVec;

	//clampで画面外に以下ないよう調整
	XMVECTOR min = XMVectorSet(speed, speed, 0.0f, 0.0f);
	XMVECTOR max = XMVectorSet(Graphic::ClientWidth - speed, Graphic::ClientHeight - speed, 0.0f, 0.0f);

	XMVECTOR correctedNewVec = XMVectorClamp(newHeadVec, min, max);

	return correctedNewVec;

}

//角度の更新
constexpr float INTERACTION_INTENSITY = 2.0f;
constexpr float PECLET_NUMBER = 4.0f;
constexpr float ROOT2 = 1.41421356;
constexpr float NOISE_INTENSITY = ROOT2 / PECLET_NUMBER;

void CyanoSimulator::updateAngle()
{
	for (int indivisualIdx = 0; indivisualIdx < mIndivisual_headPointIdx.size(); indivisualIdx++) {
		//角度の変位を計算
		const float preTheta = mIndivisual_angle[indivisualIdx];
		const float preOmega = mIndivisual_angularVelocity[indivisualIdx];
		const float deltaTheta = preOmega - INTERACTION_INTENSITY * calcInteractionValue();

		//各速度の変位を計算
		const float noise = NOISE_INTENSITY * Random::normalDist(0.0f, 1.0f);
		const float deltaOmega = -deltaTheta + noise;
		
		//角度、角速度を更新
		mIndivisual_angle[indivisualIdx] += deltaTheta;
		mIndivisual_angularVelocity[indivisualIdx] += deltaOmega;

	}
}

float CyanoSimulator::calcInteractionValue()
{
	return 0.0f;
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

