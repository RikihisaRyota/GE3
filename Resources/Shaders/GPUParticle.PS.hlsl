#include "GPUParticle.hlsli"

Texture2D<float4> gTexture : register(t2);
SamplerState gSampler : register(s0);

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    float4 textureColor = float4(1.0f, 1.0f, 1.0f, 1.0f) * gTexture.Sample(gSampler, input.texcoord);
    output.color = textureColor;
    return output;
}