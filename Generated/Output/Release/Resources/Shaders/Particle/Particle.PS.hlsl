#include "Particle.hlsli"

Texture2D<float4> gTexture : register(t1);
SamplerState gSampler : register(s0);

struct ViewProjection
{
    float4x4 view;
    float4x4 projection;
    float4x4 inverseView;
    float3 cameraPos;
};

ConstantBuffer<ViewProjection> gViewProjection : register(b0);

struct Material
{
    float4 color;
};

ConstantBuffer<Material> gMaterial : register(b1);

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    float4 textureColor = gTexture.Sample(gSampler, input.texcoord);
    if (textureColor.a == 0.0f)
    {
        discard;
    }

    output.color = gMaterial.color * textureColor * input.color;
    
    return output;
}
