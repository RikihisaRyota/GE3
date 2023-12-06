#include "Game/Resources/Shaders/GPUParticle.hlsli"

struct Particle
{
    float3 position;
    float3 velocity;
};
StructuredBuffer<Particle> gParticle : register(t0);

struct VertexShaderInput
{
    float4 position : POSITION0;
};

VertexShaderOutput main(VertexShaderInput input,uint instanceID:SV_InstanceID)
{
    VertexShaderInput output;
    output.position = float4(gParticle[instanceID].position,0.0f);
    return output;
}