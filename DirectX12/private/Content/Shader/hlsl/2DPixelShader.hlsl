SamplerState Sampler : register(s0);
Texture2D<float4> Texture : register(t0);

float4 main(in float4 i_svpos : SV_POSITION,
			in float2 i_uv : TEXCOORD0) : SV_TARGET
{
    return Texture.Sample(Sampler, i_uv);
}