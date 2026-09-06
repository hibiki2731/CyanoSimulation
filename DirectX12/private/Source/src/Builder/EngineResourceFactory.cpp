#include "pch.h"
#include "Builder/EngineResourceFactory.h"
#include "Builder/EngineResourceFactoryInternal.h"
#include "Builder/RootSignatureBuilder.h"
#include "Memory/StructuredBuffer.h"
#include "Memory/RWStructuredBuffer.h"
#include "Memory/DescriptorHeap.h"
#include "Command/Command.h"

class EngineResourceFactory : public IEngineResourceFactory
{
public:
	EngineResourceFactory(ID3D12Device& device, DescriptorHeap& shaderVisibleHeap, CommandManager& commandManager);

	std::unique_ptr<IStructuredBuffer> createStructuredBuffer(UINT numElements, UINT sizeOfElement) override;
	std::unique_ptr<IRWStructuredBuffer> createRWStructuredBuffer(UINT numElements, UINT sizeOfElement) override;

private:
	friend class Graphics;
	ID3D12Device* mDevice;
	class DescriptorHeap* mShaderVisibleHeap;
	class CommandManager& mCommandManager;
};



std::unique_ptr<IStructuredBuffer> EngineResourceFactory::createStructuredBuffer(UINT numElements, UINT sizeOfElement)
{
	return std::unique_ptr<StructuredBuffer>(new StructuredBuffer(*mDevice, mCommandManager, numElements, sizeOfElement));
}

std::unique_ptr<IRWStructuredBuffer> EngineResourceFactory::createRWStructuredBuffer(UINT numElements, UINT sizeOfElement)
{
	return std::unique_ptr<RWStructuredBuffer>(new RWStructuredBuffer(*mDevice, mCommandManager, numElements, sizeOfElement));
}

EngineResourceFactory::EngineResourceFactory(ID3D12Device& device, DescriptorHeap& shaderVisibleHeap,  CommandManager& commandManager):
	mDevice(&device),
	mShaderVisibleHeap(&shaderVisibleHeap),
	mCommandManager(commandManager)
{
}

static std::unique_ptr<EngineResourceFactory> sFactoryInstance = nullptr;

void InitializeEngineResourceFactory(ID3D12Device& device, DescriptorHeap& heap, CommandManager& command) {
	if (!sFactoryInstance) {
		sFactoryInstance = std::make_unique<EngineResourceFactory>(device, heap, command);
	}
}

void ShutDownEngineResourceFactory() {
	sFactoryInstance.reset();
}

IEngineResourceFactory& GetEngineResourceFactory() {
	if (!sFactoryInstance) throw std::runtime_error("EngineResourceFactory is not initialized");

	return *sFactoryInstance;
}
