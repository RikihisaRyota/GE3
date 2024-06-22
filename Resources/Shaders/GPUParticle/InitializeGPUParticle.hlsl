#include "GPUParticle.hlsli"
AppendStructuredBuffer<uint> particleIndexCommands : register(u0);

[numthreads(threadBlockSize, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint index = DTid.x;
    
    particleIndexCommands.Append(index);
}
