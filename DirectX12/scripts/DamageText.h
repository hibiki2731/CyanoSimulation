#pragma once
#include <DirectXMath.h>
#include "Graphic.h"
#include <vector>
using namespace DirectX;

class Game;


class DamageText
{
public:
	DamageText(XMFLOAT3& position, int digit, float maxLifeTime, XMFLOAT3 velocity);
	~DamageText();

	void update();

	float getPosX();
	float getPosY();
	float getPosZ();
	XMFLOAT3 getPos();
	float getLifeTime();

private:
	XMFLOAT3 mCenterPosition;
	XMFLOAT3 mVelocity;
	int mDigit;	//数値
	float mLifeTime;	//表示時間
};

class DamageTextManager {
public:
	DamageTextManager(Game* game);
	~DamageTextManager();

	void update();
	void draw();

	//ダメージテキストの追加
	void createDamageText(XMFLOAT3& position, int digit);

	void updateView(XMMATRIX& view);

	float getSize();

private:
	struct DamageTextInstance {
		XMFLOAT3 pos;
		float size;
		float  digit;
		float alpha;
	};

	const UINT NumElementsPerVertex = 6; //頂点ごとの要素数
	const int MaxNum = 16;
	//const int MaxInstanceNum = MaxNum;
	const UINT NumElements = NumElementsPerVertex * MaxNum; //全要素数
	const UINT SizeInByte = sizeof(float) * NumElements; //全バイト数
	std::vector<DamageTextInstance> mInstanceRawData;	//{中心座標x, y, z, 大きさ, 数値, alpha}
	int mNextInstanceIndex; //次にインスタンスデータを入れるインデックス
	UINT8* mMappedData[2];
	ComPtr<ID3D12Resource> mVertexBuf[2];
	D3D12_VERTEX_BUFFER_VIEW mVertexBufView[2];

	BillboardConstBuf mBC;
	ID3D12Resource* mTextureBuf;

	int mCBIndex;
	int mCBSize;
	int mHeapIndex;
	int mHeapSize;

	Game* mGame;

	//各ダメージテキストの設定値
	const float DTSize = 0.1f;
	const float MaxLifeTime = 1.0f;	//最大表示時間
	const XMFLOAT3 Velocity = {0.0f, 0.2f, 0.0f};

};
