#include "GPUParticle.hlsli"

Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    output.color = float4(1.0f,1.0f,1.0f,1.0f) * gTexture.Sample(gSampler, input.texcoord);
    return output;
}