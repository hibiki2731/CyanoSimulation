#include "pch.h"
#include "Builder/EngineResourceFactory.h"
#include "Builder/RootSignatureBuilder.h"
#include "Memory/StructuredBuffer.h"
#include "Memory/RWStructuredBuffer.h"
#include "Compute/ComputeShader.h"
#include "Memory/DescriptorHeap.h"

std::shared_ptr<StructuredBuffer> EngineResourceFactory::createStructuredBuffer(UINT numElements, UINT sizeOfElement)
{
	return std::shared_ptr<StructuredBuffer>(new StructuredBuffer(mDevice, mCopyCommand, numElements, sizeOfElement));
}

std::shared_ptr<RWStructuredBuffer> EngineResourceFactory::createRWStructuredBuffer(UINT numElements, UINT sizeOfElement)
{
	return std::shared_ptr<RWStructuredBuffer>(new RWStructuredBuffer(mDevice, mCopyCommand, numElements, sizeOfElement));
}

std::shared_ptr<ComputeShader> EngineResourceFactory::createComputeShader(const std::string& shaderFilePath)
{
	return std::make_shared<ComputeShader>(mDevice, mComputeKit.getRootSignature(), mShaderVisibleHeap, mComputeKit.getDescriptorHeap(), mComputeKit.getCommand(), shaderFilePath);
}

EngineResourceFactory::EngineResourceFactory(ID3D12Device& device, class DescriptorHeap& shaderVisibleHeap, class DescriptorHeap& shaderNonVisibleHeap,class Command& graphicsCommand, class Command& copyCommand, class Command& computeCommand):
	mDevice(device),
	mShaderVisibleHeap(shaderVisibleHeap),
	mGraphicsCommand(graphicsCommand),
	mCopyCommand(copyCommand),
	mComputeKit(device, shaderNonVisibleHeap, computeCommand)
{
}

void EngineResourceFactory::ComputeKit::createRootSignature()
{
	mRootSignature = RootSignatureBuilder()
		.addRootConstants(0, 4, D3D12_SHADER_VISIBILITY_ALL)
		.addSRVTable(0, 4, D3D12_SHADER_VISIBILITY_ALL)
		.addUAVTable(0, 4, D3D12_SHADER_VISIBILITY_ALL)
		.build(mDevice);

	isInitialized = true;
}
