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
    if (triIndex >= vertexCount.count)
    {
        return;
    }

    uint32_t seed = gRandom.random*triIndex;
    int32_t createParticlenum = -1;
    InterlockedAdd(particleIndexCounter[0].count, -1, createParticlenum);
    if (createParticlenum > 0)
    {
        int32_t index = particleIndexCommands.Consume();
        Output[index].collisionInfo = meshEmitter.collisionInfo;
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

        Output[index].translate =  mul(vertices[triIndex].position, worldTransform.world).xyz;

        Output[index].velocity = randomRange(meshEmitter.velocity3D.range.min, meshEmitter.velocity3D.range.max, seed);
        
        // 各頂点の法線を使用してカラーを設定
        Output[index].colorRange.min=randomRange(meshEmitter.color.range.start.min, meshEmitter.color.range.start.max, seed);
        Output[index].colorRange.max=randomRange(meshEmitter.color.range.end.min, meshEmitter.color.range.end.max, seed);
        Output[index].color = Output[index].colorRange.min;
        
        Output[index].textureIndex = meshEmitter.textureIndex;;

        Output[index].isAlive = 1;
        Output[index].isHit = 0;
    }
}