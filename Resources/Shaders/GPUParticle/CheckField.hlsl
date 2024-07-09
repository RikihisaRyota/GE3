#include "GPUParticle.hlsli"

RWStructuredBuffer<FieldForGPU> addField : register(u0);
RWStructuredBuffer<FieldForGPU> origalField : register(u1);

[numthreads(1, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
}