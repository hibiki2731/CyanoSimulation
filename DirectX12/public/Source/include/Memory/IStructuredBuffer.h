#pragma once
class IStructuredBuffer
{
public:
	IStructuredBuffer() = default;
	virtual ~IStructuredBuffer();

	virtual void upload(void* srcData, UINT sizeInBytes) = 0;

};

