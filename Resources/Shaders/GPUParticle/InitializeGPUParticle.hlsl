#include "GPUParticleShaderStructs.h"
#include "GPUParticle.hlsli"
RWStructuredBuffer<uint32_t> directParticleFreeList : register(u0);
RWStructuredBuffer<int32_t> directParticleFreeListIndex : register(u1);
RWStructuredBuffer<uint32_t> computeParticleFreeList : register(u2);
RWStructuredBuffer<int32_t> computeParticleFreeListIndex : register(u3);
RWStructuredBuffer<uint32_t> fieldParticleFreeList : register(u4);
RWStructuredBuffer<int32_t> fieldParticleFreeListIndex : register(u5);
RWStructuredBuffer<GPUParticleShaderStructs::DrawIndex> drawIndex : register(u6);

AppendStructuredBuffer<int> trailsIndexBuffer : register(u7);

[numthreads(GPUParticleShaderStructs::ComputeThreadBlockSize, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint index = DTid.x;
    if(index==0){
        directParticleFreeListIndex[0]=GPUParticleShaderStructs::DivisionParticleNum-1;
        computeParticleFreeListIndex[0]=GPUParticleShaderStructs::DivisionParticleNum-1;
    }
    if(index < GPUParticleShaderStructs::DivisionParticleNum){
        directParticleFreeList[index]=index;
        computeParticleFreeList[index]=index;
    }
    if(index < GPUParticleShaderStructs::MaxParticleNum){
        drawIndex[index].directIndex=-1;
        drawIndex[index].computeIndex=-1;
    }
    if(index < GPUParticleShaderStructs::MaxTrailsNum){
        trailsIndexBuffer.Append(int(index));
    }
}
