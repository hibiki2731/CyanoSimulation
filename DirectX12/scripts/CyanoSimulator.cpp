#include "CyanoSimulator.h"
#include "Math.h"
#include "SpriteComponent.h"
#include "input.h"
#include "Random.h"
#include "timer.h"

const int CyanoSimulator::CELL_SIZE = 20;
const float CyanoSimulator::AREA_WIDTH = Graphic::ClientWidth * 0.5f;
const float CyanoSimulator::AREA_HEIGHT = Graphic::ClientWidth * 0.5f;

const int CyanoSimulator::GRID_WIDTH =  AREA_WIDTH / CELL_SIZE;
const int CyanoSimulator::GRID_HEIGHT = AREA_HEIGHT / CELL_SIZE;
const float CyanoSpeed = 8;

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
		addCyano(XMFLOAT2(200.0f, 300.0f), 100, CyanoSpeed);

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
	mIndivisual_angularVelocity.push_back(0.0f);
	mIndivisual_speed.push_back(speed);

	//点配列への追加
	//仮に真横へ伸びるようにする
	mPoints_pos.resize(mPoints_pos.size() + size);
	mPoints_angle.resize(mPoints_angle.size() + size);
	mPoints_sprites.resize(mPoints_sprites.size() + size);
	mCellNext.resize(mCellNext.size() + size);
	mCellPrev.resize(mCellPrev.size() + size);
	for (int i = 0; i < size; i++) {
		//点の位置を算出
		auto pos = headPos + XMFLOAT2(speed * i, 0);
		mPoints_pos[beginIdx + i] = pos;
		mPoints_angle[beginIdx + i] = 0.0f;

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
		const int newHeadIdx = preHeadIdx + 1 >= mIndivisual_beginPointIdx[indivisualIdx] + mIndivisual_size[indivisualIdx] ? mIndivisual_beginPointIdx[indivisualIdx] : preHeadIdx + 1;
		const XMFLOAT2& preHeadPos = mPoints_pos[preHeadIdx];
		const XMVECTOR preHeadVec = XMLoadFloat2(&preHeadPos);
		const float& speed = mIndivisual_speed[indivisualIdx];
		const float& angle = mPoints_angle[newHeadIdx];

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
		mPoints_angle[newHeadIdx] = newAngle;

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
		addCyano(XMFLOAT2(200.0f, 300.0f), 200, CyanoSpeed);
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
	XMVECTOR max = XMVectorSet(AREA_WIDTH - speed, AREA_HEIGHT - speed, 0.0f, 0.0f);

	XMVECTOR correctedNewVec = XMVectorClamp(newHeadVec, min, max);

	return correctedNewVec;

}

//角度の更新
constexpr float INTERACTION_INTENSITY = 4.0f;
constexpr float PECLET_NUMBER = 2.0f;
constexpr float ROOT2 = 1.41421356;
constexpr float NOISE_INTENSITY = ROOT2 / PECLET_NUMBER;

void CyanoSimulator::updateAngle()
{
	for (int indivisualIdx = 0; indivisualIdx < mIndivisual_headPointIdx.size(); indivisualIdx++) {
		const int preHeadIdx = mIndivisual_headPointIdx[indivisualIdx];
		const int newHeadIdx = preHeadIdx + 1 >= mIndivisual_beginPointIdx[indivisualIdx] + mIndivisual_size[indivisualIdx] ? mIndivisual_beginPointIdx[indivisualIdx] : preHeadIdx + 1;
		//角度の変位を計算
		const float preTheta = mPoints_angle[preHeadIdx];
		const float preOmega = mIndivisual_angularVelocity[indivisualIdx];
		const float deltaTheta = preOmega - INTERACTION_INTENSITY * calcInteractionValue(indivisualIdx, mPoints_pos[preHeadIdx], preTheta);

		//各速度の変位を計算
		const float noise = NOISE_INTENSITY * Random::normalDist(0.0f, 1.0f);
		const float deltaOmega = -deltaTheta + noise;
		
		//角度、角速度を更新
		mPoints_angle[newHeadIdx] += deltaTheta;
		mIndivisual_angularVelocity[indivisualIdx] += deltaOmega;

	}
}

float CyanoSimulator::calcInteractionValue(const int indivisualIdx, const XMFLOAT2& basePos, const float baseAngle)
{
	const int cellIdx = calcCellIdx(basePos);
	const int selfBeginIdx = mIndivisual_beginPointIdx[indivisualIdx];
	const int selfSize = mIndivisual_size[indivisualIdx];
	InteractParamater interactParam;
	//自身のセルを中心とした3×3の範囲に存在する点の影響を計算
	//上の列
	const int upCellIdx = cellIdx - GRID_WIDTH;
	if (upCellIdx >= 0) {
		interactParam = calcInteractInCell(selfBeginIdx, selfSize, upCellIdx, interactParam, basePos, baseAngle);

		//左上
		const int leftUpCellIdx = upCellIdx - 1;
		if (leftUpCellIdx >= 0 && leftUpCellIdx % GRID_WIDTH != GRID_WIDTH - 1)
			interactParam = calcInteractInCell(selfBeginIdx, selfSize, leftUpCellIdx, interactParam, basePos, baseAngle);

		//右上
		const int rightUpCellIdx = upCellIdx + 1;
		if (rightUpCellIdx % GRID_WIDTH != 0)
			interactParam = calcInteractInCell(selfBeginIdx, selfSize, rightUpCellIdx, interactParam, basePos, baseAngle);

	}

	//真ん中の列
	const int centerCellIdx = cellIdx;
	interactParam = calcInteractInCell(selfBeginIdx, selfSize, centerCellIdx, interactParam, basePos, baseAngle);

	//左
	const int leftCenterCellIdx = centerCellIdx - 1;
	if (leftCenterCellIdx >= 0 && leftCenterCellIdx % GRID_WIDTH != GRID_WIDTH - 1)
		interactParam = calcInteractInCell(selfBeginIdx, selfSize, leftCenterCellIdx, interactParam, basePos, baseAngle);

	//右
	const int rightCenterCellIdx = centerCellIdx + 1;
	if (rightCenterCellIdx < GRID_WIDTH * GRID_HEIGHT && rightCenterCellIdx % GRID_WIDTH != 0)
		interactParam = calcInteractInCell(selfBeginIdx, selfSize, rightCenterCellIdx, interactParam, basePos, baseAngle);

	//下の列
	const int downCellIdx = cellIdx + GRID_WIDTH;
	if (downCellIdx <= GRID_WIDTH * GRID_HEIGHT - 1) {
		interactParam = calcInteractInCell(selfBeginIdx, selfSize, downCellIdx, interactParam, basePos, baseAngle);

		//左下
		const int leftDownCellIdx = downCellIdx - 1;
		if (leftDownCellIdx % GRID_WIDTH != GRID_WIDTH - 1)
			interactParam = calcInteractInCell(selfBeginIdx, selfSize, leftDownCellIdx, interactParam, basePos, baseAngle);

		//右下
		const int rightDownCellIdx = downCellIdx + 1;
		if (rightDownCellIdx < GRID_WIDTH * GRID_HEIGHT && rightDownCellIdx % GRID_WIDTH != 0)
			interactParam = calcInteractInCell(selfBeginIdx, selfSize, rightDownCellIdx, interactParam, basePos, baseAngle);

	}

	if (interactParam.interactNum == 0) return 0.0f;

	return interactParam.interactValue / interactParam.interactNum;
}

CyanoSimulator::InteractParamater CyanoSimulator::calcInteractInCell(const int selfBeginIdx, const int selfSize, const int cellIdx, const CyanoSimulator::InteractParamater& refParam, const XMFLOAT2& basePos, const float baseAngle)
{
	int pointIdx = mCellHeads[cellIdx];
	InteractParamater newParam = refParam;
	while (pointIdx >= 0) {
		if (pointIdx >= selfBeginIdx && pointIdx < selfBeginIdx + selfSize) {
			pointIdx = mCellPrev[pointIdx];
			continue;
		}

		const XMFLOAT2& otherPos = mPoints_pos[pointIdx];

		const float distance = Math::distance(basePos, otherPos);

		if (distance < CyanoSpeed) {
			const float otherAngle = mPoints_angle[pointIdx];

			newParam.interactValue += cosf(baseAngle - otherAngle);
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

