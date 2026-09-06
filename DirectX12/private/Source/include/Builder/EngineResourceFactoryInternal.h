#pragma once
#include <d3dx12.h>

void InitializeEngineResourceFactory(ID3D12Device& device, class DescriptorHeap& heap, class CommandManager& command);

void ShutDownEngineResourceFactory();
