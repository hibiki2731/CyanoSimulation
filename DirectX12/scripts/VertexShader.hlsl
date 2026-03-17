#include<Header.hlsli>

void main(
    in float4 i_pos : POSITION,
    in float4 i_normal : NORMAL,
    in float2 i_uv : TEXCOORD0,
    out float4 o_svpos : SV_POSITION,
    out float2 o_uv : TEXCOORD0,
    out float4 o_wpos : TEXCOORD1,
    out float4 o_wnormal : TEXCOORD2
) 
{
    o_wpos = mul(World, i_pos);
    o_svpos = mul(ViewProj, o_wpos);
    
    i_normal.w = 0;
    o_wnormal = normalize(mul(World, i_normal));
    
    o_uv = i_uv;
}
