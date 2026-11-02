#pragma once
class IStructuredBuffer
{
public:
	IStructuredBuffer() = default;
	virtual ~IStructuredBuffer();

	virtual void copyFrom(IStructuredBuffer& copySrc) = 0;
	virtual void copyFrom(class IRWStructuredBuffer& copySrc) = 0;
	virtual void uploadToGPU(void* srcData, UINT srcOffset, UINT dstOffset, UINT sizeInBytes) = 0;
	virtual void uploadToCPU(void* srcData, UINT srcOffset, UINT dstOffset, UINT sizeInBytes) = 0;

};