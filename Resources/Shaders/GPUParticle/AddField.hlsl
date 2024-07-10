#include "GPUParticle.hlsli"

StructuredBuffer<FieldForGPU> addField : register(t0);
RWStructuredBuffer<FieldForGPU> origalField : register(u0);
RWStructuredBuffer<int32_t> createFieldCounter : register(u1);
ConsumeStructuredBuffer<uint> fieldIndexStockBuffer : register(u2);


[numthreads(fieldSize, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    // Countが一致する場合の処理
    int32_t createFieldNum = -1;
    InterlockedAdd(createFieldCounter[0], -1, createFieldNum);
    if(createFieldNum >= 0){
        // addFieldバッファから対応するインデックスのエミッターを取得
        FieldForGPU newField = addField[createFieldNum];
        if (newField.isAlive) {
            uint32_t index = fieldIndexStockBuffer.Consume();
            origalField[index] = newField;
        }
    }
}