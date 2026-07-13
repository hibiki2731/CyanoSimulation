#include "CyanoSimulator.h"
#include "Math.h"
#include "SpriteComponent.h"
#include "input.h"

const int CyanoSimulator::GRID_WIDTH = 500;
const int CyanoSimulator::GRID_HEIGHT = 500;
const int CyanoSimulator::CELL_SIZE = 1;

CyanoSimulator::CyanoSimulator(Scene& scene):
	Actor(scene)
{
	//セルの数だけ確保
	mCellHeads.resize(GRID_WIDTH * GRID_WIDTH);
	for (auto& head : mCellHeads) head = -1;

}

void CyanoSimulator::inputActor()
{
	if (isKeyJustPressed('I'))
		addCyano(XMFLOAT2(500.0f, 300.0f), 100, 20);
}

void CyanoSimulator::addCyano(const XMFLOAT2& headPos, float length, float speed)
{
	//個体の点配列の先頭インデックス
	int beginIdx = mPoints_pos.size();
	mIndivisual_beginPointIdx.push_back(beginIdx);
	mIndivisual_headPointIdx.push_back(beginIdx);
	//個体のサイズ
	const int size = static_cast<int>(length / speed);
	mIndivisual_size.push_back(size);

	//点配列への追加
	//仮に真横へ伸びるようにする
	mPoints_pos.resize(mPoints_pos.size() + size);
	mPoints_sprites.resize(mPoints_sprites.size() + size);
	mCellNext.resize(mCellNext.size() + size);
	mCellPrev.resize(mCellPrev.size() + size);
	for (int i = 0; i < size; i++) {
		//点の位置を算出
		auto pos = headPos + XMFLOAT2(speed * i + 10, 0);
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

void CyanoSimulator::addCell(const XMFLOAT2& pos, int idx)
{
	//セル番号を計算
	int cellIdx = static_cast<int>(pos.y / CELL_SIZE * GRID_WIDTH + pos.x / CELL_SIZE);

	int oldHead = -1;
	int it = mCellHeads[idx];
	if (it != -1) oldHead = it;

	mCellNext[idx] = oldHead;
	mCellPrev[idx] = -1;
	if (oldHead != -1) mCellPrev[oldHead] = idx;

	mCellHeads[cellIdx] = idx;
}
