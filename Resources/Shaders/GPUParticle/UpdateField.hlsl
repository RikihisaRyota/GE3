#include "GPUParticle.hlsli"

RWStructuredBuffer<FieldForGPU> origalField : register(u0);

[numthreads(fieldSize, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    uint32_t index = DTid.x;
    if(origalField[index].isAlive&&!origalField[index].frequency.isLoop){
        origalField[index].frequency.lifeTime--;
        if(origalField[index].frequency.lifeTime <= 0){
            origalField[index].isAlive = false;
        }
    }
}