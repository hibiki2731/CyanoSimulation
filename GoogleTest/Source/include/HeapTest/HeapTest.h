#pragma once
#include "gtest/gtest.h"
#include <d3dx12.h>
#include <DirectXMath.h>
#include "Memory/UploadHeap.h"
#include "Graphic/Core/GraphicDeviceBuilder.h"
#include "Graphic/Core/CommandAllocatorBuilder.h"
#include "Graphic/Core/CommandListBuilder.h"
#include "Graphic/Core/CommandQueueBuilder.h"

class GraphicCoreTest : public ::testing::Test {
protected:
	virtual void SetUp() {
		mDevice = GraphicDeviceBuilder().build();
		mCommandAllocator = CommandAllocatorBuilder().build(*mDevice.Get());
		mCommandList = CommandListBuilder().setCommandAllocator(mCommandAllocator).build(*mDevice.Get());
		mCommandQueue = CommandQueueBuilder().build(*mDevice.Get());
	}

	ComPtr<ID3D12Device> mDevice;
	ComPtr<ID3D12CommandAllocator> mCommandAllocator;
	ComPtr<ID3D12GraphicsCommandList> mCommandList;
	ComPtr<ID3D12CommandQueue> mCommandQueue;


};


