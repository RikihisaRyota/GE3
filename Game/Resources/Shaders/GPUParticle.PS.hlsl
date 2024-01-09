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
    float4 textureColor = gTexture.Sample(gSampler, input.texcoord);

    ////if (textureColor.a <= 0.5f || textureColor.a == 0.0f)
    ////{
    ////    discard;
    ////}
    
    //float2 xy = input.texcoord * 2.0f - 1.0f;
    //if (length(xy) > 1.0f)
    //{
    //    discard;
    //}
    
    output.color = textureColor;
    //output.color = float4(0.0f, 0.0f, 0.0f, 1.0f);
    return output;
}