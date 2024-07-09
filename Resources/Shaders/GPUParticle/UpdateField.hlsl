#include "GPUParticle.hlsli"

RWStructuredBuffer<FieldForGPU > origalField : register(u0);
RWStructuredBuffer<Particle> particle : register(u1);

[numthreads(1, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
}