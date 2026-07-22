#pragma once
#include "Actor.h"

class CyanoSimulator : public Actor
{
public:
	CyanoSimulator(Scene& scene);
	void inputActor() override;

	void updateActor() override;
	void addCyano(const XMFLOAT2& headPos, float length, float speed);

	const std::string getClassName() const { return "CyanoSimulator"; }

private:
	bool adjustUpdateRate();
	void addCell(const XMFLOAT2& pos, int idx);
	void deleteCell(int idx);
	void createHead();
	int calcCellIdx(const XMFLOAT2& pos);
	void add100Cyano();

	//壁との衝突判定
	bool isNearWall(const int cellIdx);
	XMVECTOR calcWallHit(const XMFLOAT2& preHeadPos, FXMVECTOR newHeadVec, const float speed);

	//角度の更新
	void updateAngle();
	float calcInteractionValue();
	float calcDeltaHeadAngle(FXMVECTOR preHeadVec, FXMVECTOR newHeadVec, float preAngle);

	//空間分割法に用いるパラメータ
	static const int GRID_WIDTH;
	static const int GRID_HEIGHT;
	static const int CELL_SIZE;

	//更新頻度調整用のタイマー
	float mUpdateInterval = 0.01f;
	float mUpdateTimer = 0.0f;

	//点毎のデータ
	std::vector<XMFLOAT2> mPoints_pos;	//各点の位置
	std::vector<int> mCellNext;			//各点の同じグリッド内の次の点のインデックス
	std::vector<int> mCellPrev;			//各点の同じグリッド内の前の点のインデックス

	//一様グリッド
	std::vector<int> mCellHeads;	//グリッド内の先頭点のインデックス

	//個体ごとのデータ
	std::vector<int> mIndivisual_headPointIdx;	//頭のインデックス
	std::vector<float> mIndivisual_angle;		//個体の頭の角度
	std::vector<float> mIndivisual_angularVelocity;		//個体の頭の角速度
	std::vector<float> mIndivisual_speed;
	//シアノバクテリアの各個体のpositionsの占有領域
	std::vector<int> mIndivisual_beginPointIdx;	//占有領域の先頭インデックス
	std::vector<int> mIndivisual_size;				//占有領域のサイズ

	std::vector<class SpriteComponent*> mPoints_sprites;

#ifdef _DEBUG
	friend class GUIDebugger;
#endif

};

