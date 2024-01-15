#include "Model.hlsli"

struct WorldTransform
{
    matrix world; // ���[���h�ϊ��s��
};

ConstantBuffer<WorldTransform> gWorldTransform : register(b0);

struct ViewProjection
{
    matrix view; // �r���[�ϊ��s��
    matrix projection; // �v���W�F�N�V�����ϊ��s��
    float3 cameraPos; // �J�����̃��[���h���W
};

ConstantBuffer<ViewProjection> gViewProjection : register(b1);

struct VertexShaderInput
{
    float3 position : POSITION0;
    float3 normal : NORMAL0;
    float2 texcoord : TEXCOORD0;
};

VertexShaderOutput main(VertexShaderInput input)
{
    VertexShaderOutput output; // �s�N�Z���V�F�[�_�[�ɓn���l
    output.position = float4(input.position,1.0f);
    output.position = mul(mul(output.position, gWorldTransform.world),mul(gViewProjection.view, gViewProjection.projection));
    output.normal = normalize(mul(input.normal, (float3x3) gWorldTransform.world));
    output.texcoord = input.texcoord;
    return output;
}