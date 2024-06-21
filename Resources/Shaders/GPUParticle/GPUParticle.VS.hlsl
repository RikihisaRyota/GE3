#include "GPUParticle.hlsli"

StructuredBuffer<Particle> gParticle : register(t0);

StructuredBuffer<uint> gDrawIndex : register(t1);

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

VertexShaderOutput main(VertexShaderInput input, uint32_t instanceID : SV_InstanceID)
{
    uint32_t particleID = gDrawIndex[instanceID];
    VertexShaderOutput output;    
    
    // ワールド座標からスクリーン座標に変換
    output.position = mul(mul(float32_t4(input.position, 1.0f),gParticle[particleID].worldMatrix), mul(gViewProjection.view, gViewProjection.projection));
    
    output.texcoord = input.texcoord;
    
    output.color = gParticle[particleID].color;
    
    output.instanceId = particleID;
    
    return output;
}
