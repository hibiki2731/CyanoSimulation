#include <FadeHeader.hlsli>

PSInput main(uint vertexID : SV_VertexID)
{
    PSInput result;
    // 3つの頂点ID (0, 1, 2) から画面を覆う三角形を計算
    result.position.x = (vertexID == 2) ?  3.0f : -1.0f;
    result.position.y = (vertexID == 1) ? -3.0f :  1.0f;
    result.position.z = 0.0f; // 深度は一番手前
    result.position.w = 1.0f;
    return result;
}