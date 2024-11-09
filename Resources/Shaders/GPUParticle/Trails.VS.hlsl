#include "GPUParticle.hlsli"
#include "GPUParticleShaderStructs.h"

// 入力バッファ

StructuredBuffer<GPUParticleShaderStructs::TrailsData> trailsData : register(t0);
StructuredBuffer<GPUParticleShaderStructs::TrailsPosition> trailsPosition : register(t1);
StructuredBuffer<GPUParticleShaderStructs::TrailsIndex> trailsCounter : register(t2);
StructuredBuffer<GPUParticleShaderStructs::TrailsVertexData> trailsVertexData : register(t3);
StructuredBuffer<uint> drawInstanceCount : register(t4);
struct ViewProjection
{
    float32_t4x4 view; // ビュー変換行列
    float32_t4x4 projection; // プロジェクション変換行列
    float32_t4x4 inverseView;
    float32_t3 cameraPos; // カメラのワールド座標
    float32_t pad;
};
ConstantBuffer<ViewProjection> gViewProjection : register(b0);

VertexShaderOutput main(uint32_t instanceID : SV_InstanceID, uint32_t vertexID : SV_VertexID) {
    VertexShaderOutput output;

    // 頂点の位置を取得
    int32_t index = instanceID;
    int32_t maxInstance=drawInstanceCount[0];
    index = min(index,maxInstance);
    float32_t3 position = trailsVertexData[index].vertex[vertexID].position;
    float32_t4 color = float32_t4(1.0f, 1.0f, 1.0f, 1.0f);
    //if(index >= maxInstance){
    //    position = trailsVertex[index-1].vertex[vertexID].position;
    //    color = float32_t4(0.0f, 0.0f, 0.0f, 0.0f);
    //}
    // ビュー・プロジェクション行列で座標変換
    float32_t4 worldPosition = float32_t4(position, 1.0f);
    output.position = mul(worldPosition, mul(gViewProjection.view, gViewProjection.projection));
    output.color = color;

    return output;
}
