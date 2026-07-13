#pragma once
#include "Definition.h"

//Windows API
#include <wrl/client.h>
#include <DirectXMath.h>
#include "directx/d3dx12.h"

class IndexBuffer
{
public:
	IndexBuffer(ID3D12Device& device, const std::vector<UINT16>& data);

	const D3D12_INDEX_BUFFER_VIEW& getView() const { return mIndexBufferView; }
	
private:
	void createBuf(ID3D12Device& device);
	D3D12_HEAP_PROPERTIES createHeapProperties();
	D3D12_RESOURCE_DESC createResourceDesc();
	void setData(const std::vector<UINT16>& src);

	void createView();

	ComPtr<ID3D12Resource> mIndexBuffer;
	D3D12_INDEX_BUFFER_VIEW mIndexBufferView;

	//インデックスバッファの情報
	const UINT mSizeInBytes;

};

