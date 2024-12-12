#include "GPUParticle.hlsli"
#include "GPUParticleShaderStructs.h"

RWStructuredBuffer<GPUParticleShaderStructs::FieldForGPU> origalField : register(u0);
RWStructuredBuffer<int32_t> fieldFreeList : register(u1);
RWStructuredBuffer<uint32_t> fieldFreeListIndex : register(u2);
AppendStructuredBuffer<uint> fieldIndexBuffer : register(u3);

[numthreads(GPUParticleShaderStructs::MaxFieldNum, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    uint32_t index = DTid.x;
    if(origalField[index].isAlive && !origalField[index].frequency.isLoop){
        origalField[index].frequency.lifeCount--;
        if(origalField[index].frequency.lifeCount <= 0){
            origalField[index].isAlive = false;
            origalField[index].fieldCount=-1;
            // freeListにindexを返却
            int32_t freeListIndex = -1; 
            InterlockedAdd(fieldFreeListIndex[0], 1,freeListIndex);
            fieldFreeList[freeListIndex + 1] = index;
        }  
    }
    if(origalField[index].isAlive){
        fieldIndexBuffer.Append(index);
    }
}