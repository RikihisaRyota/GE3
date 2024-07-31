#include "../Fullscreen.hlsli"

Texture2D<float4> blurTexture0 : register(t0);
Texture2D<float4> blurTexture1 : register(t1);
Texture2D<float4> original : register(t2);
SamplerState smp : register(s0);

struct Desc{
	float32_t intensity;
};

ConstantBuffer<Desc> desc : register(b0);
struct PixelShaderOutPut
{
    float32_t4 color : SV_TARGET0;
};

PixelShaderOutPut main(VertexShaderOutPut input)
{
	PixelShaderOutPut output;
	float32_t4 color=float32_t4(0.0f,0.0f,0.0f,0.0f);
	color += blurTexture0.Sample(smp,input.texcoord);
	color += blurTexture1.Sample(smp,input.texcoord);
	color += original.Sample(smp,input.texcoord);
	color /= 3.0f;
	color.a=1.0f;
	output.color =(color)*desc.intensity;
	return output;
}