#include "GPUParticle.hlsli"

StructuredBuffer<Particle> gParticle : register(t0);

StructuredBuffer<uint> gDrawIndex : register(t1);

struct ViewProjection
{
    float4x4 view; // ビュー変換行列
    float4x4 projection; // プロジェクション変換行列
    float4x4 inverseView; // プロジェクション変換行列
    float3 cameraPos; // カメラのワールド座標
};
ConstantBuffer<ViewProjection> gViewProjection : register(b0);

struct VertexShaderInput
{
    float3 position : POSITION0;
    float2 texcoord : TEXCOORD0;
};

VertexShaderOutput main(VertexShaderInput input, uint instanceID : SV_InstanceID)
{
    uint particleID = gDrawIndex[instanceID];
    VertexShaderOutput output;
    
    // カメラの位置と向きを取得する
    float3 cameraPos = gViewProjection.cameraPos;
    float3 cameraDir = normalize(cameraPos - input.position);

    // カメラの方向を基にビルボードの回転行列を計算する
    float3 upVector = float3(0.0f, 1.0f, 0.0f);
    float3 sideVector = cross(cameraDir, upVector); // カメラの方向と上方向の外積を取る
    float3 newUpVector = cross(sideVector, cameraDir); // カメラの方向と側方向の外積を取る

    float4x4 billboardRotationMatrix =
    {
        float4(sideVector, 0.0f),
        float4(newUpVector, 0.0f),
        float4(-cameraDir, 0.0f), // カメラの方向の反転を使用する
        float4(0.0f, 0.0f, 0.0f, 1.0f)
    };
    
    // パーティクルのスケール、回転、平行移動を適用
    float4x4 mat = mul(billboardRotationMatrix, MakeAffine(gParticle[particleID].scale, gParticle[particleID].rotate, gParticle[particleID].translate));
    float4 worldPos = mul(float4(input.position, 1.0f), mat);
    
    // ワールド座標からスクリーン座標に変換
    output.position = mul(worldPos, mul(gViewProjection.view, gViewProjection.projection));
    
    output.texcoord = input.texcoord;
    
    output.color = gParticle[particleID].color;
    
    output.instanceId = particleID;
    
    return output;
}
