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

ConstantBuffer<Count> vertexCount : register(b2);

[numthreads(meshThreadBlockSize, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint triIndex = DTid.x;
    if (triIndex >= vertexCount.count)
    {
        return;
    }

    uint32_t seed = gRandom.random;
for(uint32_t i=0;i<5;i++){

    int32_t createParticlenum = -1;
    InterlockedAdd(particleIndexCounter[0].count, -1, createParticlenum);
    if (createParticlenum > 0)
    {
        int32_t index = particleIndexCommands.Consume();
        Output[index].particleLifeTime.maxTime = 1;
        Output[index].particleLifeTime.time = 0;

        Output[index].scaleRange.min = float32_t3(0.1f, 0.1f, 0.1f);
        Output[index].scaleRange.max = float32_t3(0.05f, 0.05f, 0.05f);
        Output[index].scale = Output[index].scaleRange.min;

        Output[index].rotateVelocity = 0.0f;

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
        if (u + v > 1.0f) {
            u = 1.0f - u;
            v = 1.0f - v;
        }

        // ランダムな点を計算
        float32_t3 p = v0 + u * (v1 - v0) + v * (v2 - v0);
        Output[index].translate = p;

        Output[index].velocity = float32_t3(0.0f, 0.0f, 0.0f);
        
        // 各頂点の法線を使用してカラーを設定
        Output[index].colorRange.min = float32_t4(vertices[triIndices.x].normal, 1.0f);
        Output[index].colorRange.max = float32_t4(vertices[triIndices.x].normal, 1.0f);
        Output[index].color = float32_t4(vertices[triIndices.x].normal, 1.0f);
        Output[index].textureIndex = 0;

        Output[index].isAlive = 1;
    }
}
}