#include "pch.h"
#include "Builder/EngineResourceFactory.h"
#include "Builder/RootSignatureBuilder.h"
#include "Memory/StructuredBuffer.h"
#include "Memory/RWStructuredBuffer.h"
#include "Compute/ComputeShader.h"
#include "Memory/DescriptorHeap.h"

std::shared_ptr<StructuredBuffer> EngineResourceFactory::createStructuredBuffer(UINT numElements, UINT sizeOfElement)
{
	return std::shared_ptr<StructuredBuffer>(new StructuredBuffer(*mDevice, mCommandManager, numElements, sizeOfElement));
}

std::shared_ptr<RWStructuredBuffer> EngineResourceFactory::createRWStructuredBuffer(UINT numElements, UINT sizeOfElement)
{
	return std::shared_ptr<RWStructuredBuffer>(new RWStructuredBuffer(*mDevice, mCommandManager, numElements, sizeOfElement));
}

EngineResourceFactory::EngineResourceFactory(ID3D12Device& device, DescriptorHeap& shaderVisibleHeap,  CommandManager& commandManager):
	mDevice(&device),
	mShaderVisibleHeap(&shaderVisibleHeap),
	mCommandManager(commandManager)
{
}
