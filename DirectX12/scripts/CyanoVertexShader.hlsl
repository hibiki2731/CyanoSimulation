#include "CyanoHeader.hlsli"
static const float MAX_Z_DEPTH = 100.1;

PS_INPUT main(VS_INPUT input)
{
    PS_INPUT output;
    output.position.xy = input.position * CyanoPointSize + g_points[input.instatnceID].position.xy;
    output.position.z = 50.0f;
    output.position.w = 1.0f;
    //画面座標変換
    output.position.x = (output.position.x / WindowSize.x) * 2.0f - 1.0f;
    output.position.y = 1.0f - (output.position.y / WindowSize.y) * 2.0f;
    float z = output.position.z / MAX_Z_DEPTH;
    output.position.z = clamp(z, 0.0f, 1.0f);
    output.uv = input.uv;
    
    return output;
}