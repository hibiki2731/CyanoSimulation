#include <DTHeader.hlsli>
Texture2D<float4> Texture : register(t0); //テクスチャ0盤
SamplerState Sampler : register(s0); //サンプラ0番


float4 main(PS_INPUT input) : SV_TARGET
{
    return Texture.Sample(Sampler, input.uv) * float4(1.0f, 0.0f, 0.0f, 1.0f) * input.alpha;

}
