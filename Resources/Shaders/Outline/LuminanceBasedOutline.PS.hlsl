#include "../Fullscreen.hlsli"
#include "LuminanceBasedOutline.hlsli"

struct CameraData
{
    float4x4 inverseCamera;
};

ConstantBuffer<CameraData> gCamera : register(b0);
Texture2D<float32_t4> tex : register(t0); // 0番スロットに設定されたテクスチャ
Texture2D<float32_t> gDepthTexture  : register(t1); // 1番スロットに設定されたテクスチャ
SamplerState smp : register(s0); // 0番スロットに設定されたサンプラー
SamplerState smpPoint : register(s1); // 0番スロットに設定されたサンプラー

struct PixelShaderOutPut
{
    float32_t4 color : SV_TARGET0;
};

PixelShaderOutPut main(VertexShaderOutPut input)
{
	float32_t2 difference = PrewittFilter(input.texcoord, gCamera.inverseCamera,gDepthTexture,smpPoint);
	float32_t weight=length(difference);
	weight=saturate(weight);
	PixelShaderOutPut output;
	output.color.rgb=(1.0f-weight)*tex.Sample(smp, input.texcoord).rgb;
	output.color.a=1.0f;
	return output;
}