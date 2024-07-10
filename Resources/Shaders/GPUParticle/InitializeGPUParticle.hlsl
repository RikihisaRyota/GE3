#include "GPUParticle.hlsli"
AppendStructuredBuffer<uint> particleIndexCommands : register(u0);
AppendStructuredBuffer<uint> fieldIndexBuffer : register(u1);

[numthreads(threadBlockSize, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint index = DTid.x;
    if(index < fieldSize){
        fieldIndexBuffer.Append(index);
    }
    particleIndexCommands.Append(index);
}
