#include "GPUParticle.hlsli"

StructuredBuffer<FieldForGPU> addField : register(t0);
RWStructuredBuffer<FieldForGPU> origalField : register(u0);
RWStructuredBuffer<int32_t> createEmitterCounter : register(u1);


[numthreads(1, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
}