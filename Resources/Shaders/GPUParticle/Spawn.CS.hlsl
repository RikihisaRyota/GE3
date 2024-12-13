#include "GPUParticle.hlsli"
#include "GPUParticleShaderStructs.h"

RWStructuredBuffer<GPUParticleShaderStructs::Particle> directParticle : register(u0);
RWStructuredBuffer<uint32_t> directFreeList : register(u1);
RWStructuredBuffer<int32_t> directFreeLsitIndex : register(u2);
RWStructuredBuffer<GPUParticleShaderStructs::Particle> computeParticle : register(u3);
RWStructuredBuffer<uint32_t> computeFreeList : register(u4);
RWStructuredBuffer<int32_t> computeFreeLsitIndex : register(u5);
// エミッター一つの生成情報
RWStructuredBuffer<GPUParticleShaderStructs::CreateParticle> createParticle : register(u6);

ConsumeStructuredBuffer<int> trailsStock : register(u8);
RWStructuredBuffer<GPUParticleShaderStructs::TrailsData> trailsData : register(u9);
RWStructuredBuffer<GPUParticleShaderStructs::TrailsHead> trailsHead : register(u10);

StructuredBuffer<GPUParticleShaderStructs::EmitterForGPU> gEmitter : register(t11);
StructuredBuffer<GPUParticleShaderStructs::VertexEmitterForGPU> gVertexEmitter : register(t12);
StructuredBuffer<GPUParticleShaderStructs::MeshEmitterForGPU> gMeshEmitter : register(t13);
StructuredBuffer<GPUParticleShaderStructs::TransformModelEmitterForGPU> gTransformModelEmitter : register(t14);
StructuredBuffer<GPUParticleShaderStructs::TransformAreaEmitterForGPU> gTransformAreaEmitter : register(t15);

struct Vertex
{
    float32_t4 position;
    float32_t3 normal;
    float32_t2 texcoord;
};

struct Index{
    uint32_t index;
};

StructuredBuffer<Vertex> vertexBuffers[] : register(t16, space0);
StructuredBuffer<Index> indexBuffers[] : register(t17, space1);

struct Random
{
    uint32_t random;
};


ConstantBuffer<Random> gRandom : register(b0);

void GetParticleIndex(inout uint32_t particleType,inout int32_t particleIndex){
    int32_t freeListIndex = -1; 
    if(directFreeListIndex[0] > 0){
        // FreeristIndexをデクリメント
        InterlockedAdd(directFreeListIndex[0], -1, freeListIndex);
        // パーティクルタイプDirect
        particleType = 0;
        // フリーリストが空っぽじゃないか
        if(freeListIndex >= 0){
            particleIndex = directFreeList[freeListIndex];
        } 
        // 空だったら戻しておく
        else {
            InterlockedAdd(directFreeListIndex[0], 1);
        }
    }else if(computeFreeListIndex[0] > 0) {
        // FreeristIndexをデクリメント
        InterlockedAdd(computeFreeListIndex[0], -1, freeListIndex);
        // パーティクルタイプCompute
        particleType = 1;
        // フリーリストが空っぽじゃないか
        if(freeListIndex >= 0){
            particleIndex = computeFreeList[freeListIndex];
        } 
        // 空だったら戻しておく
        else {
            InterlockedAdd(computeFreeListIndex[0], 1);
        }
    } 
}

void SetParticle(inout uint32_t particleType,inout int32_t particleIndex,GPUParticleShaderStructs::Particle particle){
    // パーティクルインデックスを取得
    GetParticleIndex( particleType,particleIndex);
    if(particleType==0){
        directParticle[particleIndex] = particle;
    }else if(particleType==1){
        computeParticle[particleIndex] = particle;
    }
}

void CheckTrailsData(GPUParticleShaderStructs::EmitterTrails emitterTrails,uint32_t particleIndex){
    if(emitterTrails.isTrails == true){
        int32_t trailsRangeValue = GPUParticleShaderStructs::TrailsRange;
        int32_t originalValue;

        // headIndex をインクリメントし、元の値を取得
        InterlockedAdd(trailsHead[0].headIndex, trailsRangeValue, originalValue);

        // 加算後の headIndex を計算
        int32_t headCount = originalValue + trailsRangeValue;

        // headCount が範囲を超えた場合の処理
        if (headCount >= GPUParticleShaderStructs::MaxTrailsTotal) {
            headCount = headCount % GPUParticleShaderStructs::MaxTrailsTotal;
        }
        GPUParticleShaderStructs::TrailsData data;
        
        data.particleIndex = particleIndex;
        data.isAlive = true;
        data.trailsIndex = trailsStock.Consume();
        data.startIndex = headCount;
        data.endIndex = data.startIndex + GPUParticleShaderStructs::TrailsRange;
        data.currentIndex = data.startIndex;
        
        data.width =  emitterTrails.width;
        data.textureIndex =  emitterTrails.textureIndex;
        data.interval = emitterTrails.interval;
        data.time = data.interval;
        data.loopNum = 0;

        data.lifeLimit = emitterTrails.lifeLimit;
        trailsData[data.trailsIndex] = data;
    }
}

[numthreads(1, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID, uint3 GTid : SV_GroupThreadID, uint3 GID : SV_GroupID)
{
    // グループスレッドIDを使用して、オリジナルエミッターのインデックスを取得
    uint32_t origalIndex = GTid.x;
    // グループIDを使用(dispach数)
    uint32_t emitterNum  = GID.y;
    // seed値の初期化
    uint32_t seed = setSeed(origalIndex * gRandom.random);
    // 作成できない場合早期リターン
    if(directFreeList[0] <= 0 || computeFreeList[0] <= 0) {
        return;
    }
    // dispach数が多い場合リターン
    if(origalIndex > GPUParticleShaderStructs::DivisionParticleNum){
        return;
    }
    // 生成するパーティクルが無かったらリターン
    if(createParticle[emitterNum].createParticleNum > 0){
        return;
    }
    // 生成すべきパーティクルの残り数
    int32_t createNum=-1; 
    InterlockedAdd(createParticle[emitterNum].createParticleNum, -1,createNum);
    // エミッターインデックス取得
    uint32_t emitterIndex=createParticle[emitterNum].emitterIndex;
    // パーティクル
    GPUParticleShaderStructs::Particle particle=(GPUParticleShaderStructs::Particle)0;
    // パーティクルインデックス
    int32_t particleIndex=-1;
    // パーティクルタイプ
    int32_t particleType=-1;
    // エミッターごとの処理
    if(createParticle[emitterNum].emitterType == 0){
        // エミッター
        GPUParticleShaderStructs::EmitterForGPU emitter=gEmitter[emitterIndex];
        // パーティクル初期化
        CreateParticle(particle, emitter,seed,emitterIndex);
    }else if(createParticle[emitterNum].emitterType == 1){
        GPUParticleShaderStructs::VertexEmitterForGPU emitter = gVertexEmitter[emitterIndex];
        float32_t4x4 worldMatrix;
        float32_t3 translate=float32_t3(0.0f,0.0f,0.0f);
        float32_t4 vertexPosition=float32_t4(0.0f,0.0f,0.0f,0.0f);
        GPUParticleShaderStructs::TriangleInfo info;
        
        // エミッターの座標
        worldMatrix = MakeAffine(emitter.localTransform.scale,emitter.localTransform.rotate,emitter.localTransform.translate);
        // 頂点座標
        vertexPosition = vertexBuffers[emitter.model.vertexBufferIndex][createNum].position;
        // ワールド座標
        translate =  mul(vertexPosition,worldMatrix).xyz;
        emitter.translate.easing.max = translate;
        // メッシュの計算
        info.vertex = float32_t3 (1.0f,1.0f,1.0f);
        info.weight = float32_t3(1.0f,1.0f,1.0f);
        CreateParticle(particle, emitter,translate,info,seed,emitterIndex);
    }else if(createParticle[emitterNum].emitterType == 2){
        GPUParticleShaderStructs::MeshEmitterForGPU emitter = gMeshEmitter[emitterIndex];
        float32_t4x4 worldMatrix;
        float32_t3 translate;
        float32_t4 vertexPosition;
        GPUParticleShaderStructs::TriangleInfo info;

        // 0からに
        createNum = createParticle[emitterNum].maxCreateParticleNum - createNum;
        // 三角形の頂点インデックスを取得
        uint32_t3 triIndices = uint32_t3(
            indexBuffers[emitter.model.indexBufferIndex][createNum * 3].index,
            indexBuffers[emitter.model.indexBufferIndex][createNum * 3 + 1].index,
            indexBuffers[emitter.model.indexBufferIndex][createNum * 3 + 2].index
        );
        info.vertex = triIndices;
        // ランダムなバリセンター座標を生成
        float32_t u = randFloat(seed);
        float32_t v = randFloat(seed);
        float32_t w = 1.0f - u - v;
        if (w < 0.0f) {
            u = 1.0f - u;
            v = 1.0f - v;
            w = 1.0f - u - v;
        }
        info.weight=float32_t3(u,v,w);
        vertexPosition= vertexBuffers[emitter.model.vertexBufferIndex][info.vertex.x].position*info.weight.x + 
                        vertexBuffers[emitter.model.vertexBufferIndex][info.vertex.y].position*info.weight.y +
                        vertexBuffers[emitter.model.vertexBufferIndex][info.vertex.z].position*info.weight.z;
        worldMatrix = MakeAffine(emitter.localTransform.scale,emitter.localTransform.rotate,emitter.localTransform.translate);
        translate =  mul(vertexPosition,worldMatrix).xyz;
        emitter.translate.easing.max = translate;
        CreateParticle(particle, emitter,translate,info,seed,emitterIndex);
    }else if(createParticle[emitterNum].emitterType == 3){
        GPUParticleShaderStructs::TransformModelEmitterForGPU emitter = gTransformModelEmitter[emitterIndex];
        // 重なり防止
        if(emitter.startModel.vertexCount >= emitter.endModel.vertexCount){
            if(createNum / emitter.endModel.vertexCount >= 1){
                return;
            }
        }
        float32_t4x4 startWorldMatrix,endWorldMatrix;
        uint32_t startModelIndex = createNum % emitter.startModel.vertexCount;
        uint32_t endModelIndex = createNum % emitter.endModel.vertexCount;
        startWorldMatrix=emitter.startModelWorldMatrix;
        endWorldMatrix=emitter.endModelWorldMatrix;
        emitter.translate.isEasing=true;
        emitter.translate.easing.min = mul(vertexBuffers[emitter.startModel.vertexBufferIndex][startModelIndex].position,startWorldMatrix).xyz;
        emitter.translate.easing.max = mul(vertexBuffers[emitter.endModel.vertexBufferIndex][endModelIndex].position,endWorldMatrix).xyz;
        CreateParticle(particle, emitter,seed,emitterIndex);
    }else if(createParticle[emitterNum].emitterType == 4){
        GPUParticleShaderStructs::TransformAreaEmitterForGPU emitter = gTransformAreaEmitter[emitterIndex];
        uint32_t modelIndex = createNum % emitter.model.vertexCount;
        float32_t4x4 worldMatrix;
        worldMatrix=emitter.modelWorldMatrix;
        emitter.translate.isEasing=true;
        emitter.translate.easing.max = mul(vertexBuffers[emitter.model.vertexBufferIndex][modelIndex].position,worldMatrix).xyz;
        CreateParticle(particle, emitter,seed,emitterIndex);
    }
    // パーティクルをセット
    SetParticle(particleType,particleIndex,particle);
    // トレイル初期化(パーティクルを分割したため機能しない)
    //CheckTrailsData(emitter.emitterTrails,index);
}