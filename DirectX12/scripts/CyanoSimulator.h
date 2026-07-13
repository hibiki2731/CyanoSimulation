#pragma once
#include "Actor.h"

class CyanoSimulator : public Actor
{
public:
	CyanoSimulator(Scene& scene);
	void inputActor() override;

	void addCyano(const XMFLOAT2& headPos, float length, float speed);

	const std::string getClassName() const { return "CyanoSimulator"; }

private:
	void addCell(const XMFLOAT2& pos, int idx);

	static const int GRID_WIDTH;
	static const int GRID_HEIGHT;
	static const int CELL_SIZE;

	//点毎のデータ
	std::vector<XMFLOAT2> mPoints_pos;	//各点の位置
	std::vector<int> mCellNext;			//各点の同じグリッド内の次の点のインデックス
	std::vector<int> mCellPrev;			//各点の同じグリッド内の前の点のインデックス

	//一様グリッド
	std::vector<int> mCellHeads;	//グリッド内の先頭点のインデックス

	//個体ごとのデータ
	std::vector<int> mIndivisual_headPointIdx;	//頭のインデックス
	//シアノバクテリアの各個体のpositionsの占有領域
	std::vector<int> mIndivisual_beginPointIdx;	//占有領域の先頭インデックス
	std::vector<int> mIndivisual_size;				//占有領域のサイズ

	std::vector<class SpriteComponent*> mPoints_sprites;

};

