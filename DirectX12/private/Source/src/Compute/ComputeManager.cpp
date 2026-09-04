#include "pch.h"
#include "Compute/ComputeManager.h"
#include "Builder/RootSignatureBuilder.h"
#include "Compute/ComputeShader.h"
#include "Memory/DescriptorHeap.h"
#include "Command/Command.h"
#include "Graphic/Core/Fence.h"

ComputeManager::~ComputeManager() = default;
std::unique_ptr<Fence> ComputeManager::sFence = nullptr;

void ComputeManager::Initialize(ID3D12Device* device, DescriptorHeap* shaderVisibleHeap, DescriptorHeap* shaderNonVisibleHeap, CommandManager* commandManager)
{
	//再度初期化することは出来ません
	assert(sIsInitialized != true);

	sDevice = device;
	sShaderVisibleHeap = shaderVisibleHeap;
	sShaderNonVisibleHeap = shaderNonVisibleHeap;
	sCommandManager = commandManager;
	createRootSignature();
	sFence = std::make_unique<Fence>(*device);

	sIsInitialized = true;

}

std::shared_ptr<class ComputeShader> ComputeManager::CreateComputeShader(const std::string& filePath)
{
	return std::shared_ptr<ComputeShader>(new ComputeShader(*sDevice, *sRootSignature.Get(), *sShaderVisibleHeap, *sShaderNonVisibleHeap, *sCommandManager, filePath));
}

void ComputeManager::Execute()
{
	if (!sIsInitialized) return;

	auto& list = sCommandManager->getComputeCommandList();
	auto& queue = sCommandManager->getCommandQueue();
	auto& allocator = sCommandManager->getComputeCommandAllocator();

    list->Close();
    ID3D12CommandList* lists[] = { list.Get()};
    queue->ExecuteCommandLists(1, lists);

	//GPUの処理を待機する
	sFence->waitGPU(*sCommandManager->getCommandQueue().Get());

	//コマンドをリセット
	allocator->Reset();
	list->Reset(allocator.Get(), nullptr);

}

void ComputeManager::createRootSignature() {
	sRootSignature = RootSignatureBuilder()
		.addRootConstants(0, 4, D3D12_SHADER_VISIBILITY_ALL)
		.addSRVTable(0, 4, D3D12_SHADER_VISIBILITY_ALL)
		.addUAVTable(0, 4, D3D12_SHADER_VISIBILITY_ALL)
		.build(*sDevice);
}
