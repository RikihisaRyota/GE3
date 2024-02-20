#include "Particle.hlsli"

struct ParticleForGPU
{
    float4x4 world;
    float4 color;
};

StructuredBuffer<ParticleForGPU> gParticleForGPU : register(t0);

struct ViewProjection
{
    float4x4 view;
    float4x4 projection;
    float4x4 inverseView;
    float3 cameraPos;
};

ConstantBuffer<ViewProjection> gViewProjection : register(b0);

struct VertexShaderInput
{
    float3 position : POSITION0;
    float2 texcoord : TEXCOORD0;
};

VertexShaderOutput main(VertexShaderInput input, uint instanceID : SV_InstanceID)
{
    VertexShaderOutput output; // ピクセルシェーダーに渡す値
    float4x4 mat = mul(gViewProjection.inverseView, gParticleForGPU[instanceID].world);
    float4 worldPos = mul(float4(input.position, 1.0f), mat);
    output.position = mul(worldPos, mul(gViewProjection.view, gViewProjection.projection));
    output.texcoord = input.texcoord;
    output.color = gParticleForGPU[instanceID].color;
    return output;
}