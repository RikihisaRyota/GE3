#include "GPUParticle.hlsli"

StructuredBuffer<Particle> gParticle : register(t0);

struct ViewProjection
{
    matrix view; // �r���[�ϊ��s��
    matrix projection; // �v���W�F�N�V�����ϊ��s��
    float3 cameraPos; // �J�����̃��[���h���W
};
ConstantBuffer<ViewProjection> gViewProjection : register(b0);

struct VertexShaderInput
{
    float3 position : POSITION0;
};

VertexShaderOutput main(VertexShaderInput input, uint instanceID : SV_InstanceID)
{
    VertexShaderOutput output;
    output.position = mul(mul(float4(input.position,1.0f), MakeAffine(gParticle[instanceID].scale, gParticle[instanceID].rotate, gParticle[instanceID].translate)),
    mul(gViewProjection.view, gViewProjection.projection));
    //float4(gParticle[instanceID].position,0.0f);
    return output;
}