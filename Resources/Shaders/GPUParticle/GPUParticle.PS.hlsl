#include "GPUParticle.hlsli"

#define myTex2DSpace space1

StructuredBuffer<Particle> gParticle : register(t0);

Texture2D<float4> gTexture[] : register(t0, myTex2DSpace);
SamplerState gSampler : register(s0);

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

float3 HSVToRGB(in float3 hsv)
{
    float4 k = float4(1.0f, 2.0f / 3.0f, 1.0f / 3.0f, 3.0f);
    float3 p = abs(frac(hsv.xxx + k.xyz) * 6.0f - k.www);
    return hsv.z * lerp(k.xxx, clamp(p - k.xxx, 0.0f, 1.0f), hsv.y);
}

PixelShaderOutput main(VertexShaderOutput input)
{
    
    PixelShaderOutput output;
    float4 textureColor = input.color * gTexture[gParticle[input.instanceId].textureInidex].Sample(gSampler, input.texcoord);
    output.color = textureColor;
    if (output.color.a <= 0.5f)
    {
        discard;
    }
    return output;
}