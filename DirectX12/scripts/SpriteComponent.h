#pragma once
#include <memory>
#include <d3d12.h>
#include <DirectXMath.h>
#include "Component.h"
#include "Graphic.h"

class AssetManager;
struct SpriteData;
class GUIDebugger;

class SpriteComponent :
    public Component
{
public:
	SpriteComponent(Actor& owner, float zDepth = 100.0f);

	void endProcess() override;

    void create(const std::string filename);
    virtual void draw();

	//描画範囲のセッター
	void setPosition(const XMFLOAT3& position);
	void setScale(const XMFLOAT2& scale);
	void setRotation(const float rotation);
    void setSpriteSize(const XMFLOAT2& size);
	void setBordarSize(const float size);
	void movePosition(const XMFLOAT2& diff);
    void setZPos(float zPos);

    //デバッグ用
#ifdef _DEBUG
    void activateControll(const std::string& structName);
	bool getActiveControll() const { return mActiveControll; }

#endif

private:
    //描画範囲
    XMFLOAT3 mPosition;
	XMFLOAT2 mScale;
	float mRotation;
    XMFLOAT2 mSpriteSize;
	XMFLOAT2 mTextureSize;
	float mBordarSize;
	std::string mTextureFilePath;

    //デバック用
    HRESULT Hr;

    //グラフィック
    Graphic& mGraphic;
    AssetManager& mAssetManager;

    //コマンドリスト
    ID3D12GraphicsCommandList* mCommandList;

    //コンスタントバッファ1(World Matrix)
    //使用するディスクリプタヒープおよびコンスタントバッファのインデックス
    int mHeapIndex;
    int mHeapSize;
    int mCBIndex;
    int mCBSize;
    int mNumSprites;

    SpriteConstBuf Cb3;

    //頂点バッファ
    D3D12_VERTEX_BUFFER_VIEW mVertexBufView;
    D3D12_INDEX_BUFFER_VIEW mIndexBufView;
    //テクスチャバッファ
    ID3D12Resource* mTextureBuf;

    //デバッグ用
#ifdef _DEBUG
	friend class GUIDebugger;
	bool mActiveControll = false;
	std::string mStructName;
#endif

};

