#include "GPUParticle.hlsli"
#include "GPUParticleShaderStructs.h"

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

struct Index
{
    uint32_t index;
};

StructuredBuffer<Index> indices : register(t1);

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

ConstantBuffer<Count> indexCount : register(b2);

ConstantBuffer<MeshEmitter> meshEmitter : register(b3);

[numthreads(GPUParticleShaderStructs::MeshComputeThreadBlockSize, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint triIndex = DTid.x;
    if (triIndex > indexCount.count / 3)
    {
        return;
    }

    uint32_t seed = gRandom.random*triIndex;
    int32_t createParticlenum = -1;
    InterlockedAdd(particleIndexCounter[0].count, -1, createParticlenum);
    if (createParticlenum > 0)
    {
        int32_t index = particleIndexCommands.Consume();
        
        // 三角形の頂点インデックスを取得
        uint32_t3 triIndices = uint32_t3(
            indices[triIndex * 3].index,
            indices[triIndex * 3 + 1].index,
            indices[triIndex * 3 + 2].index
        );

        // 三角形の頂点座標を取得
        float32_t3 v0 = mul(vertices[triIndices.x].position, worldTransform.world).xyz;
        float32_t3 v1 = mul(vertices[triIndices.y].position, worldTransform.world).xyz;
        float32_t3 v2 = mul(vertices[triIndices.z].position, worldTransform.world).xyz;

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
        //CreateParticle(Output[index], meshEmitter,p,seed);
    }
}