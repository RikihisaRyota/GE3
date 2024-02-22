#include "GPUParticle.hlsli"

ConsumeStructuredBuffer<Emitter> addEmitter : register(u0);

RWStructuredBuffer<Emitter> origalEmiiter : register(u1);

[numthreads(emitterSize, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    uint index = DTid.x;
    if (!origalEmiiter[index].isAlive)
    {
        origalEmiiter[index] = addEmitter.Consume();
    }
}