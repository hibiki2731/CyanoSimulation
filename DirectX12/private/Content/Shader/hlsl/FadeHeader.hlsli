struct PSInput
{
    float4 position : SV_Position;
};

cbuffer FadeParams : register(b0)
{
    float alpha;
};

