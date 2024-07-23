#include "GPUParticle.hlsli"

RWStructuredBuffer<Particle> Output : register(u0);

ConsumeStructuredBuffer<uint> particleIndexCommands : register(u1);

RWStructuredBuffer<int> particleIndexCounter : register(u2);

struct Vertex
{
    float32_t4 position;
    float32_t3 normal;
    float32_t2 texcoord;
};

StructuredBuffer<Vertex> vertices : register(t0);

struct Index{
    uint32_t index;
};

ConstantBuffer<Index> verticeSize : register(b0);

struct WorldTransform
{
    float32_t4x4 world;
    float32_t4x4 inverseMatWorld;
};

ConstantBuffer<WorldTransform> worldTransform : register(b1);

ConstantBuffer<TransformAreaEmitter> transformEmitter : register(b2);

ConstantBuffer<Index> gRandom : register(b3);
[numthreads(meshThreadBlockSize, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    //uint32_t index = DTid.x;
    //if(index >= verticeSize.index){
    //    return;
    //}
    //uint32_t seed = gRandom.index*index;
    //int32_t createParticlenum = -1;
    //InterlockedAdd(particleIndexCounter[0], -1, createParticlenum);
    //if (createParticlenum > 0){
    //    int32_t particleIndex = particleIndexCommands.Consume();
    //    TransformEmitter emitter=transformEmitter;
    //    emitter.translate.isEasing=true;
    //    emitter.translate.easing.max = mul(vertices[index].position,worldTransform.world).xyz;
    //    //CreateParticle(Output[particleIndex], emitter,seed,true);
    //}
}