#pragma once
#include <DirectXMath.h>
using namespace DirectX;

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

enum class TurnType {
	PLAYER,
	ENEMY,
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

//3D用バッファ
struct Base3DData {
	XMMATRIX viewProj;  //ビュー射影行列
	XMFLOAT4 cameraPos; //xyz:カメラ位置、w:未使用
	PointLightData pointLights[MAX_LIGHT_NUM];  //ポイントライト配列
	SpotLightData spotLights[MAX_LIGHT_NUM];  //スポットライト配列
	float playerFlashIntensity; //ダメージを受けたときの点滅フラグ
	XMFLOAT3 playerFlashColor;
};

//3Dオブジェクトのワールド行列
struct World3DConstBuf {
	XMMATRIX world;  //ワールド行列
	XMFLOAT3 flashColor;
	float flashIntensity;
};

//メッシュごとのマテリアル情報
struct MaterialConstBuf {
    XMFLOAT4 ambient;
    XMFLOAT4 diffuse;
    XMFLOAT4 specular;
};

//2D用バッファ
struct SpriteConstBuf {
	XMMATRIX world;
	XMFLOAT2 windowSize; //xy:ウィンドウサイズ
	XMFLOAT2 spriteSize; //xy:サイズ (px)
	XMFLOAT2 textureSize; //xy:テクスチャサイズ (px)
	float bordarSize; //9スライス用ボーダーサイズ (px)
	float padding; //パディング
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
