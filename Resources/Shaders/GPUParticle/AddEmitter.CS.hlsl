#include "GPUParticle.hlsli"

RWStructuredBuffer<Emitter> origalEmitter : register(u0);
RWStructuredBuffer<int32_t> createEmitterCounter : register(u1);

RWStructuredBuffer<VertexEmitter> origalVertexEmitter : register(u2);
RWStructuredBuffer<int32_t> createVertexEmitterCounter : register(u3);

RWStructuredBuffer<MeshEmitter> origalMeshEmitter : register(u4);
RWStructuredBuffer<int32_t> createMeshEmitterCounter : register(u5);

RWStructuredBuffer<TransformModelEmitter> origalTransformModelEmitter : register(u6);
RWStructuredBuffer<int32_t> createTransformModelEmitterCounter : register(u7);

RWStructuredBuffer<TransformAreaEmitter> origalTransformAreaEmitter : register(u8);
RWStructuredBuffer<int32_t> createTransformAreaEmitterCounter : register(u9);

StructuredBuffer<Emitter> addEmitter : register(t0);
StructuredBuffer<VertexEmitter> addVertexEmitter : register(t1);
StructuredBuffer<MeshEmitter> addMeshEmitter : register(t2);
StructuredBuffer<TransformModelEmitter> addTransformModelEmitter : register(t3);
StructuredBuffer<TransformAreaEmitter> addTransformAreaEmitter : register(t4);

[numthreads(emitterSize, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    int32_t val = -1;
    InterlockedAdd(createEmitterCounter[0], -1, val);
    if (val >= 0) {
        uint32_t index = DTid.x;

        // 現在のスレッドが処理するエミッターを取得
        Emitter currentEmitter = origalEmitter[index];

        // addEmitterバッファから対応するインデックスのエミッターを取得
        Emitter newEmitter = addEmitter[val];

        if (!currentEmitter.isAlive && newEmitter.isAlive) {
            origalEmitter[index] = newEmitter;
        }
    }

    // VertexEmitter
    val = -1;
    InterlockedAdd(createVertexEmitterCounter[0], -1, val);
    if (val >= 0) {
        uint32_t index = DTid.x;
        VertexEmitter currentEmitter = origalVertexEmitter[index];
        VertexEmitter newEmitter = addVertexEmitter[val];

        if (!currentEmitter.isAlive && newEmitter.isAlive) {
            origalVertexEmitter[index] = newEmitter;
        }
    }

    // MeshEmitter
    val = -1;
    InterlockedAdd(createMeshEmitterCounter[0], -1, val);
    if (val >= 0) {
        uint32_t index = DTid.x;
        MeshEmitter currentEmitter = origalMeshEmitter[index];
        MeshEmitter newEmitter = addMeshEmitter[val];

        if (!currentEmitter.isAlive && newEmitter.isAlive) {
            origalMeshEmitter[index] = newEmitter;
        }
    }

    // TransformModelEmitter
    val = -1;
    InterlockedAdd(createTransformModelEmitterCounter[0], -1, val);
    if (val >= 0) {
        uint32_t index = DTid.x;
        TransformModelEmitter currentEmitter = origalTransformModelEmitter[index];
        TransformModelEmitter newEmitter = addTransformModelEmitter[val];

        if (!currentEmitter.isAlive && newEmitter.isAlive) {
            origalTransformModelEmitter[index] = newEmitter;
        }
    }

    // TransformAreaEmitter
    val = -1;
    InterlockedAdd(createTransformAreaEmitterCounter[0], -1, val);
    if (val >= 0) {
        uint32_t index = DTid.x;
        TransformAreaEmitter currentEmitter = origalTransformAreaEmitter[index];
        TransformAreaEmitter newEmitter = addTransformAreaEmitter[val];

        if (!currentEmitter.isAlive && newEmitter.isAlive) {
            origalTransformAreaEmitter[index] = newEmitter;
        }
    }
}
