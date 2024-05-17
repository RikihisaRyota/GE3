#include "../Fullscreen.hlsli"

Texture2D<float32_t4> tex : register(t0); // 0番スロットに設定されたテクスチャ
SamplerState smp : register(s0); // 0番スロットに設定されたサンプラー

struct PixelShaderOutPut
{
    float32_t4 color : SV_TARGET0;
};

static const uint32_t kTexelSize = 5;
static const uint32_t kTexelCenter = kTexelSize / 2;
static const float32_t kTexelAverage = pow(float32_t(kTexelSize), 2);
static const float32_t PI=3.14159265f;

float Gauss(float32_t x,float32_t y,float32_t sigma){
    float32_t exponent=-(x*x+y*y)*rcp(2.0f*sigma*sigma);
    float32_t denominator=2.0f*PI*sigma*sigma;
    return exp(exponent)*rcp(denominator);
}

float32_t3 Smoothing(float32_t2 originalTexcoord, Texture2D<float32_t4> tex, SamplerState smp){
    uint32_t width, height;
    tex.GetDimensions(width, height);
    float32_t2 uvStepSize = float32_t2(rcp(width), rcp(height));
    float32_t3 result = {0.0f, 0.0f, 0.0f};

    float32_t kernel = (1.0f / kTexelAverage);
    float32_t wight = 0.0f;
    for(uint32_t x = 0; x < kTexelSize; ++x){
        for(uint32_t y = 0; y < kTexelSize; ++y){
            wight+= Gauss(float32_t(int32_t(x) - int32_t(kTexelCenter)), float32_t(int32_t(y) - int32_t(kTexelCenter)),1.2f);
            float32_t2 texcoord = originalTexcoord + float32_t2(int32_t(x) - int32_t(kTexelCenter), int32_t(y) - int32_t(kTexelCenter)) * uvStepSize;
            if(texcoord.x < 0.0f || texcoord.y < 0.0f || texcoord.x > 1.0f || texcoord.y > 1.0f){
                continue;
            }
            float32_t3 fetchColor = tex.Sample(smp, texcoord).rgb;
            result += fetchColor * kernel;
        }
    }
    return result*rcp(wight);
}


PixelShaderOutPut main(VertexShaderOutPut input)
{
	PixelShaderOutPut output;
	output.color.rgb = Smoothing(input.texcoord,tex,smp);
	output.color.a=1.0f;
	return output;
}