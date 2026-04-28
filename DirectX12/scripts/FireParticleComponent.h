#pragma once
#include "Component.h"
#include "Math.h"
#include "Graphic.h"
using namespace DirectX;

class FireParticleComponent :
    public Component
{
public:
    FireParticleComponent(class Actor& owner);
    DECLARE_COMPONENT_NAME(FireParticleComponent)
    
    void endProcess() override;
    void updateComponent() override;
    void draw();

    //setter
    void setEmitterPosition(const XMFLOAT3& position) { mEmitterPosition = position; }

private:

    //GPUへ送るデータ
    struct UploadDataForGPU {
        XMFLOAT3 position;  //ワールド座標
        float size;         //ビルボードサイズ
        float lifeRatio;    //寿命の割合 1.0 生存-> 0.0 死亡
    };

    //エミッター設定
    struct EmitterConfig {
        float spawnRate     = 0.08f;  //何秒ごとに1パーティクル生成
        int   spawnCount    = 1;      //1回の生成数
        float lifeTimeMin   = 1.0f;
        float lifeTimeMax   = 2.0f;
        float speedMin      = 0.01f;
        float speedMax      = 0.8f;
        float sizeStart     = 0.15f;
        float sizeEnd       = 0.02f;
        float spreadRadius  = 0.05f;  //横方向のばらつき
    };

    //CPU側の管理データ
    struct ParticleData {
        UploadDataForGPU gpu;   //GPU送信用
        float        lifeTime;  //最大寿命
        float        elapsed;   //経過時間
        XMFLOAT3     velocity;  //速度
        float        sizeStart; //初期サイズ
    };

    void spawnParticle();
    void uploadToGPU();

    //パーティクルデータ配列
    std::vector<ParticleData> mParticles;
    std::vector<UploadDataForGPU> mGPUData;

    //GPUリソース
    ComPtr<ID3D12Resource>   mVertexBuf[2];
    D3D12_VERTEX_BUFFER_VIEW mVertexBufView[2];
    UINT8*                   mMappedData[2];

    //コンスタントバッファ
    BillboardConstBuf mCB;
    int mHeapSize,  mHeapIndex;

    //テクスチャ
    ID3D12Resource* mTextureBuf;

    //エミッターの設定
    XMFLOAT3        mEmitterPosition;
    EmitterConfig   mConfig;
    float           mSpawnTimer;

    //定数    
    const int  MaxParticles     = 64;
    const UINT NumElemPerVertex = 5; // pos(3) + size(1) + lifeRatio(1)
    const UINT SizeInByte = sizeof(UploadDataForGPU) * 64;

    //マネージャー
    class AssetManager& mAssetManager;
    class Graphic& mGraphic;

#ifdef _DEBUG
    friend class GUIDebugger;
#endif
};
