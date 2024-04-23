#include "GPUParticle.hlsli"

StrcturedBuffer<Emitter> srvEmitter : register(t0);

AppendStructuredBuffer<Emitter> appendEmitter : register(u0);

[numthreads(1, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    uint index = DTid.x;
    appendEmitter.Append(srvEmitter[index]);
}