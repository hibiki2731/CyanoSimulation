cbuffer b0 : register(b0)
{
    matrix World;
    float2 WindowSize;
    float2 SpriteSize;
    float2 TextureSize;
    float BorderSize;
}

static const float MAX_Z_DEPTH = 100.1;

void calcNineSlicePosition(float pos, float spriteSize, float textureSize, float borderSize, out float outputPos, out float outputUV)
{
    if (pos == 0.0)
    {
        outputPos = 0;
        outputUV = 0;
    }
    else if (pos == 1.0)
    {
        outputPos = spriteSize;
        outputUV = 1.0;
    }
    else if (pos <= 0.5)
    {
        outputPos = borderSize;
        outputUV = borderSize / textureSize;
    }
    else if (pos > 0.5)
    {
        outputPos = spriteSize - borderSize;
        outputUV = 1.0 - (borderSize / textureSize);
    }
    
}

void main( in float4 i_pos : POSITION,
           in float2 i_uv : TEXCOORD0,
           out float4 o_svpos: SV_POSITION,
           out float2 o_uv : TEXCOORD
) 
{
    float4 outputPos = float4(0.0, 0.0, 0.0, 0.0);
    float2 outputUV = float2(1.0, 1.0);
    //x: 0.0 ～ 1.0、y: 0.0 ～ 1.0の頂点を入力(y座標は下が正)
    //頂点が9スライスのどの部分にあるかを判定
    calcNineSlicePosition(i_pos.x, SpriteSize.x, TextureSize.x, BorderSize, outputPos.x, outputUV.x);
    calcNineSlicePosition(i_pos.y, SpriteSize.y, TextureSize.y, BorderSize, outputPos.y, outputUV.y);
    outputPos.w = 1.0f;
    
    //ワールド変換
    outputPos = mul(World, outputPos);
    //画面座標変換
    outputPos.x = (outputPos.x / WindowSize.x) * 2.0f - 1.0f;
    outputPos.y = 1.0f - (outputPos.y / WindowSize.y) * 2.0f;
    float z = outputPos.z / MAX_Z_DEPTH;
    outputPos.z = clamp(z, 0.0, 1.0);
    o_svpos = outputPos;
    o_uv = outputUV;
    
    //i_pos.y = -i_pos.y; //y座標反転
    //o_svpos = i_pos;
    //o_uv = i_uv;
    
}
