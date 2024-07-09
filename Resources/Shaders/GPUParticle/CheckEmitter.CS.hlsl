#include "GPUParticle.hlsli"

RWStructuredBuffer<Emitter> addEmitter : register(u0);
RWStructuredBuffer<Emitter> origalEmitter : register(u1);

[numthreads(emitterSize, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID, uint3 GTid : SV_GroupThreadID, uint3 GID : SV_GroupID)
{
    // グループスレッドIDを使用して、オリジナルエミッターのインデックスを取得
    uint32_t origalIndex = GTid.x;

    // グループIDを使用して、追加エミッターのインデックスを取得
    uint32_t addIndex = GID.x;

    if (addEmitter[addIndex].emitterCount == origalEmitter[origalIndex].emitterCount && 
        addEmitter[addIndex].frequency.isLoop) {
        EmitterTime time = origalEmitter[origalIndex].time;  
        origalEmitter[origalIndex] = addEmitter[addIndex];
        origalEmitter[origalIndex].time = time;
        addEmitter[addIndex].isAlive = false;
    }
}
