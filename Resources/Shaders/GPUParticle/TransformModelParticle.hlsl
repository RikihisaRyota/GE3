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

StructuredBuffer<Vertex> startVertices : register(t0);
StructuredBuffer<Vertex> endVertices : register(t1);

struct Index{
    uint32_t index;
};

ConstantBuffer<Index> startVerticeSize : register(b0);
ConstantBuffer<Index> endVerticeSize : register(b1);

struct WorldTransform
{
    float32_t4x4 world;
    float32_t4x4 inverseMatWorld;
};

ConstantBuffer<WorldTransform> startWorldTransform : register(b2);
ConstantBuffer<WorldTransform> endWorldTransform : register(b3);
struct Time{
    float32_t time;
};

ConstantBuffer<Time> easingTime : register(b4);

ConstantBuffer<MeshEmitter> meshEmitter : register(b5);

ConstantBuffer<Index> gRandom : register(b6);
[numthreads(meshThreadBlockSize, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    uint32_t index = DTid.x;
    uint32_t sumCreateParticle=lerp(startVerticeSize.index,endVerticeSize.index,easingTime.time);
    if(index >= sumCreateParticle){
        return;
    }
    uint32_t seed = gRandom.index*index;
    int32_t createParticlenum = -1;
    InterlockedAdd(particleIndexCounter[0], -1, createParticlenum);
    if (createParticlenum > 0){
        int32_t particleIndex = particleIndexCommands.Consume();
        uint32_t startModelIndex=index % startVerticeSize.index;
        uint32_t endModelIndex=index % endVerticeSize.index;
        float32_t3 translate =  lerp((mul(startVertices[startModelIndex].position, startWorldTransform.world).xyz), (mul(endVertices[endModelIndex].position,endWorldTransform.world).xyz),easingTime.time);
        CreateParticle(Output[particleIndex], meshEmitter,translate,seed);
    }
}