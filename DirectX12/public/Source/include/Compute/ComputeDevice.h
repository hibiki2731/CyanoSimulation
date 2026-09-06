#pragma once
#include <memory>
#include <string>
#include "Compute/IComputeShader.h"

enum class ComputeShaderFormat {
	S4_RW4,
	RW_12
};

class IComputeDevice {
public:
	virtual ~IComputeDevice() = default;

	virtual std::unique_ptr<IComputeShader> createComputeShader(const std::string& filePath, ComputeShaderFormat format = ComputeShaderFormat::S4_RW4) = 0;
	virtual void executeAndWaitGPU() = 0;
};

IComputeDevice& GetComputeDevice();
