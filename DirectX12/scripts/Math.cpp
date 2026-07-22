#include "Math.h"

XMFLOAT3 normalZ = {0 ,0, 1};

XMFLOAT4 operator+(const XMFLOAT4& v1, const XMFLOAT4& v2)
{
	auto vec1 = XMLoadFloat4(&v1);
	auto vec2 = XMLoadFloat4(&v2);

	auto vResult = vec1 + vec2;
	XMFLOAT4 result;
	XMStoreFloat4(&result, vResult);
	return std::move(result);
}

XMFLOAT4 operator-(const XMFLOAT4& v1, const XMFLOAT4& v2)
{
	auto vec1 = XMLoadFloat4(&v1);
	auto vec2 = XMLoadFloat4(&v2);

	auto vResult = vec1 - vec2;
	XMFLOAT4 result;
	XMStoreFloat4(&result, vResult);
	return std::move(result);
}

XMFLOAT4 operator*(const XMFLOAT4& v1, const float& val)
{
	auto vec1 = XMLoadFloat4(&v1);
	auto vResult = vec1 * val;
	XMFLOAT4 result;
	XMStoreFloat4(&result, vResult);
	return std::move(result);
}

XMFLOAT4 operator/(const XMFLOAT4& v1, const float& val)
{
	auto vec1 = XMLoadFloat4(&v1);
	auto vResult = vec1 / val;
	XMFLOAT4 result;
	XMStoreFloat4(&result, vResult);
	return std::move(result);
}

XMFLOAT3 operator+(const XMFLOAT3& v1, const XMFLOAT3& v2)
{
	auto vec1 = XMLoadFloat3(&v1);
	auto vec2 = XMLoadFloat3(&v2);

	auto vResult = vec1 + vec2;
	XMFLOAT3 result;
	XMStoreFloat3(&result, vResult);
	return std::move(result);
}
XMFLOAT3 operator-(const XMFLOAT3& v1, const XMFLOAT3& v2)
{
	auto vec1 = XMLoadFloat3(&v1);
	auto vec2 = XMLoadFloat3(&v2);

	auto vResult = vec1 - vec2;
	XMFLOAT3 result;
	XMStoreFloat3(&result, vResult);
	return std::move(result);
}

XMFLOAT3 operator*(const XMFLOAT3& v1, const float& val) {
	auto vec1 = XMLoadFloat3(&v1); 
	auto vResult = vec1 * val;
	XMFLOAT3 result;
	XMStoreFloat3(&result, vResult);
	return std::move(result);
}

XMFLOAT3 operator/(const XMFLOAT3& v1, const float& val)
{
	auto vec1 = XMLoadFloat3(&v1); 
	auto vResult = vec1 / val;
	XMFLOAT3 result;
	XMStoreFloat3(&result, vResult);
	return std::move(result);
}

XMFLOAT2 operator+(const XMFLOAT2& v1, const XMFLOAT2& v2)
{
	auto vec1 = XMLoadFloat2(&v1);
	auto vec2 = XMLoadFloat2(&v2);

	auto vResult = vec1 + vec2;
	XMFLOAT2 result;
	XMStoreFloat2(&result, vResult);
	return std::move(result);
}

XMFLOAT2 operator-(const XMFLOAT2& v1, const XMFLOAT2& v2)
{
	auto vec1 = XMLoadFloat2(&v1);
	auto vec2 = XMLoadFloat2(&v2);

	auto vResult = vec1 - vec2;
	XMFLOAT2 result;
	XMStoreFloat2(&result, vResult);
	return std::move(result);
}

XMFLOAT2 operator*(const XMFLOAT2& v1, const float& val)
{
	auto vec1 = XMLoadFloat2(&v1);

	auto vResult = vec1 * val;
	XMFLOAT2 result;
	XMStoreFloat2(&result, vResult);
	return std::move(result);
}

XMFLOAT2 operator/(const XMFLOAT2& v1, const float& val)
{
	auto vec1 = XMLoadFloat2(&v1);

	auto vResult = vec1 / val;
	XMFLOAT2 result;
	XMStoreFloat2(&result, vResult);
	return std::move(result);
}

float Math::length(const XMFLOAT2& v)
{
	XMVECTOR vec = XMLoadFloat2(&v);
	XMVECTOR length = XMVector2Length(vec);
	
	return XMVectorGetX(length);
}

float Math::distance(const XMFLOAT2& v1, const XMFLOAT2& v2)
{
	XMVECTOR vec1 = XMLoadFloat2(&v1);
	XMVECTOR vec2 = XMLoadFloat2(&v2);
	XMVECTOR sub = XMVectorSubtract(vec1, vec2);
	XMVECTOR length = XMVector2Length(sub);
	return XMVectorGetX(length);
}

float Math::length(const XMFLOAT3& v)
{
	XMVECTOR vec = XMLoadFloat3(&v);
	XMVECTOR length = XMVector3Length(vec);
	
	return XMVectorGetX(length);
}

XMFLOAT3 Math::rotateY(const XMFLOAT3& v1, const float& rot)
{
	//レジスタへロード
	XMVECTOR vec = XMLoadFloat3(&v1);
	//回転行列の生成
	XMMATRIX rotate = XMMatrixRotationY(rot);

	//行列の積を計算
	XMVECTOR vResult = XMVector3TransformCoord(vec, rotate);

	//XMFLOAT3へ戻す
	XMFLOAT3 result;
	XMStoreFloat3(&result, vResult);

	return result;
}

XMFLOAT3 Math::rotateX(const XMFLOAT3& v1, const float& rot)
{
	//レジスタへロード
	XMVECTOR vec = XMLoadFloat3(&v1);
	//回転行列の生成
	XMMATRIX rotate = XMMatrixRotationX(rot);

	//行列の積を計算
	XMVECTOR vResult = XMVector3TransformCoord(vec, rotate);

	//XMFLOAT3へ戻す
	XMFLOAT3 result;
	XMStoreFloat3(&result, vResult);

	return result;
}

XMFLOAT3 Math::rotateZ(const XMFLOAT3& v1, const float& rot)
{
	//レジスタへロード
	XMVECTOR vec = XMLoadFloat3(&v1);
	//回転行列の生成
	XMMATRIX rotate = XMMatrixRotationZ(rot);

	//行列の積を計算
	XMVECTOR vResult = XMVector3TransformCoord(vec, rotate);

	//XMFLOAT3へ戻す
	XMFLOAT3 result;
	XMStoreFloat3(&result, vResult);

	return result;
}

XMFLOAT3 Math::rotate(const XMFLOAT3& v1, const XMFLOAT3& rot)
{
	//レジスタへロード
	XMVECTOR vec = XMLoadFloat3(&v1);
	//回転行列の生成
	XMMATRIX rotate = XMMatrixRotationRollPitchYaw(rot.x, rot.y, rot.z);

	//行列の積を計算
	XMVECTOR vResult = XMVector3TransformCoord(vec, rotate);

	//XMFLOAT3へ戻す
	XMFLOAT3 result;
	XMStoreFloat3(&result, vResult);

	return result;
}

XMFLOAT3 Math::normalize(const XMFLOAT3& v)
{
	//レジスタへロード
	XMVECTOR vec = XMLoadFloat3(&v);
	//正規化
	XMVECTOR n = XMVector3Normalize(vec);
	//XMFLOAT3に戻す
	XMFLOAT3 result;
	XMStoreFloat3(&result, n);

	return result;
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
