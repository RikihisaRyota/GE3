#include "GPUParticle.hlsli"
#include "GPUParticleShaderStructs.h"

// エミッター用のバッファ
RWStructuredBuffer<GPUParticleShaderStructs::EmitterForGPU> addEmitter : register(u0);
RWStructuredBuffer<GPUParticleShaderStructs::EmitterForGPU> origalEmitter : register(u1);

RWStructuredBuffer<GPUParticleShaderStructs::VertexEmitterForGPU> addVertexEmitter : register(u2);
RWStructuredBuffer<GPUParticleShaderStructs::VertexEmitterForGPU> origalVertexEmitter : register(u3);

RWStructuredBuffer<GPUParticleShaderStructs::MeshEmitterForGPU> addMeshEmitter : register(u4);
RWStructuredBuffer<GPUParticleShaderStructs::MeshEmitterForGPU> origalMeshEmitter : register(u5);

RWStructuredBuffer<GPUParticleShaderStructs::TransformModelEmitterForGPU> addTransformModelEmitter : register(u6);
RWStructuredBuffer<GPUParticleShaderStructs::TransformModelEmitterForGPU> origalTransformModelEmitter : register(u7);

RWStructuredBuffer<GPUParticleShaderStructs::TransformAreaEmitterForGPU> addTransformAreaEmitter : register(u8);
RWStructuredBuffer<GPUParticleShaderStructs::TransformAreaEmitterForGPU> origalTransformAreaEmitter : register(u9);

// エミッターの数を表す定数バッファ
struct Index{
    int32_t index;
};
ConstantBuffer<Index> addEmitterCount : register(b0);
ConstantBuffer<Index> addVertexEmitterCount : register(b1);
ConstantBuffer<Index> addMeshEmitterCount : register(b2);
ConstantBuffer<Index> addTransformModelEmitterCount : register(b3);
ConstantBuffer<Index> addTransformAreaEmitterCount : register(b4);

void UpdateEmitterTime(inout GPUParticleShaderStructs::EmitterTime time, bool isOnce, bool isAlive)
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

void ReplaceEmitter(inout GPUParticleShaderStructs::EmitterForGPU origEmitter, in GPUParticleShaderStructs::EmitterForGPU addEmitter)
{
    GPUParticleShaderStructs::EmitterTime time = origEmitter.time;
    UpdateEmitterTime(time, origEmitter.frequency.isOnce, origEmitter.isAlive);
    origEmitter = addEmitter;
    origEmitter.time = time;
}

void ReplaceEmitter(inout GPUParticleShaderStructs::VertexEmitterForGPU origEmitter, in GPUParticleShaderStructs::VertexEmitterForGPU addEmitter)
{
    GPUParticleShaderStructs::EmitterTime time = origEmitter.time;
    UpdateEmitterTime(time, origEmitter.frequency.isOnce, origEmitter.isAlive);
    origEmitter = addEmitter;
    origEmitter.time = time;
}

void ReplaceEmitter(inout GPUParticleShaderStructs::MeshEmitterForGPU origEmitter, in GPUParticleShaderStructs::MeshEmitterForGPU addEmitter)
{
    GPUParticleShaderStructs::EmitterTime time = origEmitter.time;
    UpdateEmitterTime(time, origEmitter.frequency.isOnce, origEmitter.isAlive);
    origEmitter = addEmitter;
    origEmitter.time = time;
}

void ReplaceEmitter(inout GPUParticleShaderStructs::TransformModelEmitterForGPU origEmitter, in GPUParticleShaderStructs::TransformModelEmitterForGPU addEmitter)
{
    GPUParticleShaderStructs::EmitterTime time = origEmitter.time;
    UpdateEmitterTime(time, origEmitter.frequency.isOnce, origEmitter.isAlive);
    origEmitter = addEmitter;
    origEmitter.time = time;
}

void ReplaceEmitter(inout GPUParticleShaderStructs::TransformAreaEmitterForGPU origEmitter, in GPUParticleShaderStructs::TransformAreaEmitterForGPU addEmitter)
{
    GPUParticleShaderStructs::EmitterTime time = origEmitter.time;
    UpdateEmitterTime(time, origEmitter.frequency.isOnce, origEmitter.isAlive);
    origEmitter = addEmitter;
    origEmitter.time = time;
}

bool ShouldReplaceEmitter(GPUParticleShaderStructs::EmitterForGPU origEmitter, GPUParticleShaderStructs::EmitterForGPU addEmitter)
{
    return (addEmitter.emitterCount == origEmitter.emitterCount && addEmitter.frequency.isLoop);
}

bool ShouldReplaceEmitter(GPUParticleShaderStructs::VertexEmitterForGPU origEmitter, GPUParticleShaderStructs::VertexEmitterForGPU addEmitter)
{
    return (addEmitter.emitterCount == origEmitter.emitterCount && addEmitter.frequency.isLoop);
}

bool ShouldReplaceEmitter(GPUParticleShaderStructs::MeshEmitterForGPU origEmitter, GPUParticleShaderStructs::MeshEmitterForGPU addEmitter)
{
    return (addEmitter.emitterCount == origEmitter.emitterCount && addEmitter.frequency.isLoop);
}

bool ShouldReplaceEmitter(GPUParticleShaderStructs::TransformModelEmitterForGPU origEmitter, GPUParticleShaderStructs::TransformModelEmitterForGPU addEmitter)
{
    return (addEmitter.emitterCount == origEmitter.emitterCount && addEmitter.frequency.isLoop);
}

bool ShouldReplaceEmitter(GPUParticleShaderStructs::TransformAreaEmitterForGPU origEmitter, GPUParticleShaderStructs::TransformAreaEmitterForGPU addEmitter)
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
