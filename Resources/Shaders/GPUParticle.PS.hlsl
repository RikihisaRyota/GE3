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
    float4 textureColor = float4(1.0f, 1.0f, 1.0f, 0.3f);// * gTexture.Sample(gSampler, input.texcoord);
    //if (textureColor.a <= 0.5f)
    //{
    //    discard;
    //}
    output.color = textureColor;
    return output;
}