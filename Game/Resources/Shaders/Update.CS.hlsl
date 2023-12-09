#include "GPUParticle.hlsli"

RWStructuredBuffer<Particle> Output : register(u0);

struct ParticleInfo
{
    float speed;
};
ConstantBuffer<ParticleInfo> Info : register(b0);

[numthreads(1, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    Output[DTid.x].velocity = -Output[DTid.x].tarnslate;
    Output[DTid.x].tarnslate.z += Info.speed;
    
    Output[DTid.x].matWorld = MakeAffine(Output[DTid.x].scale, Output[DTid.x].rotate, Output[DTid.x].tarnslate);
}