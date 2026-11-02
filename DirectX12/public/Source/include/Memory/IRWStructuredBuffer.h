#pragma once
class IRWStructuredBuffer
{
public:
	IRWStructuredBuffer() = default;
	virtual ~IRWStructuredBuffer();

	virtual void copyFrom(IRWStructuredBuffer& copySrc) = 0;
	virtual void copyFrom(class IStructuredBuffer& copySrc) = 0;
	virtual void uploadToGPU(void* srcData, UINT srcOffset, UINT dstOffset, UINT sizeInBytes) = 0;
	virtual void uploadToCPU(void* srcData, UINT srcOffset, UINT dstOffset, UINT sizeInBytes) = 0;
	virtual void* read() = 0;
};

