#include "DTHeader.hlsli"

Texture2D<float4> Texture : register(t0);
SamplerState      Sampler : register(s0);

float4 main(PS_INPUT input) : SV_TARGET
{
    float4 tex = Texture.Sample(Sampler, input.uv);

    float t = input.alpha; //1.0(新生) → 0.0(消滅)

    //炎の色グラデーション
    //新生時：明るい黄白色
    //中間  ：オレンジ
    //消滅時：暗い赤→透明
    float3 colorYoung = float3(1.0f, 0.9f, 0.5f); //黄白
    float3 colorMid   = float3(1.0f, 0.4f, 0.0f); //オレンジ
    float3 colorOld   = float3(0.5f, 0.0f, 0.0f); //暗い赤

    float3 color;
    if (t > 0.5f) {
        // 新生 → 中間
        color = lerp(colorMid, colorYoung, (t - 0.5f) * 2.0f);
    } else {
        // 中間 → 消滅
        color = lerp(colorOld, colorMid, t * 2.0f);
    }
    

    // テクスチャのアルファをそのまま使う場合
    float alpha = tex.a;

    // さらにシェーダーで縁をぼかしたい場合（追加で乗算）
    // uv中心からの距離でもう一段フェード
    float2 centeredUV = input.uv - 0.5f;
    float dist = length(centeredUV) * 2.0f; // 0(中心)~1(端)
    float edgeFade = 1.0f - smoothstep(0.2f, 1.0f, dist);
    alpha *= edgeFade;

    // 寿命による透明化
    alpha *= smoothstep(0.0f, 0.15f, t);
    
    return float4(color, alpha);
}