#include "GPUParticle.hlsli"
#include "GPUParticleShaderStructs.h"

// エミッター用のバッファ
RWStructuredBuffer<EmitterForGPU> addEmitter : register(u0);
RWStructuredBuffer<EmitterForGPU> origalEmitter : register(u1);

RWStructuredBuffer<VertexEmitterForGPU> addVertexEmitter : register(u2);
RWStructuredBuffer<VertexEmitterForGPU> origalVertexEmitter : register(u3);

RWStructuredBuffer<MeshEmitterForGPU> addMeshEmitter : register(u4);
RWStructuredBuffer<MeshEmitterForGPU> origalMeshEmitter : register(u5);

RWStructuredBuffer<TransformModelEmitterForGPU> addTransformModelEmitter : register(u6);
RWStructuredBuffer<TransformModelEmitterForGPU> origalTransformModelEmitter : register(u7);

RWStructuredBuffer<TransformAreaEmitterForGPU> addTransformAreaEmitter : register(u8);
RWStructuredBuffer<TransformAreaEmitterForGPU> origalTransformAreaEmitter : register(u9);

// エミッターの数を表す定数バッファ
struct Index{
    int32_t index;
};
ConstantBuffer<Index> addEmitterCount : register(b0);
ConstantBuffer<Index> addVertexEmitterCount : register(b1);
ConstantBuffer<Index> addMeshEmitterCount : register(b2);
ConstantBuffer<Index> addTransformModelEmitterCount : register(b3);
ConstantBuffer<Index> addTransformAreaEmitterCount : register(b4);

void UpdateEmitterTime(inout EmitterTime time, bool isOnce, bool isAlive)
{
    if (isOnce)
    {
        if (isAlive)
        {
            time.particleTime = -1;
        }
        else
        {
            time.particleTime = 0;
        }
    }
}

void ReplaceEmitter(inout Emitter origEmitter, in Emitter addEmitter)
{
    EmitterTime time = origEmitter.time;
    UpdateEmitterTime(time, origEmitter.frequency.isOnce, origEmitter.isAlive);
    origEmitter = addEmitter;
    origEmitter.time = time;
}

void ReplaceEmitter(inout VertexEmitter origEmitter, in VertexEmitter addEmitter)
{
    EmitterTime time = origEmitter.time;
    UpdateEmitterTime(time, origEmitter.frequency.isOnce, origEmitter.isAlive);
    origEmitter = addEmitter;
    origEmitter.time = time;
}

void ReplaceEmitter(inout MeshEmitter origEmitter, in MeshEmitter addEmitter)
{
    EmitterTime time = origEmitter.time;
    UpdateEmitterTime(time, origEmitter.frequency.isOnce, origEmitter.isAlive);
    origEmitter = addEmitter;
    origEmitter.time = time;
}

void ReplaceEmitter(inout TransformModelEmitter origEmitter, in TransformModelEmitter addEmitter)
{
    EmitterTime time = origEmitter.time;
    UpdateEmitterTime(time, origEmitter.frequency.isOnce, origEmitter.isAlive);
    origEmitter = addEmitter;
    origEmitter.time = time;
}

void ReplaceEmitter(inout TransformAreaEmitter origEmitter, in TransformAreaEmitter addEmitter)
{
    EmitterTime time = origEmitter.time;
    UpdateEmitterTime(time, origEmitter.frequency.isOnce, origEmitter.isAlive);
    origEmitter = addEmitter;
    origEmitter.time = time;
}

bool ShouldReplaceEmitter(Emitter origEmitter, Emitter addEmitter)
{
    return (addEmitter.emitterCount == origEmitter.emitterCount && addEmitter.frequency.isLoop);
}

bool ShouldReplaceEmitter(VertexEmitter origEmitter, VertexEmitter addEmitter)
{
    return (addEmitter.emitterCount == origEmitter.emitterCount && addEmitter.frequency.isLoop);
}

bool ShouldReplaceEmitter(MeshEmitter origEmitter, MeshEmitter addEmitter)
{
    return (addEmitter.emitterCount == origEmitter.emitterCount && addEmitter.frequency.isLoop);
}

bool ShouldReplaceEmitter(TransformModelEmitter origEmitter, TransformModelEmitter addEmitter)
{
    return (addEmitter.emitterCount == origEmitter.emitterCount && addEmitter.frequency.isLoop);
}

bool ShouldReplaceEmitter(TransformAreaEmitter origEmitter, TransformAreaEmitter addEmitter)
{
    return (addEmitter.emitterCount == origEmitter.emitterCount && addEmitter.frequency.isLoop);
}


[numthreads(GPUParticleShaderStructs::MaxEmitterNum, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID, uint3 GTid : SV_GroupThreadID, uint3 GID : SV_GroupID)
{
    // グループスレッドIDを使用して、オリジナルエミッターのインデックスを取得
    uint32_t origalIndex = GTid.x;

    // グループIDを使用して、追加エミッターのインデックスを取得
    uint32_t addIndex = GID.x;
    if (addIndex < addEmitterCount.index) {
        if (ShouldReplaceEmitter(origalEmitter[origalIndex], addEmitter[addIndex])) 
        {
            ReplaceEmitter(origalEmitter[origalIndex], addEmitter[addIndex]);
            addEmitter[addIndex].isAlive = false;
        }
    } else if (addIndex < addEmitterCount.index + addVertexEmitterCount.index) {
        addIndex -= addEmitterCount.index;
        if (ShouldReplaceEmitter(origalVertexEmitter[origalIndex], addVertexEmitter[addIndex])) 
        {
            ReplaceEmitter(origalVertexEmitter[origalIndex], addVertexEmitter[addIndex]);
            addVertexEmitter[addIndex].isAlive = false;
        }
    } else if (addIndex < addEmitterCount.index + addVertexEmitterCount.index + addMeshEmitterCount.index) {
        addIndex -= (addEmitterCount.index + addVertexEmitterCount.index);
        if (ShouldReplaceEmitter(origalMeshEmitter[origalIndex], addMeshEmitter[addIndex])) 
        {
            ReplaceEmitter(origalMeshEmitter[origalIndex], addMeshEmitter[addIndex]);
            addMeshEmitter[addIndex].isAlive = false;
        }
    } else if (addIndex < addEmitterCount.index + addVertexEmitterCount.index + addMeshEmitterCount.index + addTransformModelEmitterCount.index) {
        addIndex -= (addEmitterCount.index + addVertexEmitterCount.index + addMeshEmitterCount.index);
        if (ShouldReplaceEmitter(origalTransformModelEmitter[origalIndex], addTransformModelEmitter[addIndex])) 
        {
            ReplaceEmitter(origalTransformModelEmitter[origalIndex], addTransformModelEmitter[addIndex]);
            addTransformModelEmitter[addIndex].isAlive = false;
        }
    } else if (addIndex < addEmitterCount.index + addVertexEmitterCount.index + addMeshEmitterCount.index + addTransformModelEmitterCount.index + addTransformAreaEmitterCount.index) {
        addIndex -= (addEmitterCount.index + addVertexEmitterCount.index + addMeshEmitterCount.index + addTransformModelEmitterCount.index);
        if (ShouldReplaceEmitter(origalTransformAreaEmitter[origalIndex], addTransformAreaEmitter[addIndex])) 
        {
            ReplaceEmitter(origalTransformAreaEmitter[origalIndex], addTransformAreaEmitter[addIndex]);
            addTransformAreaEmitter[addIndex].isAlive = false;
        }
    }
}
