#include "myJson.h"
#include "json.hpp"

//XMFLOAT2をjsonから取得
void DirectX::from_json(const nlohmann::json& j, XMFLOAT2& p) {
	//JSONが配列[x, y, z]の形式を想定する場合
	if (j.is_array() && j.size() == 2) {
		p.x = j.at(0).get<float>();
		p.y = j.at(1).get<float>();
	}
	//JSONがオブジェクト{"x": 0, "y":0}の形式を想定する場合
	else {
		j.at("x").get_to(p.x);
		j.at("y").get_to(p.y);
	}
}

//XMFLOAT3をjsonから取得
void DirectX::from_json(const nlohmann::json& j, XMFLOAT3& p) {
	//JSONが配列[x, y, z]の形式を想定する場合
	if (j.is_array() && j.size() == 3) {
		p.x = j.at(0).get<float>();
		p.y = j.at(1).get<float>();
		p.z = j.at(2).get<float>();
	}
	//JSONがオブジェクト{"x": 0, "y":0, "z":0}の形式を想定する場合
	else {
		j.at("x").get_to(p.x);
		j.at("y").get_to(p.y);
		j.at("z").get_to(p.z);
	}
}

//XMFLAOT4をjsonから取得
void DirectX::from_json(const nlohmann::json& j, XMFLOAT4& p) {
	//JSONが配列[x, y, z]の形式を想定する場合
	if (j.is_array() && j.size() == 4) {
		p.x = j.at(0).get<float>();
		p.y = j.at(1).get<float>();
		p.z = j.at(2).get<float>();
		p.w = j.at(3).get<float>();
	}
	//JSONがオブジェクト{"x": 0, "y":0, "z":0, "w":0}の形式を想定する場合
	else {
		j.at("x").get_to(p.x);
		j.at("y").get_to(p.y);
		j.at("z").get_to(p.z);
		j.at("w").get_to(p.w);
	}
}

void DirectX::to_json(nlohmann::json& j, const XMFLOAT2& p) {
	j = nlohmann::json::array({ p.x, p.y });
}
void DirectX::to_json(nlohmann::json& j, const XMFLOAT3& p) {
	j = nlohmann::json::array({ p.x, p.y, p.z });
}
void DirectX::to_json(nlohmann::json& j, const XMFLOAT4& p) {
	j = nlohmann::json::array({ p.x, p.y, p.z, p.w });
}

void D2D1::from_json(const nlohmann::json& j, D2D1::ColorF& p)
{
	//JSONが配列[r, g, b, a]の形式を想定する場合
	if (j.is_array() && j.size() == 4) {
		p.r = j.at(0).get<float>();
		p.g = j.at(1).get<float>();
		p.b = j.at(2).get<float>();
		p.a = j.at(3).get<float>();
	}
	//JSONがオブジェクト{"r": 0, "g":0, "b":0, "a":0}の形式を想定する場合
	else {
		p.r = j.at("r").get<float>();
		p.g = j.at("g").get<float>();
		p.b = j.at("b").get<float>();
		p.a = j.at("a").get<float>();
	}
}
void D2D1::to_json(nlohmann::json& j, const D2D1::ColorF& p)
{
	j = nlohmann::json::array({ p.r, p.g, p.b, p.a });
}

void from_json(const nlohmann::json& j, D2D1_COLOR_F& c)
{
	//JSONが配列[r, g, b, a]の形式を想定する場合
	if (j.is_array() && j.size() == 4) {
		c.r = j.at(0).get<float>();
		c.g = j.at(1).get<float>();
		c.b = j.at(2).get<float>();
		c.a = j.at(3).get<float>();
	}
	//JSONがオブジェクト{"r": 0, "g":0, "b":0, "a":0}の形式を想定する場合
	else {
		c.r = j.at("r").get<float>();
		c.g = j.at("g").get<float>();
		c.b = j.at("b").get<float>();
		c.a = j.at("a").get<float>();
	}
}
