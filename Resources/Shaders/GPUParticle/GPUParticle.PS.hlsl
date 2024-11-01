#include "GPUParticle.hlsli"
#include "GPUParticleShaderStructs.h"

#define myTex2DSpace space1

StructuredBuffer<GPUParticleShaderStructs::Particle> gParticle : register(t0);

Texture2D<float4> gTexture[] : register(t0, myTex2DSpace);
SamplerState gSampler : register(s0);

struct PixelShaderOutput
{
    float32_t4 color : SV_TARGET0;
};

float3 HSVToRGB(in float3 hsv)
{
    float32_t4 k = float32_t4(1.0f, 2.0f / 3.0f, 1.0f / 3.0f, 3.0f);
    float32_t3 p = abs(frac(hsv.xxx + k.xyz) * 6.0f - k.www);
    return hsv.z * lerp(k.xxx, clamp(p - k.xxx, 0.0f, 1.0f), hsv.y);
}

PixelShaderOutput main(VertexShaderOutput input)
{
    
    PixelShaderOutput output;
    float32_t4 textureColor = input.color * gTexture[gParticle[input.instanceId].textureIndex].Sample(gSampler, input.texcoord);
    output.color = textureColor;
    // 輪郭
    //output.color.a = abs(ddx(textureColor.a)) + abs(ddy(textureColor.a));
    if (output.color.a <= 0.0f)
    {
        discard;
    }
    return output;
}