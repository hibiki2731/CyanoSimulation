#pragma once

class IStructuredBuffer;
class IRWStructuredBuffer;

enum class ComputeShaderFormat {
	C4S4RW4,
	C14S8RW8
};

class IComputeShader
{
public:
	virtual ~IComputeShader() = default;
	virtual void dispatch(UINT threadGroupCountX, UINT threadGroupCountY, UINT threadGroupCountZ) = 0;
	virtual void setStructuredBuffer(IStructuredBuffer& buffer, UINT position) = 0;
	virtual void setRWStructuredBuffer(IRWStructuredBuffer& buffer, UINT position) = 0;
	virtual void setRootConstants(void* dataSrc) = 0;
	virtual void waitWriteBuffer(UINT position) = 0;
	virtual void clearRWStructuredBuffer(UINT position) = 0;
};

