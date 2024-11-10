#include "GPUParticle.hlsli"
#include "GPUParticleShaderStructs.h"

#define myTex2DSpace space1

StructuredBuffer<GPUParticleShaderStructs::TrailsData> trailsData : register(t0);
StructuredBuffer<GPUParticleShaderStructs::TrailsPosition> trailsPosition : register(t1);
StructuredBuffer<GPUParticleShaderStructs::TrailsIndex> trailsCounter : register(t2);
StructuredBuffer<GPUParticleShaderStructs::TrailsVertexData> trailsVertexData : register(t3);
StructuredBuffer<uint> drawInstanceCount : register(t4);

Texture2D<float4> gTexture[] : register(t0, myTex2DSpace);
SamplerState gSampler : register(s0);

struct PixelShaderOutput
{
    float32_t4 color : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput input)
{
    
    PixelShaderOutput output; 
    output.color = input.color * gTexture[trailsData[trailsCounter[input.instanceId].trailsIndex].textureIndex].Sample(gSampler, input.texcoord);
    return output;
}