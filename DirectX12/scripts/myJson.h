#pragma once
#include "json_fwd.hpp"
#include <DirectXMath.h>
#include <d2d1.h>
#include <d2d1helper.h>

using namespace DirectX;

namespace DirectX {
	//XMFLOAT2
	void from_json(const nlohmann::json& j, XMFLOAT2& p);
	void to_json(nlohmann::json& j, const XMFLOAT2& p);

	//XMFLOAT3
	void from_json(const nlohmann::json& j, XMFLOAT3& p);
	void to_json(nlohmann::json& j, const XMFLOAT3& p);

	//XMFLAOT4
	void from_json(const nlohmann::json& j, XMFLOAT4& p);
	void to_json(nlohmann::json& j, const XMFLOAT4& p);

}

namespace D2D1 {
	//D2D1::ColorF
	void from_json(const nlohmann::json& j, D2D1::ColorF& p);
	void to_json(nlohmann::json& j, const D2D1::ColorF& p);
}
void from_json(const nlohmann::json& j, D2D1_COLOR_F& c);
