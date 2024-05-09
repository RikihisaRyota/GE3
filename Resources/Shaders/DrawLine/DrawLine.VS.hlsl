#include "DrawLine.hlsli"

struct ViewProjection
{
    float32_t4x4 view; // ビュー変換行列
    float32_t4x4 projection; // プロジェクション変換行列
    float32_t3 cameraPos; // カメラのワールド座標
};

ConstantBuffer<ViewProjection> gViewProjection : register(b0);

struct VertexShaderInput
{
    float32_t3 position : POSITION0;
    float32_t4 color : COLOR0;
};

VertexShaderOutput main(VertexShaderInput input)
{
    VertexShaderOutput output;
    output.position = float32_t4(input.position,1.0f);
    output.position = mul(output.position, mul(gViewProjection.view, gViewProjection.projection));
    output.color = input.color;
    return output;
}