#include "GPUParticle.hlsli"
#include "GPUParticleShaderStructs.h"

#define myTex2DSpace space1

Texture2D<float4> gTexture[] : register(t0, myTex2DSpace);
SamplerState gSampler : register(s0);

struct PixelShaderOutput
{
    float32_t4 color : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput input)
{
    
    PixelShaderOutput output;
    output.color = input.color;
    return output;
}