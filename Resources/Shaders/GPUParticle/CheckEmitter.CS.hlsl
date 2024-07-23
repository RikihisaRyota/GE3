#include "GPUParticle.hlsli"

// エミッター用のバッファ
RWStructuredBuffer<Emitter> addEmitter : register(u0);
RWStructuredBuffer<Emitter> origalEmitter : register(u1);

RWStructuredBuffer<VertexEmitter> addVertexEmitter : register(u2);
RWStructuredBuffer<VertexEmitter> origalVertexEmitter : register(u3);

RWStructuredBuffer<MeshEmitter> addMeshEmitter : register(u4);
RWStructuredBuffer<MeshEmitter> origalMeshEmitter : register(u5);

RWStructuredBuffer<TransformModelEmitter> addTransformModelEmitter : register(u6);
RWStructuredBuffer<TransformModelEmitter> origalTransformModelEmitter : register(u7);

RWStructuredBuffer<TransformAreaEmitter> addTransformAreaEmitter : register(u8);
RWStructuredBuffer<TransformAreaEmitter> origalTransformAreaEmitter : register(u9);

// エミッターの数を表す定数バッファ
struct Index{
    int32_t index;
};
ConstantBuffer<Index> addEmitterCount : register(b0);
ConstantBuffer<Index> addVertexEmitterCount : register(b1);
ConstantBuffer<Index> addMeshEmitterCount : register(b2);
ConstantBuffer<Index> addTransformModelEmitterCount : register(b3);
ConstantBuffer<Index> addTransformAreaEmitterCount : register(b4);

[numthreads(emitterSize, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID, uint3 GTid : SV_GroupThreadID, uint3 GID : SV_GroupID)
{
    // グループスレッドIDを使用して、オリジナルエミッターのインデックスを取得
    uint32_t origalIndex = GTid.x;

    // グループIDを使用して、追加エミッターのインデックスを取得
    uint32_t addIndex = GID.x;
    if (addIndex < addEmitterCount.index) {
        if (addEmitter[addIndex].emitterCount == origalEmitter[origalIndex].emitterCount &&
            addEmitter[addIndex].frequency.isLoop) {
            EmitterTime time = origalEmitter[origalIndex].time;
            if(origalEmitter[origalIndex].frequency.isOnce &&
                origalEmitter[origalIndex].isAlive){
                time.particleTime = -1;
            } 
            if(origalEmitter[origalIndex].frequency.isOnce &&
                !origalEmitter[origalIndex].isAlive){
                time.particleTime = 0;
            }
            origalEmitter[origalIndex] = addEmitter[addIndex];
            origalEmitter[origalIndex].time = time;
            addEmitter[addIndex].isAlive = false;
        }
    } else if (addIndex < addEmitterCount.index + addVertexEmitterCount.index) {
        addIndex -= addEmitterCount.index;
        if (addVertexEmitter[addIndex].emitterCount == origalVertexEmitter[origalIndex].emitterCount &&
            addVertexEmitter[addIndex].frequency.isLoop) {
            EmitterTime time = origalVertexEmitter[origalIndex].time;  
            if(origalVertexEmitter[origalIndex].frequency.isOnce &&
                origalVertexEmitter[origalIndex].isAlive){
                time.particleTime = -1;
            }
            if(origalVertexEmitter[origalIndex].frequency.isOnce &&
                !origalVertexEmitter[origalIndex].isAlive){
                time.particleTime = 0;
            }
            origalVertexEmitter[origalIndex] = addVertexEmitter[addIndex];
            origalVertexEmitter[origalIndex].time = time;
            addVertexEmitter[addIndex].isAlive = false;
        }
    } else if (addIndex < addEmitterCount.index + addVertexEmitterCount.index + addMeshEmitterCount.index) {
        addIndex -= (addEmitterCount.index + addVertexEmitterCount.index);
        if (addMeshEmitter[addIndex].emitterCount == origalMeshEmitter[origalIndex].emitterCount &&
            addMeshEmitter[addIndex].frequency.isLoop) {
            EmitterTime time = origalMeshEmitter[origalIndex].time;  
            if(origalMeshEmitter[origalIndex].frequency.isOnce &&
                origalMeshEmitter[origalIndex].isAlive){
                time.particleTime = -1;
            }
            origalMeshEmitter[origalIndex] = addMeshEmitter[addIndex];
            origalMeshEmitter[origalIndex].time = time;
            addMeshEmitter[addIndex].isAlive = false;
        }
    } else if (addIndex < addEmitterCount.index + addVertexEmitterCount.index + addMeshEmitterCount.index + addTransformModelEmitterCount.index) {
        addIndex -= (addEmitterCount.index + addVertexEmitterCount.index + addMeshEmitterCount.index);
        if (addTransformModelEmitter[addIndex].emitterCount == origalTransformModelEmitter[origalIndex].emitterCount &&
            addTransformModelEmitter[addIndex].frequency.isLoop) {
            EmitterTime time = origalTransformModelEmitter[origalIndex].time;  
            if(origalTransformModelEmitter[origalIndex].frequency.isOnce &&
                origalTransformModelEmitter[origalIndex].isAlive){
                time.particleTime = -1;
            }
            origalTransformModelEmitter[origalIndex] = addTransformModelEmitter[addIndex];
            origalTransformModelEmitter[origalIndex].time = time;
            addTransformModelEmitter[addIndex].isAlive = false;
        }
    } else if (addIndex < addEmitterCount.index + addVertexEmitterCount.index + addMeshEmitterCount.index + addTransformModelEmitterCount.index+ addTransformAreaEmitterCount.index) {
        addIndex -= (addEmitterCount.index + addVertexEmitterCount.index + addMeshEmitterCount.index + addTransformModelEmitterCount.index);
        if (addTransformAreaEmitter[addIndex].emitterCount == origalTransformAreaEmitter[origalIndex].emitterCount &&
            addTransformAreaEmitter[addIndex].frequency.isLoop) {
            EmitterTime time = origalTransformAreaEmitter[origalIndex].time;  
            if(origalTransformAreaEmitter[origalIndex].frequency.isOnce &&
                origalTransformAreaEmitter[origalIndex].isAlive){
                time.particleTime = -1;
            }
            origalTransformAreaEmitter[origalIndex] = addTransformAreaEmitter[addIndex];
            origalTransformAreaEmitter[origalIndex].time = time;
            addTransformAreaEmitter[addIndex].isAlive = false;
        }
    }
}
