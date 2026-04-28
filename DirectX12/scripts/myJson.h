#pragma once
#include "json.hpp"
#include <DirectXMath.h>

using json = nlohmann::json;
using namespace DirectX;

namespace DirectX {
	//XMFLOAT2をjsonから取得
	void from_json(const json& j, XMFLOAT2& p);

	//XMFLOAT3をjsonから取得
	void from_json(const json& j, XMFLOAT3& p);

	//XMFLAOT4をjsonから取得
	void from_json(const json& j, XMFLOAT4& p);
	void to_json(json& j, const XMFLOAT2& p);
	void to_json(json& j, const XMFLOAT3& p);
	void to_json(json& j, const XMFLOAT4& p);
}

