#include "GPUParticle.hlsli"

StructuredBuffer<Emitter> addEmitter : register(t0);
RWStructuredBuffer<Emitter> origalEmiiter : register(u0);
RWStructuredBuffer<int32_t> createEmitterCounter : register(u1);

[numthreads(emitterSize, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    // emitterCountが一致する場合の処理
    int32_t val = -1;
    InterlockedAdd(createEmitterCounter[0], -1, val);
    if(val >= 0){
        uint32_t index = DTid.x;

        // 現在のスレッドが処理するエミッターを取得
        Emitter currentEmitter = origalEmiiter[index];

        // addEmitterバッファから対応するインデックスのエミッターを取得
        Emitter newEmitter = addEmitter[val];

        if (!currentEmitter.isAlive && newEmitter.isAlive) {
            origalEmiiter[index] = newEmitter;
        }
    }
}
