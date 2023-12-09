#include "Game/Resources/Shaders/GPUParticle.hlsli"

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

float4 main(VertexShaderOutput input) : SV_TARGET0
{
    PixelShaderOutput output;
    output.color = float4(1.0f, 1.0f, 1.0f, 1.0f);
    return float4(1.0f, 1.0f, 1.0f, 1.0f);
}