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
    Output[DTid.x].velocity = normalize(-Output[DTid.x].translate);
    //Output[DTid.x].velocity * Info.speed
}