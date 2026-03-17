#pragma once
#include <fbxsdk.h>
#include <DirectXMath.h>
#include <cmath>
#define PI 3.141592

using namespace DirectX;

extern XMFLOAT3 normalZ;

XMFLOAT3 operator+(const XMFLOAT3& v1, const XMFLOAT3& v2);
XMFLOAT3 operator-(const XMFLOAT3& v1, const XMFLOAT3& v2);
XMFLOAT3 operator*(const XMFLOAT3& v1, const float& val);

class Math {
public:
	static float length(const XMFLOAT3& v);
	static XMFLOAT3 rotateY(const XMFLOAT3& v1, const float& rot);
	static XMFLOAT3 rotateX(const XMFLOAT3& v1, const float& rot);
	static XMFLOAT3 rotateZ(const XMFLOAT3& v1, const float& rot);
	static XMFLOAT3 normalize(const XMFLOAT3& v);
	static XMFLOAT3 lerp(const XMFLOAT3& start, const XMFLOAT3& end, const float& ratio);

	static FbxVector4 translate(const FbxVector4& v, const FbxVector4& translation);

	static FbxVector4 rotateX(const FbxVector4& v1, const FbxDouble& rot);
	static FbxVector4 rotateY(const FbxVector4& v1, const FbxDouble& rot);
	static FbxVector4 rotateZ(const FbxVector4& v1, const FbxDouble& rot);

	static FbxVector4 scale(const FbxVector4& v, const FbxVector4& scale);

	static FbxVector4 normalize(const FbxVector4& v);
};
