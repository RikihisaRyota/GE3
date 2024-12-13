#include "GPUParticle.hlsli"
#include "GPUParticleShaderStructs.h"

StructuredBuffer<GPUParticleShaderStructs::FieldForGPU> addField : register(t0);
RWStructuredBuffer<GPUParticleShaderStructs::FieldForGPU> origalField : register(u0);
RWStructuredBuffer<int32_t> createFieldCounter : register(u1);
RWStructuredBuffer<int32_t> fieldFreeList : register(u2);
RWStructuredBuffer<uint32_t> fieldFreeListIndex : register(u3);

[numthreads(GPUParticleShaderStructs::MaxFieldNum, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    // Countが一致する場合の処理
    int32_t createFieldNum = -1;
    InterlockedAdd(createFieldCounter[0], -1, createFieldNum);
    if(createFieldNum >= 0){
        // addFieldバッファから対応するインデックスのエミッターを取得
        GPUParticleShaderStructs::FieldForGPU newField = addField[createFieldNum];
        if (newField.isAlive) {
            int32_t index = -1;
            InterlockedAdd(fieldFreeListIndex[0], -1, index);
            // フリーリストが空っぽじゃないか
            if(index >= 0){
                origalField[fieldFreeList[index]] = newField;
            } 
            // 空だったら戻しておく
            else {
                InterlockedAdd(fieldFreeListIndex[0], 1);
            }
        }
    }
}