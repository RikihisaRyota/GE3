#include "GPUParticle.hlsli"

// 出力パーティクルバッファ
RWStructuredBuffer<Particle> Output : register(u0);

// パーティクルインデックスコマンド
ConsumeStructuredBuffer<uint> particleIndexCommands : register(u1);

// カウンタ構造体
struct Count {
    int32_t count;
};

// パーティクルインデックスカウンタ
RWStructuredBuffer<Count> particleIndexCounter : register(u2);

// 頂点構造体
struct Vertex {
    float32_t4 position;
    float32_t3 normal;
    float32_t2 texcoord;
};

// 頂点バッファ
StructuredBuffer<Vertex> vertices : register(t0);

// インデックス構造体
struct Index {
    uint32_t index;
};

// インデックスバッファ
StructuredBuffer<Index> indices : register(t1);

// ランダム値
struct Random {
    uint32_t random;
};

// ランダム値バッファ
ConstantBuffer<Random> gRandom : register(b0);

// ワールド変換
struct WorldTransform {
    float32_t4x4 world;
    float32_t4x4 inverseMatWorld;
};

// ワールド変換バッファ
ConstantBuffer<WorldTransform> worldTransform : register(b1);

// インデックスカウントバッファ
ConstantBuffer<Count> indexCount : register(b2);

// メッシュエミッタバッファ
ConstantBuffer<MeshEmitter> meshEmitter : register(b3);

[numthreads(meshThreadBlockSize, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID) {
    uint triIndex = DTid.x;
    if (triIndex >= indexCount.count / 3) {
        return;
    }

    uint32_t seed = gRandom.random * triIndex;

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

    // 平均スケールを計算
    float32_t averageScale = (((meshEmitter.scale.range.start.min.x + meshEmitter.scale.range.start.min.y + meshEmitter.scale.range.start.min.z) / 3.0f) + 
                             ((meshEmitter.scale.range.start.max.x + meshEmitter.scale.range.start.max.y + meshEmitter.scale.range.start.max.z) / 3.0f)) * 0.5f;

    // 各エッジに沿ってパーティクルを生成
    for (uint32_t i = 0; i < 3; i++) {
        uint32_t sumCreateParticleNum = 0;
        float32_t3 start, end;

        if (i == 0) {
            start = v0;
            end = v1;
            sumCreateParticleNum = uint32_t(length(v0 - v1) / averageScale);
        } else if (i == 1) {
            start = v1;
            end = v2;
            sumCreateParticleNum = uint32_t(length(v1 - v2) / averageScale);
        } else {
            start = v2;
            end = v0;
            sumCreateParticleNum = uint32_t(length(v2 - v0) / averageScale);
        }

        for (uint32_t j = 0; j < sumCreateParticleNum; j++) {
            int32_t createParticleNum = -1;
            InterlockedAdd(particleIndexCounter[0].count, -1, createParticleNum);

            if (createParticleNum > 0) {
                int32_t index = particleIndexCommands.Consume();
                float32_t t = float32_t(j) / float32_t(sumCreateParticleNum);
                float32_t3 translate = lerp(start, end, t);
                //CreateParticle(Output[index], meshEmitter,translate,seed);
            }
        }
    }
}
