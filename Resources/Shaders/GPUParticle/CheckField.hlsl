#include "GPUParticle.hlsli"
#include "GPUParticleShaderStructs.h"

RWStructuredBuffer<FieldForGPU> addField : register(u0);
RWStructuredBuffer<FieldForGPU> origalField : register(u1);

[numthreads(GPUParticleShaderStructs::MaxFieldNum, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID, uint3 GTid : SV_GroupThreadID, uint3 GID : SV_GroupID)
{
    // グループスレッドIDを使用して、オリジナルエミッターのインデックスを取得
    uint32_t origalIndex = GTid.x;

    // グループIDを使用して、追加エミッターのインデックスを取得
    uint32_t addIndex = GID.x;
    if (addField[addIndex].fieldCount == origalField[origalIndex].fieldCount) {
        origalField[origalIndex] = addField[addIndex];
        addField[addIndex].isAlive = false;
    }
}