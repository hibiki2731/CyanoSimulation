#pragma once
#include <d3dx12.h>
#include <memory>
using Microsoft::WRL::ComPtr;

class DescriptorHeap;
class CommandManager;

class ComputeManager
{
public:
	~ComputeManager();

	static void Initialize(ID3D12Device* device, DescriptorHeap* shaderVisibleHeap, DescriptorHeap* shaderNonVisibleHeap, CommandManager* commandManager);
	static std::shared_ptr<class ComputeShader> CreateComputeShader(const std::string& filePath);
	static void Execute();

private:
	inline static ID3D12Device* sDevice;
	inline static DescriptorHeap*sShaderVisibleHeap;
	inline static DescriptorHeap* sShaderNonVisibleHeap;
	inline static CommandManager* sCommandManager;
	inline static ComPtr<ID3D12RootSignature> sRootSignature;
	inline static bool sIsInitialized = false;
	static std::unique_ptr<class Fence> sFence;

	static void createRootSignature();
};

