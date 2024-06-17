#include "Sprite.hlsli"

Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

ConstantBuffer<Material> gMaterial : register(b1);

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

PixelShaderOutput main(VSOutput input)
{
    PixelShaderOutput output;
    output.color = gMaterial.color * gTexture.Sample(gSampler, input.uv);
    if(output.color.a<=0.5f){
        discard;
    }
    return output;

}