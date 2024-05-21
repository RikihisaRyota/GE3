#include "../Fullscreen.hlsli"

Texture2D<float32_t4> tex : register(t0); // 0番スロットに設定されたテクスチャ
Texture2D<float32_t> maskTexture : register(t1); // 0番スロットに設定されたテクスチャ
SamplerState smp : register(s0); // 0番スロットに設定されたサンプラー

struct Desc{
	float32_t threshold;
	float32_t edgeWidth;
};

ConstantBuffer <Desc> descBuffer:register(b0);

struct PixelShaderOutPut
{
    float32_t4 color : SV_TARGET0;
};

PixelShaderOutPut main(VertexShaderOutPut input)
{
	float32_t mask=maskTexture.Sample(smp, input.texcoord);
	if(mask<=descBuffer.threshold){
		discard;
	}
	float32_t edge=1.0f-smoothstep(descBuffer.threshold,descBuffer.threshold + descBuffer.edgeWidth,mask);
	PixelShaderOutPut output;
	output.color.rgb=tex.Sample(smp, input.texcoord).rgb;
	output.color.rgb+=edge;
	output.color.a=1.0f;
	return output;
}