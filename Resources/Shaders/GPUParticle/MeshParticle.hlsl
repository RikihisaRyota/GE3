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

ConstantBuffer<Count> indexCount : register(b2);

ConstantBuffer<MeshEmitter> meshEmitter : register(b3);

[numthreads(meshThreadBlockSize, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint triIndex = DTid.x;
    if (triIndex >= indexCount.count)
    {
        return;
    }

    uint32_t seed = gRandom.random*triIndex;
    int32_t createParticlenum = -1;
    InterlockedAdd(particleIndexCounter[0].count, -1, createParticlenum);
    if (createParticlenum > 0)
    {
        int32_t index = particleIndexCommands.Consume();
        Output[index].particleLifeTime.maxTime =randomRange(meshEmitter.particleLifeSpan.range.min, meshEmitter.particleLifeSpan.range.max, seed);
        Output[index].particleLifeTime.time = 0;

        Output[index].scaleRange.min.x = randomRange(meshEmitter.scale.range.start.min.x, meshEmitter.scale.range.start.max.x,seed);
        Output[index].scaleRange.min.y = randomRange(meshEmitter.scale.range.start.min.y, meshEmitter.scale.range.start.max.y,seed);
        Output[index].scaleRange.min.z = randomRange(meshEmitter.scale.range.start.min.z, meshEmitter.scale.range.start.max.z,seed);
    
        Output[index].scaleRange.max.x = randomRange(meshEmitter.scale.range.end.min.x, meshEmitter.scale.range.end.max.x,seed);
        Output[index].scaleRange.max.y = randomRange(meshEmitter.scale.range.end.min.y, meshEmitter.scale.range.end.max.y,seed);
        Output[index].scaleRange.max.z = randomRange(meshEmitter.scale.range.end.min.z, meshEmitter.scale.range.end.max.z,seed);
    
        Output[index].scale = Output[index].scaleRange.min;

        Output[index].rotateVelocity = meshEmitter.rotateAnimation.rotate;
        Output[index].rotate = 0.0f;
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
        Output[index].translate = p;

         Output[index].velocity = randomRange(meshEmitter.velocity3D.range.min, meshEmitter.velocity3D.range.max, seed);
        
        // 各頂点の法線を使用してカラーを設定
        Output[index].colorRange.min=randomRange(meshEmitter.color.range.start.min, meshEmitter.color.range.start.max, seed);
        Output[index].colorRange.max=randomRange(meshEmitter.color.range.end.min, meshEmitter.color.range.end.max, seed);
        Output[index].color = Output[index].colorRange.min;
        
        Output[index].textureIndex = meshEmitter.textureIndex;;

        Output[index].isAlive = 1;
    }
}