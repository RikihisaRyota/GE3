#include "GPUParticle.hlsli"

ConsumeStructuredBuffer<Emitter> addEmitter : register(u0);

RWStructuredBuffer<Emitter> origalEmiiter : register(u1);

RWStructuredBuffer<uint> createEmitterCounter : register(u2);

[numthreads(emitterSize, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    uint index = DTid.x;
    if (!origalEmiiter[index].isAlive)
    {
        InterlockedAdd(createEmitterCounter[0], -1);
        uint32_t emitterCount =createEmitterCounter[0];
        if(emitterCount > 0){
        Emitter emitter = addEmitter.Consume();
        origalEmiiter[index] = emitter;
        }
        
    }
}