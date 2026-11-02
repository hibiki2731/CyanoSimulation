#pragma once
#include <memory>
#include <string>
#include "Compute/IComputeShader.h"

class IComputeDevice {
public:
	virtual ~IComputeDevice() = default;

	virtual std::unique_ptr<IComputeShader> createComputeShader(const std::string& filePath, ComputeShaderFormat format = ComputeShaderFormat::C4S4RW4) = 0;
	virtual void executeAndWaitGPU() = 0;
};

IComputeDevice& GetComputeDevice();
