#pragma once

class IRWStructuredBuffer
{
public:
	IRWStructuredBuffer() = default;
	virtual ~IRWStructuredBuffer();

	virtual void upload(void* srcData, UINT sizeInBytes) = 0;
	virtual void* read() = 0;
};

