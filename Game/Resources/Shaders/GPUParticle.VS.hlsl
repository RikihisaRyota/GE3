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
    float2 texcoord : TEXCOORD0;
};

VertexShaderOutput main(VertexShaderInput input)
{
    VertexShaderOutput output;
//    float3 toCamera = gViewProjection.cameraPos - gParticle.translate;
//    float3 up = float3(0.0f, 1.0f, 0.0f);

//    float3 zAxis = normalize(toCamera);
//    float3 xAxis = normalize(cross(up, zAxis));
//    float3 yAxis = cross(zAxis, xAxis);
    
//    float4x4 billboardMatrix = float4x4(
//    float4(xAxis, 0.0f),
//    float4(yAxis, 0.0f),
//    float4(zAxis, 0.0f),
//    float4(
//    -dot(xAxis, normalize(gViewProjection.cameraPos)),
//    -dot(yAxis, normalize(gViewProjection.cameraPos)), 
//    -dot(zAxis, normalize(gViewProjection.cameraPos)), 1.0f)
//);
//    billboardMatrix = Inverse(billboardMatrix);
//    billboardMatrix[3] = float4(0.0f, 0.0f, 0.0f, 1.0f);
    float4x4 mat = MakeAffine(gParticle.scale, gParticle.rotate, gParticle.translate);
    //float4x4 mat = mul(billboardMatrix,MakeAffine(gParticle.scale, gParticle.rotate, gParticle.translate));
    float4 worldPos = mul(float4(input.position, 1.0f), mat);
    output.position = mul(worldPos, mul(gViewProjection.view, gViewProjection.projection));
    output.texcoord = input.texcoord;
    return output;
}