#include "../Fullscreen.hlsli"


Texture2D<float32_t4> tex : register(t0); // 0番スロットに設定されたテクスチャ
struct Desc{
	float32_t threshold;
};

ConstantBuffer <Desc> descBuffer:register(b0);
SamplerState smp : register(s0); // 0番スロットに設定されたサンプラー

struct PixelShaderOutPut
{
    float32_t4 color : SV_TARGET0;
};

float3 ExtractBrightParts(float3 color, float threshold) {
     // 閾値を超えた輝度成分だけを残す
	return max(color - threshold, 0.0f);
}
PixelShaderOutPut main(VertexShaderOutPut input)
{

	PixelShaderOutPut output;
	float32_t3 outputColor = tex.Sample(smp, input.texcoord).rgb;
	outputColor = ExtractBrightParts(outputColor,descBuffer.threshold);
	output.color.rgb = outputColor;
	output.color.a=1.0f;
	return output;
}