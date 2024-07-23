#include "GPUParticle.hlsli"

RWStructuredBuffer<Particle> Output : register(u0);

ConsumeStructuredBuffer<uint> particleIndexCommands : register(u1);

struct Count
{
    int32_t count;
};

RWStructuredBuffer<Count> particleIndexCounter : register(u2);

struct Vertex
{
    float32_t4 position;
    float32_t3 normal;
    float32_t2 texcoord;
};

StructuredBuffer<Vertex> vertices : register(t0);

struct Random
{
    uint32_t random;
};

ConstantBuffer<Random> gRandom : register(b0);

struct WorldTransform
{
    float32_t4x4 world;
    float32_t4x4 inverseMatWorld;
};

ConstantBuffer<WorldTransform> worldTransform : register(b1);

ConstantBuffer<Count> vertexCount : register(b2);

ConstantBuffer<MeshEmitter> meshEmitter : register(b3);

[numthreads(meshThreadBlockSize, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint triIndex = DTid.x;
    if (triIndex > vertexCount.count)
    {
        return;
    }

    uint32_t seed = gRandom.random*triIndex;
    int32_t createParticlenum = -1;
    InterlockedAdd(particleIndexCounter[0].count, -1, createParticlenum);
    if (createParticlenum > 0)
    {
        int32_t index = particleIndexCommands.Consume();
        float32_t3 translate=  mul(vertices[triIndex].position, worldTransform.world).xyz;
        //CreateParticle(Output[index], meshEmitter,translate,seed);
    }
}