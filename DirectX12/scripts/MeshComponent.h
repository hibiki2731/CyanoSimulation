#pragma once
#pragma once

//#define USE_INDEX
#include <memory>
#include <d3d12.h>
#include <DirectXMath.h>
#include "Graphic.h"
#include "AssetManager.h"
#include "Component.h"

class MeshComponent : public Component
{
public:
	MeshComponent(Actor& owner, int updateOrder = 100);
    DECLARE_COMPONENT_NAME(MeshComponent)
    void loadFromJson(const nlohmann::json& json) override;

	void endProcess() override;

    void create(const std::string& meshID);
    void create(const MeshData * meshData);
	void draw();

    void updateFlashIntensity(float intensity);

    //getter
    const std::string& getMeshID();

private:

    HRESULT Hr;

    //コマンドリスト
    Graphic& mGraphic;
    ID3D12GraphicsCommandList* mCommandList;

    //コンスタントバッファ1(World Matrix)
    World3DConstBuf Cb1;

    //メッシュパーツ
    int NumParts;
    struct PARTS {
        //頂点バッファ
        UINT NumVertices;
        D3D12_VERTEX_BUFFER_VIEW VertexBufView;
        //コンスタントバッファ2(マテリアル)
        MaterialConstBuf Cb2;//マップしたアドレスを入れる
        //テクスチャバッファ
        ID3D12Resource* TextureBuf;
    };

	std::vector<PARTS> Parts;

    //ディスクリプタヒープ
    UINT CbvTbvSize;//ビューのサイズ
    const UINT NumDescriptors = 4;//ひとつのパーツで使用するディスクリプタの数

    //使用するディスクリプタヒープおよびコンスタントバッファのインデックス
    int mHeapIndex;
    int mHeapSize;
    int mCBIndex;
    int mCBSize;

    //メッシュのID
    std::string mMeshID;
};
