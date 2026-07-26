struct VS_INPUT
{
    float2 position : POSITION;
    float2 uv : TEXCOORD0;
    uint instatnceID : SV_InstanceID;
};

struct PS_INPUT
{
    float4 position : SV_Position;
    float2 uv : TEXCOORD0;
};

struct POINT
{
    float4 position;
};

RWStructuredBuffer<POINT> g_points : register(u0);

cbuffer b0 : register(b0)
{
    float CyanoPointSize;
    float2 WindowSize;
};
