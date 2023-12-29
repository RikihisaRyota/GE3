#include "GPUParticle.hlsli"

ConstantBuffer<Particle> gParticle : register(b0);

struct ViewProjection
{
    matrix view; // ビュー変換行列
    matrix projection; // プロジェクション変換行列
    float3 cameraPos; // カメラのワールド座標
};
ConstantBuffer<ViewProjection> gViewProjection : register(b1);

struct VertexShaderInput
{
    float3 position : POSITION0;
};

VertexShaderOutput main(VertexShaderInput input)
{
    VertexShaderOutput output;
    output.position = mul(mul(float4(input.position,1.0f), MakeAffine(gParticle.scale, gParticle.rotate, gParticle.translate)),
    mul(gViewProjection.view, gViewProjection.projection));
    //float4(gParticle[instanceID].position,0.0f);
    return output;
}