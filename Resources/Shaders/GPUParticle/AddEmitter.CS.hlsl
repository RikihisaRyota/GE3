#include "GPUParticle.hlsli"

ConsumeStructuredBuffer<Emitter> addEmitter : register(u0);

RWStructuredBuffer<Emitter> origalEmiiter : register(u1);

RWStructuredBuffer<int32_t> createEmitterCounter : register(u2);

[numthreads(emitterSize, 1, 1)]
void main( uint32_t3 DTid : SV_DispatchThreadID )
{
    uint32_t index = DTid.x;
    if (!origalEmiiter[index].isAlive)
    {
        int32_t val=-1;
        InterlockedAdd(createEmitterCounter[0], -1,val);
        if(val > 0){
        Emitter emitter = addEmitter.Consume();
        origalEmiiter[index] = emitter;
        }
        
    }
}