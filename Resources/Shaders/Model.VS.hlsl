#include "Model.hlsli"

ConstantBuffer<WorldTransform> gWorldTransform : register(b0);

ConstantBuffer<ViewProjection> gViewProjection : register(b1);

struct VertexShaderInput
{
    float32_t4 position : POSITION0;
    float32_t3 normal : NORMAL0;
    float32_t2 texcoord : TEXCOORD0;
};

VertexShaderOutput main(VertexShaderInput input)
{
    VertexShaderOutput output; // ピクセルシェーダーに渡す値
    output.position =input.position;
    output.position = mul(mul(output.position, gWorldTransform.world),mul(gViewProjection.view, gViewProjection.projection));
    output.normal = normalize(mul(input.normal, (float32_t3x3) gWorldTransform.inverseMatWorld));
    output.texcoord = input.texcoord;
    return output;
}