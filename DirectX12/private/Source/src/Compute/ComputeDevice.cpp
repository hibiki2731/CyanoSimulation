#include "pch.h"
#include "Compute/ComputeDevice.h"
#include "Compute/ComputeDeviceInternal.h"
#include "Builder/RootSignatureBuilder.h"
#include "Compute/ComputeShader.h"
#include "Memory/DescriptorHeap.h"
#include "Command/Command.h"
#include "Graphic/Core/Fence.h"

class ComputeDevice : public IComputeDevice
{
public:
	ComputeDevice(ID3D12Device* device, DescriptorHeap* shaderVisibleHeap, DescriptorHeap* shaderNonVisibleHeap, CommandManager* commandManager)
		:mDevice(device), mShaderVisibleHeap(shaderVisibleHeap), mShaderNonVisibleHeap(shaderNonVisibleHeap), mCommandManager(commandManager), mFence(std::make_unique<Fence>(*device))
	{
		createRootSignature();
	}

	std::unique_ptr<IComputeShader> createComputeShader(const std::string& filePath, ComputeShaderFormat format) override 
	{
		return std::unique_ptr<ComputeShader>(new ComputeShader(*mDevice, *mRootSignature.Get(), *mShaderVisibleHeap, *mShaderNonVisibleHeap, *mCommandManager, filePath, format));
	}

	void executeAndWaitGPU() override 
	{
		auto& list = mCommandManager->getComputeCommandList();
		auto& queue = mCommandManager->getCommandQueue();
		auto& allocator = mCommandManager->getComputeCommandAllocator();

		list->Close();
		ID3D12CommandList* lists[] = { list.Get() };
		queue->ExecuteCommandLists(1, lists);

		//GPUの処理を待機する
		mFence->waitGPU(*mCommandManager->getCommandQueue().Get());

		//コマンドをリセット
		allocator->Reset();
		list->Reset(allocator.Get(), nullptr);

	}

	void execute() {
		auto& list = mCommandManager->getComputeCommandList();
		auto& queue = mCommandManager->getCommandQueue();
		auto& allocator = mCommandManager->getComputeCommandAllocator();

		list->Close();
		ID3D12CommandList* lists[] = { list.Get() };
		queue->ExecuteCommandLists(1, lists);

		//GPUの待機はGraphicsの描画待機と同時に行う
		//アロケータのリセットもそこで行う
	}

private:
	ID3D12Device* mDevice;
	DescriptorHeap* mShaderVisibleHeap;
	DescriptorHeap* mShaderNonVisibleHeap;
	CommandManager* mCommandManager;
	ComPtr<ID3D12RootSignature> mRootSignature;
	std::unique_ptr<class Fence> mFence;

	void createRootSignature()
	{
		mRootSignature = RootSignatureBuilder()
			.addRootConstants(0, 4, D3D12_SHADER_VISIBILITY_ALL)
			.addSRVTable(0, 4, D3D12_SHADER_VISIBILITY_ALL)
			.addUAVTable(0, 4, D3D12_SHADER_VISIBILITY_ALL)
			.build(*mDevice);
	}
};

static std::unique_ptr<ComputeDevice> sComputeDeviceInstance = nullptr;

void InitializeComputeDevice(ID3D12Device* device, DescriptorHeap* shaderVisibleHeap, DescriptorHeap* shaderNonVisibleHeap, CommandManager* commandManager)
{
	if (!sComputeDeviceInstance)
		sComputeDeviceInstance = std::make_unique<ComputeDevice>(device, shaderVisibleHeap, shaderNonVisibleHeap, commandManager);
}

void ShutDownComputeDevice() {
	sComputeDeviceInstance.reset();
}

void ExecuteComputeDevice() {
	sComputeDeviceInstance->execute();
}

IComputeDevice& GetComputeDevice()
{
	if(!sComputeDeviceInstance) 
		throw std::runtime_error("ComputeDevice is not initialized");

	return *sComputeDeviceInstance;
}
