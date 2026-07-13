#pragma once
#include "Definition.h"

//Windows API
#include <wrl/client.h>
#include <DirectXMath.h>
#include "directx/d3dx12.h"

//バッファの確保

//全頂点数と1頂点あたりの要素数を保管
struct VertexBufferDescription {
	UINT numVertices;	//全頂点数
	UINT numElementsPerVertex;		//1頂点あたりの要素数
};

class VertexBuffer
{
public:
	VertexBuffer(ID3D12Device& device, const VertexBufferDescription& desc, const std::vector<float>& data);

	const D3D12_VERTEX_BUFFER_VIEW& getView() const { return mVertexBufferView; }
	const UINT getNumVertices() const { return mDesc.numVertices; }


private:
	void createBuf(ID3D12Device& device);
	D3D12_HEAP_PROPERTIES createHeapProperties();
	D3D12_RESOURCE_DESC createResourceDesc();
	void setData(const std::vector<float>& src);

	void createView();

	ComPtr<ID3D12Resource> mVertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW mVertexBufferView;

	//頂点バッファの情報
	const VertexBufferDescription mDesc;
	const UINT mSizeInBytes;
	const UINT mStrideInBytes;

};

