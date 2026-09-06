#pragma once
#include <d3d12.h>

void InitializeComputeDevice(ID3D12Device* device, class DescriptorHeap* shaderVisibleHeap, class DescriptorHeap* shaderNonVisibleHeap, class CommandManager* commandManager);
void ShutDownComputeDevice();
void ExecuteComputeDevice();
