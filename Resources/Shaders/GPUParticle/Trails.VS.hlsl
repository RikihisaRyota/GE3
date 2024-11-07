#include "GPUParticle.hlsli"
#include "GPUParticleShaderStructs.h"

// 入力バッファ

StructuredBuffer<GPUParticleShaderStructs::TrailsData> trailsData : register(t0);
StructuredBuffer<GPUParticleShaderStructs::TrailsPosition> trailsPosition : register(t1);
StructuredBuffer<int> trailsCounter : register(t2);
struct ViewProjection
{
    float32_t4x4 view; // ビュー変換行列
    float32_t4x4 projection; // プロジェクション変換行列
    float32_t4x4 inverseView;
    float32_t3 cameraPos; // カメラのワールド座標
    float32_t pad;
};
ConstantBuffer<ViewProjection> gViewProjection : register(b0);

// 頂点シェーダー本体
struct VertexShaderInput
{
    float32_t3 position : POSITION0;
    float32_t2 texcoord : TEXCOORD0;
};

VertexShaderOutput main(uint32_t instanceID : SV_InstanceID) {
    VertexShaderOutput output;
    float32_t3 position = trailsPosition[trailsCounter[instanceID]].position;
    output.position = mul(float32_t4(position,1.0f), mul(gViewProjection.view, gViewProjection.projection));
    return output;
}