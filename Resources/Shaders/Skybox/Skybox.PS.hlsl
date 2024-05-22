#include "Skybox.hlsli"

TextureCube<float32_t4> gTexture:register(t0);
SamplerState gSampler : register(s0);

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};


PixelShaderOutput main(VertexShaderOutput input)
{
	PixelShaderOutput output;
	float32_t4 textureColor=gTexture.Sample(gSampler,input.texcoord);
	output.color=textureColor;
	return output;
}