#pragma once
#include "GameObject/Actor.h"

//描画用バッファ
struct RenderData {
	XMFLOAT4 position;
};

class CyanoSimulator : public Actor
{
public:
	CyanoSimulator(Scene& scene);
	void inputActor() override;

	void endProcessActor() override;

	void updateActor() override;
	void draw();
	void addCyano(const XMFLOAT4& headPos, float length, float speed);

	const std::string getClassName() const { return "CyanoSimulator"; }

private:
	bool adjustUpdateRate();
	void addCell(const XMFLOAT4& pos, int idx);
	void deleteCell(int idx);
	void createHead();
	void copyPointsToGPU();
	int calcCellIdx(const XMFLOAT4& pos);
	int wrapCellIdx(const int cellX, const int cellY);
	void applyParamaterToPicselScale();

	//シアノの追加関数
	void addCyanos(const int num);

	//シアノの特性パラメータ
	float mInteractionIntensity;
	float mInteractionRange;
	float mPecletNumber;
	float mCyanoSpeed;
	float mCyanoLength;
	float mDeltaT;
	float mDeltaTSqrt;
	float mAreaWidth;
	float mAreaHeight;
	float mPixelParamRatio;


	//壁との衝突判定
	bool isNearWall(const int cellIdx);
	XMVECTOR calcWallHit(const XMFLOAT4& preHeadPos, FXMVECTOR newHeadVec, const float speed);

	//角度の更新
	struct InteractParamater {
		int interactNum = 0;
		float interactValue = 0.0f;
	};

	void updateAngle();
	float calcInteractionValue(const int indivisualIdx, const XMFLOAT4& basePos, const float baseoAngle);
	InteractParamater calcInteractInCell(const int selfBeginIdx, const int selfSize, const int cellIdx, const InteractParamater& refParam, const XMFLOAT4& basePos, const float baseAngle);
	float calcDeltaHeadAngle(FXMVECTOR preHeadVec, FXMVECTOR newHeadVec, float preAngle);

	//空間分割法に用いるパラメータ
	static const float PIXEL_AREA_WIDTH;
	static const float PIXEL_AREA_HEIGHT;
	static const int GRID_WIDTH;
	static const int GRID_HEIGHT;
	static const int CELL_SIZE;

	//更新頻度調整用のタイマー
	float mUpdateInterval = 0.01f;
	float mUpdateTimer = 0.0f;

	//点毎のデータ
	std::vector<XMFLOAT4> mPoints_pos;	//各点の位置
	std::vector<int> mCellNext;			//各点の同じグリッド内の次の点のインデックス
	std::vector<int> mCellPrev;			//各点の同じグリッド内の前の点のインデックス

	//一様グリッド
	std::vector<int> mCellHeads;	//グリッド内の先頭点のインデックス

	//個体ごとのデータ
	std::vector<int> mIndivisual_headPointIdx;	//頭のインデックス
	std::vector<float> mPoints_angle;		//個体の頭の角度
	std::vector<float> mIndivisual_angularVelocity;		//個体の頭の角速度
	std::vector<float> mIndivisual_speed;
	//シアノバクテリアの各個体のpositionsの占有領域
	std::vector<int> mIndivisual_beginPointIdx;	//占有領域の先頭インデックス
	std::vector<int> mIndivisual_size;				//占有領域のサイズ

	std::vector<class SpriteComponent*> mPoints_sprites;

	struct RenderDesc {
		float cyanoSize;
		XMFLOAT2 WindowSize;
	};
	RenderDesc mRenderDesc;
	const UINT MaxPointNum = 1 << 24;
	std::unique_ptr<class IRWStructuredBuffer> mUploadBuffer;
	std::unique_ptr<class VertexBuffer> mVertexBuffer;
	std::unique_ptr<class IndexBuffer> mIndexBuffer;
	class Graphic& mGraphic;
	ID3D12GraphicsCommandList& mCommandList;
	class AssetManager& mAssetManager;
	ID3D12Resource* mTexture;
	class DescriptorHeap& mDescriptorHeap;
	std::unique_ptr<class DescriptorSlotRange> mDescRange;

	void initBuffer(ID3D12Device& device);

	std::unique_ptr<class CyanoCalculator> mCalculator;
#ifdef _DEBUG
	friend class GUIDebugger;
#endif

};

