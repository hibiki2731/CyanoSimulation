#pragma once
//Windows API
#include <wrl/client.h>
#include <DirectXMath.h>

using namespace DirectX;
using Microsoft::WRL::ComPtr;

constexpr int MAX_LIGHT_NUM = 16;
constexpr float MAPTIPSIZE = 2;
constexpr float EPSILON = 0.0001f;

//左上右下
enum Direction {
	DOWN = 1,
	RIGHT = 2,
	UP = 4,
	LEFT = 8,
};

enum LightStat {
    INACTIVE = 0,
    ACTIVE = 1,
};

struct PointLightData {
	XMFLOAT4 position; //xyz:位置、w:有効無効
    XMFLOAT4 color; //xyzw:rgba
    XMFLOAT4 setValue; //x:有効無効、y:光強度、z:光の届く範囲
};


struct SpotLightData {

    XMFLOAT4 position; //xyz:座標
    XMFLOAT4 direction; //xyz:向き
    XMFLOAT4 color; //xyz:rgb w:α値
    XMFLOAT4 setValue; //x:有効無効 y:光強度 z:光の届く距離
    XMFLOAT4 attAngle; //x:角度減衰が起こらない範囲 y:ライトがあたる範囲

};
//フォント用バッファ
struct FontConstBuffer {
	XMMATRIX world;
    XMFLOAT4 effect; //x:色
};

//ビルボード処理用コンスタントバッファ
struct BillboardConstBuf {
	XMMATRIX view;  //ビュー行列
	XMMATRIX proj;  //プロジェクション行列
};
