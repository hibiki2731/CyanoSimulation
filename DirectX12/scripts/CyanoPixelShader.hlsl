#include "CyanoHeader.hlsli"

Texture2D<float4> Texture : register(t0);
SamplerState Sampler : register(s0);

float4 main(PS_INPUT input) : SV_TARGET
{
    //float4 output = Texture.Sample(Sampler, input.uv);
    return float4(1.0f, 1.0f, 1.0f, 0.05f);
}