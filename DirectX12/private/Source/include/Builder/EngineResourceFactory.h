#pragma once
#include <d3dx12.h>
#include <vector>
#include <memory>
using Microsoft::WRL::ComPtr;

class EngineResourceFactory
{
public:
	std::shared_ptr<class StructuredBuffer> createStructuredBuffer(UINT numElements, UINT sizeOfElement);
	std::shared_ptr<class RWStructuredBuffer> createRWStructuredBuffer(UINT numElements, UINT sizeOfElement);
	std::shared_ptr<class ComputeShader> createComputeShader(const std::string& shaderFilePath);

private:
	friend class Game;
	EngineResourceFactory(ID3D12Device& device, class DescriptorHeap& shaderVisibleHeap, class DescriptorHeap& shaderNonVisibleHeap,class Command& graphicsCommand, class Command& copyCommand, class Command& computeCommand);
	ID3D12Device& mDevice;
	class DescriptorHeap& mShaderVisibleHeap;
	class Command& mGraphicsCommand;
	class Command& mCopyCommand;

	class ComputeKit {
	public:
		ComputeKit(ID3D12Device& device, class DescriptorHeap& heap, class Command& command) :
			mDevice(device),
			mShaderNonVisibleHeap(heap),
			mComputeCommand(command)
		{
		}

		DescriptorHeap& getDescriptorHeap() { return mShaderNonVisibleHeap; }
		Command& getCommand() { return mComputeCommand; }
		ID3D12RootSignature& getRootSignature() {
			if (!isInitialized) createRootSignature();
			return *mRootSignature.Get();
		}
		
		void createRootSignature();

	private:
		class ID3D12Device& mDevice;
		class DescriptorHeap& mShaderNonVisibleHeap;
		class Command& mComputeCommand;
		ComPtr<ID3D12RootSignature> mRootSignature;
		bool isInitialized = false;
	};

	ComputeKit mComputeKit;

};

