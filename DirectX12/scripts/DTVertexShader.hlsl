#include <DTHeader.hlsli>

//そのまま値を送る
GS_INPUT main(VS_INPUT input )
{
    GS_INPUT output;
    output.pos.xyz = input.pos;
    output.pos.w = 1.0f; //w成分は1.0fにする必要がある
    output.size = input.size;
    output.digit = input.digit;
    output.alpha = input.alpha;
    
    return output;
}
