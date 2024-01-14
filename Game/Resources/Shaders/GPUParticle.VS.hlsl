#include "GPUParticle.hlsli"

StructuredBuffer<Particle> gParticle : register(t0);

struct ViewProjection
{
    matrix view; // ビュー変換行列
    matrix projection; // プロジェクション変換行列
    float3 cameraPos; // カメラのワールド座標
};
ConstantBuffer<ViewProjection> gViewProjection : register(b0);

struct VertexShaderInput
{
    float3 position : POSITION0;
    float2 texcoord : TEXCOORD0;
};

VertexShaderOutput main(VertexShaderInput input,uint instanceID : SV_InstanceID)
{
    VertexShaderOutput output;
    float4x4 mat = MakeAffine(gParticle[instanceID].scale, gParticle[instanceID].rotate, gParticle[instanceID].translate);
    float4 worldPos = mul(float4(input.position, 1.0f), mat);
    output.position = mul(worldPos, mul(gViewProjection.view, gViewProjection.projection));
    output.texcoord = input.texcoord;
    return output;
}