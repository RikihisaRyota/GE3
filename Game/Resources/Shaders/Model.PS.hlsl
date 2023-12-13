#include "Model.hlsli"

Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

struct ViewProjection
{
    matrix view; // �r���[�ϊ��s��
    matrix projection; // �v���W�F�N�V�����ϊ��s��
    float3 cameraPos; // �J�����̃��[���h���W
};

ConstantBuffer<ViewProjection> gViewProjection : register(b1);

//struct Material
//{
//    float4 color;
//};

//ConstantBuffer<Material> gMaterial : register(b2);

//struct DirectionLight
//{
//    float4 color;
//    float3 direction;
//    float intensity;
//    float sharpness;
//};

//ConstantBuffer<DirectionLight> gDirectionLight : register(b3);

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    output.color = float4(1.0f,1.0f,0.0f,1.0f);
    return output;
}
