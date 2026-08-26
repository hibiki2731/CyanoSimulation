#pragma once
class IComputeShader
{
public:
	virtual void dispatch(UINT threadGroupCountX, UINT threadGroupCountY, UINT threadGroupCountZ) = 0;
	virtual void setStructuredBuffer(class IStructuredBuffer& buffer, UINT position) = 0;
	virtual void setRWStructuredBuffer(class IRWStructuredBuffer& buffer, UINT position) = 0;
	virtual void setRootConstants(void* dataSrc) = 0;
	virtual void waitWriteBuffer(class IRWStructuredBuffer& buffer) = 0;
	virtual void clearRWStructuredBuffer(UINT position) = 0;
};

