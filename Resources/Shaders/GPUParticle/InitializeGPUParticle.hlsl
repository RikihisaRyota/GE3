//#include "GPUParticleShaderStructs.h"
#include "GPUParticleShaderStructs.h"

#include "GPUParticle.hlsli"
AppendStructuredBuffer<uint> particleIndexCommands : register(u0);
AppendStructuredBuffer<uint> fieldIndexBuffer : register(u1);
AppendStructuredBuffer<uint> trailsIndexBuffer : register(u2);

[numthreads(GPUParticleShaderStructs::ComputeThreadBlockSize, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint index = DTid.x;
    if(index < GPUParticleShaderStructs::MaxFieldNum){
        fieldIndexBuffer.Append(index);
    }
    if(index < GPUParticleShaderStructs::MaxTrailsNum){
        trailsIndexBuffer.Append(index);
    }
    particleIndexCommands.Append(index);
}
