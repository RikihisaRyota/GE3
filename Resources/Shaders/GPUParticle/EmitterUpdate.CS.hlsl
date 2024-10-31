#include "GPUParticle.hlsli"
#include "GPUParticleShaderStructs.h"

// バッファの宣言
RWStructuredBuffer<EmitterForGPU> inputEmitter : register(u0);
RWStructuredBuffer<VertexEmitterForGPU> inputVertexEmitter : register(u1);
RWStructuredBuffer<MeshEmitterForGPU> inputMeshEmitter : register(u2);
RWStructuredBuffer<TransformModelEmitterForGPU> inputTransformModelEmitter : register(u3);
RWStructuredBuffer<TransformAreaEmitterForGPU> inputTransformAreaEmitter : register(u4);
// エミッターごとの生成パーティクル数

AppendStructuredBuffer<CreateParticleNum> createParticle : register(u5);
// 合計何個パーティクルを生成するか
RWStructuredBuffer<uint> createParticleCounter : register(u6);

// エミッターの時間を更新する関数
void UpdateEmitterTime(inout Emitter emitter,uint32_t index) {
    if (emitter.time.particleTime >= emitter.frequency.interval) {
        // パーティクル生成の条件を満たす場合
        CreateParticleNum particle;
        particle.emitterNum = index;
        particle.emitterType = 0;
        particle.createParticleNum = emitter.createParticleNum;
        particle.maxCreateParticleNum = emitter.createParticleNum;

        // インターロック処理
        InterlockedAdd(createParticleCounter[0], particle.createParticleNum);
        createParticle.Append(particle);

        // タイマーのリセット
        if(!emitter.frequency.isOnce){
            emitter.time.particleTime = 0;
        }else{
            emitter.time.particleTime = -1;
        }
    } else {
        if(emitter.time.particleTime != -1){
            emitter.time.particleTime++;
        }
    }
}

void UpdateEmitterTime(inout VertexEmitter emitter,uint32_t index) {
    if (emitter.time.particleTime >= emitter.frequency.interval) {
        // パーティクル生成の条件を満たす場合
        CreateParticleNum particle;
        particle.emitterNum = index;
        particle.emitterType = 1;
        particle.createParticleNum = emitter.model.vertexCount;
        particle.maxCreateParticleNum = particle.createParticleNum;

        // インターロック処理
        InterlockedAdd(createParticleCounter[0], particle.createParticleNum);
        createParticle.Append(particle);

         // タイマーのリセット
        if(!emitter.frequency.isOnce){
            emitter.time.particleTime = 0;
        }else{
            emitter.time.particleTime = -1;
        }
    } else {
        if(emitter.time.particleTime != -1){
            emitter.time.particleTime++;
        }
    }
}

void UpdateEmitterTime(inout MeshEmitter emitter,uint32_t index) {
    if (emitter.time.particleTime >= emitter.frequency.interval) {
        // パーティクル生成の条件を満たす場合
        CreateParticleNum particle;
        particle.emitterNum = index;
        particle.emitterType = 2;
        particle.createParticleNum = (emitter.model.indexCount) * emitter.numCreate;
        particle.maxCreateParticleNum = particle.createParticleNum;

        // インターロック処理
        InterlockedAdd(createParticleCounter[0], particle.createParticleNum);
        createParticle.Append(particle);

        // タイマーのリセット
        if(!emitter.frequency.isOnce){
            emitter.time.particleTime = 0;
        }else{
            emitter.time.particleTime = -1;
        }
    } else {
        if(emitter.time.particleTime != -1){
            emitter.time.particleTime++;
        }
    }
}

void UpdateEmitterTime(inout TransformModelEmitter emitter,uint32_t index) {
    if (emitter.time.particleTime >= emitter.frequency.interval) {
        // パーティクル生成の条件を満たす場合
        CreateParticleNum particle;
        particle.emitterNum = index;
        particle.emitterType = 3;
        particle.createParticleNum =  max(emitter.startModel.vertexCount,emitter.endModel.vertexCount);
        particle.maxCreateParticleNum = particle.createParticleNum;
        // インターロック処理
        InterlockedAdd(createParticleCounter[0], particle.createParticleNum);
        createParticle.Append(particle);

        // タイマーのリセット
        if(!emitter.frequency.isOnce){
            emitter.time.particleTime = 0;
        }else{
            emitter.time.particleTime = -1;
        }
    } else {
        if(emitter.time.particleTime != -1){
            emitter.time.particleTime++;
        }
    }
}

void UpdateEmitterTime(inout TransformAreaEmitter emitter,uint32_t index) {
    if (emitter.time.particleTime >= emitter.frequency.interval) {
        // パーティクル生成の条件を満たす場合
        CreateParticleNum particle;
        particle.emitterNum = index;
        particle.emitterType = 4;
        particle.createParticleNum =  emitter.model.vertexCount;
        particle.maxCreateParticleNum = particle.createParticleNum;

        // インターロック処理
        InterlockedAdd(createParticleCounter[0], particle.createParticleNum);
        createParticle.Append(particle);

        // タイマーのリセット
        if(!emitter.frequency.isOnce){
            emitter.time.particleTime = 0;
        }else{
            emitter.time.particleTime = -1;
        }
    } else {
        if(emitter.time.particleTime != -1){
            emitter.time.particleTime++;
        }
    }
}


// エミッターの生存状態をチェックして更新する関数
void UpdateEmitterState(inout Emitter emitter) {
    if (!emitter.frequency.isLoop) {
        if (emitter.time.emitterTime >= emitter.frequency.emitterLife) {
            emitter.isAlive = false;
            emitter.emitterCount = -1;
            emitter.time.particleTime = 0;
            emitter.time.emitterTime = 0;
        } else {
            emitter.time.emitterTime++;
        }
    }
}

void UpdateEmitterState(inout VertexEmitter emitter) {
    if (!emitter.frequency.isLoop) {
        if (emitter.time.emitterTime >= emitter.frequency.emitterLife) {
            emitter.isAlive = false;
            emitter.emitterCount = -1;
            emitter.time.particleTime = 0;
            emitter.time.emitterTime = 0;
        } else {
            emitter.time.emitterTime++;
        }
    }
}

void UpdateEmitterState(inout MeshEmitter emitter) {
    if (!emitter.frequency.isLoop) {
        if (emitter.time.emitterTime >= emitter.frequency.emitterLife) {
            emitter.isAlive = false;
            emitter.emitterCount = -1;
            emitter.time.particleTime = 0;
            emitter.time.emitterTime = 0;
        } else {
            emitter.time.emitterTime++;
        }
    }
}

void UpdateEmitterState(inout TransformModelEmitter emitter) {
    if (!emitter.frequency.isLoop) {
        if (emitter.time.emitterTime >= emitter.frequency.emitterLife) {
            emitter.isAlive = false;
            emitter.emitterCount = -1;
            emitter.time.particleTime = 0;
            emitter.time.emitterTime = 0;
        } else {
            emitter.time.emitterTime++;
        }
    }
}

void UpdateEmitterState(inout TransformAreaEmitter emitter) {
    if (!emitter.frequency.isLoop) {
        if (emitter.time.emitterTime >= emitter.frequency.emitterLife) {
            emitter.isAlive = false;
            emitter.emitterCount = -1;
            emitter.time.particleTime = 0;
            emitter.time.emitterTime = 0;
        } else {
            emitter.time.emitterTime++;
        }
    }
}

[numthreads(GPUParticleShaderStructs::MaxEmitterNum, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint index = DTid.x;

    if (index < GPUParticleShaderStructs::MaxEmitterNum && inputEmitter[index].isAlive)
    {
        UpdateEmitterTime(inputEmitter[index],index);
        UpdateEmitterState(inputEmitter[index]);
    }
    if (index < GPUParticleShaderStructs::MaxEmitterNum && inputVertexEmitter[index].isAlive)
    {
        UpdateEmitterTime(inputVertexEmitter[index],index);
        UpdateEmitterState(inputVertexEmitter[index]);
    }
    if (index < GPUParticleShaderStructs::MaxEmitterNum && inputMeshEmitter[index].isAlive)
    {
        UpdateEmitterTime(inputMeshEmitter[index],index);
        UpdateEmitterState(inputMeshEmitter[index]);
    }
    if (index < GPUParticleShaderStructs::MaxEmitterNum && inputTransformModelEmitter[index].isAlive)
    {
        UpdateEmitterTime(inputTransformModelEmitter[index],index);
        UpdateEmitterState(inputTransformModelEmitter[index]);
    }
    if (index < GPUParticleShaderStructs::MaxEmitterNum && inputTransformAreaEmitter[index].isAlive)
    {
        UpdateEmitterTime(inputTransformAreaEmitter[index],index);
        UpdateEmitterState(inputTransformAreaEmitter[index]);
    }
}
