struct PointLight
{
    float4 position; //xyz:座標
    float4 color; //xyz:rgb w:α値
    float4 setValue; //x:有効無効 y:光強度 z:光の届く距離
};

struct SpotLight
{
    float4 position; //xyz:座標
    float4 direction; //xyz:向き
    float4 color; //xyz:rgb w:α値
    float4 setValue; //x:有効無効 y:光強度 z:光の届く距離
    float4 attAngle; //x:角度減衰が起こらない範囲 y:ライトがあたる範囲
    
};

static const int NUM_LIGHTS = 16;
static const float PI = 3.14159265359;

cbuffer b0 : register(b0)
{
    matrix ViewProj;
    float4 CameraPos;
    PointLight pointLights[NUM_LIGHTS];
    SpotLight spotLights[NUM_LIGHTS];
    float playerFlashIntensity;
    float3 playerFlashColor;
}

cbuffer b1 : register(b1)
{
    matrix World;
    float3 FlashColor;
    float FlashIntensity;
}
cbuffer b2 : register(b2)
{
    float4 Ambient;
    float4 Diffuse;
    float4 Specular;
}

Texture2D<float4> Texture : register(t0); //テクスチャ0盤
SamplerState Sampler : register(s0); //サンプラ0番
