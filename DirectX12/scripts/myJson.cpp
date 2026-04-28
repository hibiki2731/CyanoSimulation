#include "myJson.h"


	//XMFLOAT2をjsonから取得
	void DirectX::from_json(const json& j, XMFLOAT2& p) {
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
	void DirectX::from_json(const json& j, XMFLOAT3& p) {
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
	void DirectX::from_json(const json& j, XMFLOAT4& p) {
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

	void DirectX::to_json(json& j, const XMFLOAT2& p) {
		j = json::array({ p.x, p.y });
	}
	void DirectX::to_json(json& j, const XMFLOAT3& p) {
		j = json::array({ p.x, p.y, p.z });
	}
	void DirectX::to_json(json& j, const XMFLOAT4& p) {
		j = json::array({ p.x, p.y, p.z, p.w });
	}
