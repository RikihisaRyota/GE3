//#include "GPUParticleShaderStructs.h"
#include "../../../Engine/Engine/GPUParticleManager/GPUParticle/GPUParticleShaderStructs.h"
#include "GPUParticle.hlsli"
AppendStructuredBuffer<uint> particleIndexCommands : register(u0);
AppendStructuredBuffer<uint> fieldIndexBuffer : register(u1);

[numthreads(ComputeThreadBlockSize, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint index = DTid.x;
    if(index < fieldSize){
        fieldIndexBuffer.Append(index);
    }
    particleIndexCommands.Append(index);
}
