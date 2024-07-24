#include "GPUParticle.hlsli"

RWStructuredBuffer<Particle> Output : register(u0);
ConsumeStructuredBuffer<uint> particleIndexCommands : register(u1);
// エミッター一つの生成情報
RWStructuredBuffer<CreateParticleNum> createParticle : register(u2);
struct CounterParticle
{
    int32_t count;
};
RWStructuredBuffer<CounterParticle> particleIndexCounter : register(u3);

StructuredBuffer<Emitter> gEmitter : register(t0);
StructuredBuffer<VertexEmitter> gVertexEmitter : register(t1);
StructuredBuffer<MeshEmitter> gMeshEmitter : register(t2);
StructuredBuffer<TransformModelEmitter> gTransformModelEmitter : register(t3);
StructuredBuffer<TransformAreaEmitter> gTransformAreaEmitter : register(t4);

struct Vertex
{
    float32_t4 position;
    float32_t3 normal;
    float32_t2 texcoord;
};

struct Index{
    uint32_t index;
};

StructuredBuffer<Vertex> vertexBuffers[] : register(t5, space0);
StructuredBuffer<Index> indexBuffers[] : register(t6, space1);

struct Random
{
    uint32_t random;
};


ConstantBuffer<Random> gRandom : register(b0);

[numthreads(1, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID, uint3 GTid : SV_GroupThreadID, uint3 GID : SV_GroupID)
{
    // グループスレッドIDを使用して、オリジナルエミッターのインデックスを取得
    uint32_t origalIndex = GTid.x;

    // グループIDを使用(dispach数)
    uint32_t emitterNum  = GID.y;

    if(createParticle[emitterNum].emitterType == 0){
        if(createParticle[emitterNum].createParticleNum > 0){
            int32_t createNum=-1; 
            InterlockedAdd(createParticle[emitterNum].createParticleNum, -1,createNum);
            if (createNum > 0)
            { 
                int32_t counter=-1;
                InterlockedAdd(particleIndexCounter[0].count, -1,counter);
                if(counter>0){
                    int index = particleIndexCommands.Consume();
                    uint32_t seed = setSeed(index * gRandom.random);
                    uint32_t emitterIndex=createParticle[emitterNum].emitterNum;
                    Emitter emitter=gEmitter[emitterIndex];
                    CreateParticle(Output[index], emitter,seed,emitterIndex);
                }
            }
        }
    }else if(createParticle[emitterNum].emitterType == 1){
        if(createParticle[emitterNum].createParticleNum > 0){
            int32_t createNum=-1; 
            InterlockedAdd(createParticle[emitterNum].createParticleNum, -1,createNum);
            if (createNum > 0)
            { 
                int32_t counter=-1;
                InterlockedAdd(particleIndexCounter[0].count, -1,counter);
                if(counter>0){
                    int32_t index = particleIndexCommands.Consume();
                    uint32_t seed = setSeed(index * gRandom.random);
                    uint32_t emitterIndex=createParticle[emitterNum].emitterNum;
                    VertexEmitter emitter = gVertexEmitter[emitterIndex];
                    float32_t4x4 worldMatrix;
                    if(!emitter.parent.isParent){
                        worldMatrix=MakeAffine(float32_t3(1.0f,1.0f,1.0f),emitter.localTransform.rotate,emitter.localTransform.translate);
                    }else{
                        worldMatrix=mul(MakeAffine(float32_t3(1.0f,1.0f,1.0f),emitter.localTransform.rotate,emitter.localTransform.translate),emitter.parent.worldMatrix);
                    }
                    float32_t3 translate =  mul(vertexBuffers[emitter.model.vertexBufferIndex][createNum].position,worldMatrix).xyz;
                    CreateParticle(Output[index], emitter,translate,seed,emitterIndex);
                }
            }
        }
    }else if(createParticle[emitterNum].emitterType == 2){
        if(createParticle[emitterNum].createParticleNum > 0){
            int32_t createNum=-1; 
            InterlockedAdd(createParticle[emitterNum].createParticleNum, -1,createNum);
            if (createNum > 0)
            { 
                int32_t counter=-1;
                InterlockedAdd(particleIndexCounter[0].count, -3,counter);
                if(counter>0){
                    int32_t index = particleIndexCommands.Consume();
                    uint32_t seed = setSeed(index * gRandom.random);
                    uint32_t emitterIndex=createParticle[emitterNum].emitterNum;
                    MeshEmitter emitter = gMeshEmitter[emitterIndex];

                    // 三角形の頂点インデックスを取得
                    uint32_t3 triIndices = uint32_t3(
                        indexBuffers[emitter.model.indexBufferIndex][createNum * 3].index,
                        indexBuffers[emitter.model.indexBufferIndex][createNum * 3 + 1].index,
                        indexBuffers[emitter.model.indexBufferIndex][createNum * 3 + 2].index
                    );

                    // 三角形の頂点座標を取得
                    float32_t4x4 worldMatrix;
                    if(!emitter.parent.isParent){
                        worldMatrix=MakeAffine(float32_t3(1.0f,1.0f,1.0f),emitter.localTransform.rotate,emitter.localTransform.translate);
                    }else{
                        worldMatrix=mul(MakeAffine(float32_t3(1.0f,1.0f,1.0f),emitter.localTransform.rotate,emitter.localTransform.translate),emitter.parent.worldMatrix);
                    }
                    float32_t3 v1 = mul(vertexBuffers[emitter.model.vertexBufferIndex][triIndices.y].position, worldMatrix).xyz;
                    float32_t3 v0 = mul(vertexBuffers[emitter.model.vertexBufferIndex][triIndices.x].position, worldMatrix).xyz;
                    float32_t3 v2 = mul(vertexBuffers[emitter.model.vertexBufferIndex][triIndices.z].position, worldMatrix).xyz;

                    // ランダムなバリセンター座標を生成
                    float32_t u = randFloat(seed);
                    float32_t v = randFloat(seed);
                    float32_t w = 1.0f - u - v;
                    if (w < 0.0f) {
                        u = 1.0f - u;
                        v = 1.0f - v;
                        w = 1.0f - u - v;
                    }

                    // ランダムな点を計算
                    float32_t3 p = v0 + u * (v1 - v0) + v * (v2 - v0);
                    CreateParticle(Output[index], emitter,p,seed,emitterIndex);
                    }
                }
            }
    }else if(createParticle[emitterNum].emitterType == 3){
        if(createParticle[emitterNum].createParticleNum > 0){
            int32_t createNum=-1; 
            InterlockedAdd(createParticle[emitterNum].createParticleNum, -1,createNum);
            if (createNum > 0)
            { 
                int32_t counter=-1;
                InterlockedAdd(particleIndexCounter[0].count, -1,counter);
                if(counter>0){
                    int32_t particleIndex = particleIndexCommands.Consume();
                    uint32_t seed = setSeed(particleIndex * gRandom.random);
                    uint32_t emitterIndex=createParticle[emitterNum].emitterNum;
                    TransformModelEmitter emitter = gTransformModelEmitter[emitterIndex];
                    uint32_t startModelIndex = createNum % emitter.startModel.vertexCount;
                    uint32_t endModelIndex = createNum % emitter.endModel.vertexCount;
                    float32_t4x4 startWorldMatrix,endWorldMatrix;
                    if(!emitter.parent.isParent){
                        startWorldMatrix=emitter.startModelWorldMatrix;
                        endWorldMatrix=emitter.endModelWorldMatrix;
                    }else{
                        startWorldMatrix=mul(emitter.startModelWorldMatrix,emitter.parent.worldMatrix);
                        endWorldMatrix=mul(emitter.endModelWorldMatrix,emitter.parent.worldMatrix);
                    }
                    emitter.translate.isEasing=true;
                    emitter.translate.easing.min = mul(vertexBuffers[emitter.startModel.vertexBufferIndex][startModelIndex].position,startWorldMatrix).xyz;
                    emitter.translate.easing.max = mul(vertexBuffers[emitter.endModel.vertexBufferIndex][endModelIndex].position,endWorldMatrix).xyz;
                    CreateParticle(Output[particleIndex], emitter,seed,emitterIndex);
                }
            }
        }
    }else if(createParticle[emitterNum].emitterType == 4){
        if(createParticle[emitterNum].createParticleNum > 0){
            int32_t createNum=-1; 
            InterlockedAdd(createParticle[emitterNum].createParticleNum, -1,createNum);
            if (createNum > 0)
            { 
                int32_t counter=-1;
                InterlockedAdd(particleIndexCounter[0].count, -1,counter);
                if(counter>0){
                    int32_t particleIndex = particleIndexCommands.Consume();
                    uint32_t seed = setSeed(particleIndex * gRandom.random);
                    uint32_t emitterIndex=createParticle[emitterNum].emitterNum;
                    TransformAreaEmitter emitter = gTransformAreaEmitter[emitterIndex];
                    uint32_t modelIndex = createNum % emitter.model.vertexCount;
                    float32_t4x4 worldMatrix;
                    if(!emitter.parent.isParent){
                        worldMatrix=emitter.modelWorldMatrix;
                    }else{
                        worldMatrix=mul(emitter.modelWorldMatrix,emitter.parent.worldMatrix);
                    }
                    emitter.translate.isEasing=true;
                    emitter.translate.easing.max = mul(vertexBuffers[emitter.model.vertexBufferIndex][modelIndex].position,worldMatrix).xyz;
                    CreateParticle(Output[particleIndex], emitter,seed,emitterIndex);
                }
            }
        }
    }

}