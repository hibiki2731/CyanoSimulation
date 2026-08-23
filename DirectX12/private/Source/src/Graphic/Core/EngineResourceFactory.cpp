#include "pch.h"
#include "Graphic/Core/EngineResourceFactory.h"
#include "Memory/StructuredBuffer.h"
#include "Memory/RWStructuredBuffer.h"

std::shared_ptr<StructuredBuffer> EngineResourceFactory::createStructuredBuffer(UINT numElements, UINT sizeOfElement)
{
	return std::shared_ptr<StructuredBuffer>(new StructuredBuffer(mDevice, mCopyCommand, numElements, sizeOfElement));
}

std::shared_ptr<RWStructuredBuffer> EngineResourceFactory::createRWStructuredBuffer(UINT numElements, UINT sizeOfElement)
{
	return std::shared_ptr<RWStructuredBuffer>(new RWStructuredBuffer(mDevice, mCopyCommand, numElements, sizeOfElement));
}

EngineResourceFactory::EngineResourceFactory(ID3D12Device& device, class Command& graphicsCommand, class Command& copyCommand):
	mDevice(device),
	mGraphicsCommand(graphicsCommand),
	mCopyCommand(copyCommand)
{
}
