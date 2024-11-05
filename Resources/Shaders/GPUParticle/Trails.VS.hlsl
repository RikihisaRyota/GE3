#include "GPUParticleShaderStructs.h"

// 入力バッファ
StructuredBuffer<GPUParticleShaderStructs::TrailsData> trailsData : register(t0);
StructuredBuffer<GPUParticleShaderStructs::TrailsPosition> trailsPosition : register(t1);
struct ViewProjection
{
    float32_t4x4 view; // ビュー変換行列
    float32_t4x4 projection; // プロジェクション変換行列
    float32_t4x4 inverseView;
    float32_t3 cameraPos; // カメラのワールド座標
    float32_t pad;
};
ConstantBuffer<ViewProjection> gViewProjection : register(b0);

struct VertexShaderInput
{
    float32_t3 position : POSITION0;
    float32_t2 texcoord : TEXCOORD0;
};


// 頂点シェーダー本体
VSOutput main(VertexShaderInput input) {
    VSOutput output;

    // 軌跡データの取得
    TrailsData trail = trailsData[input.trailIndex];
    uint startIdx = trail.startIndex;
    uint endIdx = trail.endIndex;
    uint pointCount = endIdx - startIdx;

    // 軌跡全体に対する頂点を計算
    for (uint i = 0; i < pointCount; i++) {
        uint idx = startIdx + i;

        float3 position = trailsPosition[idx].position;
        float3 nextPosition = (i < pointCount - 1) ? trailsPosition[idx + 1].position : position;
        float3 segmentDir = normalize(nextPosition - position);

        // カメラ方向に基づいて板ポリの幅を調整
        float3 toCamera = normalize(cameraPosition - position);
        float3 right = normalize(cross(segmentDir, toCamera)) * trail.width * 0.5;

        // 左右の頂点の位置とUV座標
        float vCoord = (float)i / (pointCount - 1); // V座標の設定
        output.position = mul(viewProjectionMatrix, float4(position - right, 1.0));
        output.uv = float2(0.0, vCoord);

        output.position = mul(viewProjectionMatrix, float4(position + right, 1.0));
        output.uv = float2(1.0, vCoord);
    }
    return output;
}