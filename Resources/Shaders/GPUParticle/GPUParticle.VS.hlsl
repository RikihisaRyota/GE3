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
    
    // カメラの位置と向きを取得する
    float32_t3 cameraPos = gViewProjection.cameraPos;
    float32_t3 cameraDir = normalize(gParticle[particleID].translate-cameraPos);

    // カメラの方向を基にビルボードの回転行列を計算する
    float32_t3 upVector = float32_t3(0.0f, 1.0f, 0.0f);
    float32_t3 sideVector = cross(upVector,cameraDir); // カメラの方向と上方向の外積を取る
    float32_t3 newUpVector = cross(cameraDir,sideVector); // カメラの方向と側方向の外積を取る

    float32_t4x4 billboardRotationMatrix =
    {
        float32_t4(sideVector, 0.0f),
        float32_t4(newUpVector, 0.0f),
        float32_t4(cameraDir, 0.0f),
        float32_t4(0.0f, 0.0f, 0.0f, 1.0f)
    };
    
    // パーティクルのスケール、回転、平行移動を適用

    float32_t4x4 mat =mul(mul(MakeScaleMatrix(gParticle[particleID].scale),billboardRotationMatrix),MakeTranslationMatrix(gParticle[particleID].translate));
    float32_t4 worldPos = mul(float32_t4(input.position, 1.0f), mat);
    
    // ワールド座標からスクリーン座標に変換
    output.position = mul(worldPos, mul(gViewProjection.view, gViewProjection.projection));
    
    output.texcoord = input.texcoord;
    
    output.color = gParticle[particleID].color;
    
    output.instanceId = particleID;
    
    return output;
}
