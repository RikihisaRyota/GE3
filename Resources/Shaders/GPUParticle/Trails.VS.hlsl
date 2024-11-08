#include "GPUParticle.hlsli"
#include "GPUParticleShaderStructs.h"

// 入力バッファ

StructuredBuffer<GPUParticleShaderStructs::TrailsData> trailsData : register(t0);
StructuredBuffer<GPUParticleShaderStructs::TrailsPosition> trailsPosition : register(t1);
StructuredBuffer<GPUParticleShaderStructs::TrailsIndex> trailsCounter : register(t2);
struct ViewProjection
{
    float32_t4x4 view; // ビュー変換行列
    float32_t4x4 projection; // プロジェクション変換行列
    float32_t4x4 inverseView;
    float32_t3 cameraPos; // カメラのワールド座標
    float32_t pad;
};
ConstantBuffer<ViewProjection> gViewProjection : register(b0);

VertexShaderOutput main(GPUParticleShaderStructs::TrailsInputVertex input, uint32_t instanceID : SV_InstanceID, uint32_t vertexID : SV_VertexID){
    VertexShaderOutput output;

//    // 各インスタンスに3つの頂点があると仮定
//    int baseIndex = trailsCounter[instanceID].positionIndex;
//
//    // 三角形の頂点オフセットを計算
//    int vertexOffset = baseIndex + vertexID;
//
//    // 現在の最大のインデックスを取得
//    int32_t maxPositionIndex = trailsData[trailsCounter[instanceID].trailsIndex].currentIndex;
//
//    // 範囲外のインデックスを防ぐ
//    int clampedOffset = min(vertexOffset, maxPositionIndex - 1);
//
//    // 頂点の位置を取得
//    float32_t3 position = trailsPosition[clampedOffset].position;
//    float32_t4 color = float32_t4(1.0f, 1.0f, 1.0f, 1.0f);
//
//    // 範囲外の場合、色を黒に設定
//    if (vertexOffset >= maxPositionIndex) {
//        color = float32_t4(0.0f, 0.0f, 0.0f, 0.0f);
//    }

    // ビュー・プロジェクション行列で座標変換
    output.position = mul(float32_t4(input[instanceID].position, 1.0f), mul(gViewProjection.view, gViewProjection.projection));
    output.color = float32_t4(1.0f,1.0f,1.0f,1.0f);
    return output;
}
