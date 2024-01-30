#include "Sprite.hlsli"

ConstantBuffer<WorldTransform> gWorldTransform : register(b0);

struct VSInput
{
    float3 position : POSITION0;
    float2 texcoord : TEXCOORD0;
};

VSOutput main(VSInput input)
{
    VSOutput output;
    output.svpos = mul(float4(input.position, 1.0f), gWorldTransform.mat);
    output.uv = input.texcoord;
    return output;
}