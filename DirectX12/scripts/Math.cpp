#include "Math.h"

XMFLOAT3 normalZ = {0 ,0, 1};

XMFLOAT3 operator+(const XMFLOAT3& v1, const XMFLOAT3& v2)
{
    return XMFLOAT3(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z);
}
XMFLOAT3 operator-(const XMFLOAT3& v1, const XMFLOAT3& v2)
{
	return XMFLOAT3(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z);
}

XMFLOAT3 operator*(const XMFLOAT3& v1, const float& val) {
	return XMFLOAT3(v1.x * val, v1.y * val, v1.z * val);
}

float Math::length(const XMFLOAT3& v)
{
	return  std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

XMFLOAT3 Math::rotateY(const XMFLOAT3& v1, const float& rot)
{
	return XMFLOAT3(v1.x * cosf(rot) - v1.z * sinf(rot), v1.y, v1.x * sinf(rot) + v1.z * cosf(rot));
}

XMFLOAT3 Math::rotateX(const XMFLOAT3& v1, const float& rot)
{
	return XMFLOAT3(v1.x, v1.y * cosf(rot) - v1.z * sinf(rot), v1.y * sinf(rot) + v1.z * cosf(rot));
}

XMFLOAT3 Math::rotateZ(const XMFLOAT3& v1, const float& rot)
{
	return XMFLOAT3(v1.x * cosf(rot) - v1.y * sinf(rot), v1.x * sinf(rot) + v1.y * cosf(rot),  v1.z);
}

XMFLOAT3 Math::normalize(const XMFLOAT3& v)
{
	double length = Math::length(v);
	return XMFLOAT3(v.x / length, v.y / length, v.z / length);
}

XMFLOAT3 Math::lerp(const XMFLOAT3& start, const XMFLOAT3& end, const float& ratio)
{
	return XMFLOAT3(std::lerp(start.x, end.x, ratio), std::lerp(start.y, end.y, ratio), std::lerp(start.z, end.z, ratio));
}

FbxVector4 Math::translate(const FbxVector4& v, const FbxVector4& translation)
{
	return FbxVector4(v[0] + translation[0], v[1] + translation[1], v[2] + translation[2], v[3]);
}

FbxVector4 Math::rotateX(const FbxVector4& v1, const FbxDouble& rot)
{
	return FbxVector4(v1[0], v1[1] * cos(rot) - v1[2] * sin(rot), v1[1] * sin(rot) + v1[2] * cos(rot), v1[3]);
}

FbxVector4 Math::rotateY(const FbxVector4& v1, const FbxDouble& rot)
{
	return FbxVector4(v1[0] * cos(rot) - v1[2] * sin(rot), v1[1], v1[0] * sin(rot) + v1[2] * cos(rot), v1[3]);
}

FbxVector4 Math::rotateZ(const FbxVector4& v1, const FbxDouble& rot)
{
	return FbxVector4(v1[0] * cos(rot) - v1[1] * sin(rot), v1[0] * sin(rot) + v1[1] * cos(rot), v1[2], v1[3]);
}

FbxVector4 Math::scale(const FbxVector4& v, const FbxVector4& scale)
{
	return FbxVector4(v[0] * scale[0], v[1] * scale[1], v[2] * scale[2], v[3]);
}

FbxVector4 Math::normalize(const FbxVector4& v)
{
	double length = std::sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
	return FbxVector4(v[0] / length, v[1] / length, v[2] / length, 1.0);
}
