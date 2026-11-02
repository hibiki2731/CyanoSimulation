#pragma once
#include <vector>
#include <memory>
using Microsoft::WRL::ComPtr;

class IEngineResourceFactory {
public:
	virtual ~IEngineResourceFactory() = default;

	virtual std::unique_ptr<class IStructuredBuffer> createStructuredBuffer(UINT numElements, UINT sizeOfElement) = 0;
	virtual std::unique_ptr<class IRWStructuredBuffer> createRWStructuredBuffer(UINT numElements, UINT sizeOfElement) = 0;
};

IEngineResourceFactory& GetEngineResourceFactory();
