#include "GPUParticle.hlsli"
struct Count {
    int32_t count;
};

RWStructuredBuffer<Emitter> origalEmitter : register(u0);
RWStructuredBuffer<Count> createEmitterCounter : register(u1);

RWStructuredBuffer<VertexEmitter> origalVertexEmitter : register(u2);
RWStructuredBuffer<Count> createVertexEmitterCounter : register(u3);

RWStructuredBuffer<MeshEmitter> origalMeshEmitter : register(u4);
RWStructuredBuffer<Count> createMeshEmitterCounter : register(u5);

RWStructuredBuffer<TransformModelEmitter> origalTransformModelEmitter : register(u6);
RWStructuredBuffer<Count> createTransformModelEmitterCounter : register(u7);

RWStructuredBuffer<TransformAreaEmitter> origalTransformAreaEmitter : register(u8);
RWStructuredBuffer<Count> createTransformAreaEmitterCounter : register(u9);

RWStructuredBuffer<Emitter> addEmitter : register(u10);
RWStructuredBuffer<VertexEmitter> addVertexEmitter : register(u11);
RWStructuredBuffer<MeshEmitter> addMeshEmitter : register(u12);
RWStructuredBuffer<TransformModelEmitter> addTransformModelEmitter : register(u13);
RWStructuredBuffer<TransformAreaEmitter> addTransformAreaEmitter : register(u14);

void ResetEmitter(inout int32_t emitterCount,inout uint32_t isAlive){
    emitterCount = -1;
    isAlive = false;
}

[numthreads(emitterSize, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    if (createEmitterCounter[0].count > 0) {
        // 現在のスレット
        uint32_t index = DTid.x;
        // 現在のスレッドが処理するエミッターを取得
        Emitter currentEmitter = origalEmitter[index];

        // もしエミッターがアクティブでない場合にのみ処理を行う
        if (!currentEmitter.isAlive) {
            int32_t val = -1;
            // エミッタカウンタから1減らして取得
            InterlockedAdd(createEmitterCounter[0].count, -1, val);

            // valが0以上の場合にのみ新しいエミッターを取得
            if (val >= 0) {
                Emitter newEmitter = addEmitter[val];

                // 新しいエミッターがアクティブな場合のみ更新
                if (newEmitter.isAlive) {
                    origalEmitter[index] = newEmitter;
                    // エミッターの状態をリセット
                    ResetEmitter(addEmitter[val].emitterCount, addEmitter[val].isAlive);
                }
            }
        }
    }

// VertexEmitter
    if (createVertexEmitterCounter[0].count > 0) {
        uint32_t index = DTid.x;
        VertexEmitter currentEmitter = origalVertexEmitter[index];

        if (!currentEmitter.isAlive) {
            int32_t val = -1;
            InterlockedAdd(createVertexEmitterCounter[0].count, -1, val);

            if (val >= 0) {
                VertexEmitter newEmitter = addVertexEmitter[val];

                if (newEmitter.isAlive) {
                    origalVertexEmitter[index] = newEmitter;
                    ResetEmitter(addVertexEmitter[val].emitterCount, addVertexEmitter[val].isAlive);
                }
            }
        }
    }

    // MeshEmitter
    if (createMeshEmitterCounter[0].count > 0) {
        uint32_t index = DTid.x;
        MeshEmitter currentEmitter = origalMeshEmitter[index];

        if (!currentEmitter.isAlive) {
            int32_t val = -1;
            InterlockedAdd(createMeshEmitterCounter[0].count, -1, val);

            if (val >= 0) {
                MeshEmitter newEmitter = addMeshEmitter[val];

                if (newEmitter.isAlive) {
                    origalMeshEmitter[index] = newEmitter;
                    ResetEmitter(addMeshEmitter[val].emitterCount, addMeshEmitter[val].isAlive);
                }
            }
        }
    }

    // TransformModelEmitter
    if (createTransformModelEmitterCounter[0].count > 0) {
        uint32_t index = DTid.x;
        TransformModelEmitter currentEmitter = origalTransformModelEmitter[index];

        if (!currentEmitter.isAlive) {
            int32_t val = -1;
            InterlockedAdd(createTransformModelEmitterCounter[0].count, -1, val);

            if (val >= 0) {
                TransformModelEmitter newEmitter = addTransformModelEmitter[val];

                if (newEmitter.isAlive) {
                    origalTransformModelEmitter[index] = newEmitter;
                    ResetEmitter(addTransformModelEmitter[val].emitterCount, addTransformModelEmitter[val].isAlive);
                }
            }
        }
    }

    // TransformAreaEmitter
    if (createTransformAreaEmitterCounter[0].count > 0) {
        uint32_t index = DTid.x;
        TransformAreaEmitter currentEmitter = origalTransformAreaEmitter[index];

        if (!currentEmitter.isAlive) {
            int32_t val = -1;
            InterlockedAdd(createTransformAreaEmitterCounter[0].count, -1, val);

            if (val >= 0) {
                TransformAreaEmitter newEmitter = addTransformAreaEmitter[val];

                if (newEmitter.isAlive) {
                    origalTransformAreaEmitter[index] = newEmitter;
                    ResetEmitter(addTransformAreaEmitter[val].emitterCount, addTransformAreaEmitter[val].isAlive);
                }
            }
        }
    }
}