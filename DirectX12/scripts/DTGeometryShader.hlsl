#include <DTHeader.hlsli>

[maxvertexcount(4)]
void main(
	point GS_INPUT input[1], 
	inout TriangleStream< PS_INPUT > output
)
{
	//カメラの右と上を求める
    float3 camRight = float3(View[0][0], View[1][0], View[2][0]);
    float3 camUp = float3(View[0][1], View[1][1], View[2][1]);
	
	//UVの計算
    float uvWidth = 1.0f / 10.0f; //数字は10種類
    float uvLeft = uvWidth * input[0].digit;
	
    float2 uv[4] =
    {
        float2(uvLeft, 0.0f), //左上
		float2(uvLeft + uvWidth, 0.0f), //右上
		float2(uvLeft, 1.0f), //左下
		float2(uvLeft + uvWidth, 1.0f) //右下
    };
	
	//4頂点の生成
    float3 center = input[0].pos;
    float halfSize = input[0].size * 0.5f;
    float3 offsets[4] =
    {
        -camRight * halfSize + camUp * halfSize, //左上
		camRight * halfSize + camUp * halfSize, //右上
		-camRight * halfSize - camUp * halfSize, //左下
        camRight * halfSize - camUp * halfSize //右下
    };
	
	[unroll] //ループを展開
    for (int i = 0; i < 4; i++)
    {
        PS_INPUT outputVertex;
        float3 WorldPos = center + offsets[i];
        outputVertex.pos = float4(WorldPos, 1.0f);
        outputVertex.pos = mul(float4(WorldPos, 1.0f), mul(View, Proj)); //ワールド→ビュー→プロジェクション変換
        outputVertex.uv = uv[i];
        outputVertex.alpha = input[0].alpha;
        output.Append(outputVertex);
    }
}
