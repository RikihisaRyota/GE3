#include "GPUParticle.hlsli"

StructuredBuffer<FieldForGPU> addField : register(t0);
RWStructuredBuffer<FieldForGPU> origalField : register(u0);
RWStructuredBuffer<int32_t> createFieldCounter : register(u1);


[numthreads(fieldSize, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    // Countが一致する場合の処理
    int32_t val = -1;
    InterlockedAdd(createFieldCounter[0], -1, val);
    if(val >= 0){
        uint32_t index = DTid.x;

        // 現在のスレッドが処理するエミッターを取得
        FieldForGPU currentField = origalField[index];

        // addFieldバッファから対応するインデックスのエミッターを取得
        FieldForGPU newField = addField[val];

        if (!currentField.isAlive && newField.isAlive) {
            origalField[index] = newField;
        }
    }
}