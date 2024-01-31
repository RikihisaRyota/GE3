#include "GPUParticle.hlsli"

Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput input)
{
    
    float2 center = float2(0.5f, 0.5f);
    float radius = 0.2f;

    float distanceToCenter = length(input.texcoord - center);
    
    PixelShaderOutput output;
    float4 textureColor = float4(input.position.xy,0.5f, 0.3f);// * gTexture.Sample(gSampler, input.texcoord);
    output.color = textureColor;
    output.color.a = min(step(distanceToCenter, radius),0.2f);
    
    return output;
}