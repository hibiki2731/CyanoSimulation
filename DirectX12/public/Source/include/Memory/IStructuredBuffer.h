#pragma once
class IStructuredBuffer
{
public:
	IStructuredBuffer() = default;
	~IStructuredBuffer() = default;

	virtual void upload(void* srcData, UINT sizeInBytes) = 0;

};

