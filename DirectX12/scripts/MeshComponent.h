#pragma once

//#define USE_INDEX
#include <memory>
#include "d3d12.h"
#include <DirectXMath.h>
#include "Graphic.h"
#include "AssetManager.h"
#include "Component.h"

class MeshComponent : public Component
{
public:
	MeshComponent(Actor& owner, const std::shared_ptr<class MeshBaseCBSuballocation>& baseSuballocation, int updateOrder = 100);
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
    //コマンドリスト
    Graphic& mGraphic;
    ID3D12GraphicsCommandList* mCommandList;

    //メッシュパーツ
    int NumParts;
    struct PARTS {
        //頂点バッファ
        UINT NumVertices;
        D3D12_VERTEX_BUFFER_VIEW VertexBufView;
        //コンスタントバッファ2(マテリアル)
        std::shared_ptr<class MeshMaterialCBSuballocation> MaterialSuballocation;
        //テクスチャバッファ
        ID3D12Resource* TextureBuf;
    };

	std::vector<PARTS> Parts;

    //ディスクリプタヒープ
	class DescriptorHeap& mDescriptorHeap;
    const UINT NumDescriptors = 4;//ひとつのパーツで使用するディスクリプタの数
    UINT NumAllPartsDescriptors;
    //デスクリプタヒープのレンジ
    std::unique_ptr<class DescriptorSlotRange> mDescRange;

    //コンスタントバッファ
	class ConstantBuffer& mConstantBuffer;
	//コンスタントバッファのサブアロケータ
    std::shared_ptr<class MeshWorldCBSuballocation> mWorldSuballocation;
    std::weak_ptr<class MeshBaseCBSuballocation> mBaseSuballocation;

    //メッシュのID
    std::string mMeshID;

    //初期化フラグ
    bool isInitialized;
};
