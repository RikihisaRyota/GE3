#include "../Fullscreen.hlsli"

Texture2D<float32_t4> tex : register(t0); // 0番スロットに設定されたテクスチャ
SamplerState smp : register(s0); // 0番スロットに設定されたサンプラー

struct Desc{
	float32_t2 center;
	float32_t blurWidth;
	float32_t pad;
};

ConstantBuffer <Desc> descBuffer:register(b0);

struct PixelShaderOutPut
{
    float32_t4 color : SV_TARGET0;
};

PixelShaderOutPut main(VertexShaderOutPut input)
{
	const int32_t kNumSamples=10;

	float32_t2 direction=input.texcoord-descBuffer.center;
	float32_t3 outputColor=float32_t3(0.0f,0.0f,0.0f);
	for(int32_t sampleIndex=0;sampleIndex<kNumSamples;++sampleIndex){
		float32_t2 texcoord=input.texcoord-direction*descBuffer.blurWidth*float32_t(sampleIndex);
		outputColor+= tex.Sample(smp, texcoord).rgb;
	}
	outputColor*=rcp(kNumSamples);
	PixelShaderOutPut output;
	output.color.rgb=outputColor;
	output.color.a=1.0f;
	return output;
}