#include "../Fullscreen.hlsli"

Texture2D<float32_t4> tex : register(t0); // 0番スロットに設定されたテクスチャ
SamplerState smp : register(s0); // 0番スロットに設定されたサンプラー

struct Desc{
	float32_t hue;
    float32_t saturation;
    float32_t value;
};

ConstantBuffer<Desc> desc : register(b0);

struct PixelShaderOutPut
{
    float32_t4 color : SV_TARGET0;
};

float32_t Random(float32_t2 coord)
{
    return frac(sin(dot(coord, float32_t2(8.7819, 3.255))) * 437.645);
};

float32_t3 HSVToRGB(float32_t3 hsv)
{
    float32_t4 k = float32_t4(1.0f, 2.0f / 3.0f, 1.0f / 3.0f, 3.0f);
    float32_t3 p = abs(frac(float32_t3(hsv.x, hsv.x, hsv.x) + k.xyz) * 6.0f - k.www);
    return hsv.z * lerp(k.xxx, clamp(p - k.xxx, 0.0f, 1.0f), hsv.y);
}

float32_t3 RGBToHSV(float32_t3 rgb)
{
    float32_t4 K = float32_t4(0.0f, -1.0f / 3.0f, 2.0f / 3.0f, -1.0f);
    float32_t4 p = (rgb.g < rgb.b) ? float32_t4(rgb.bg, K.wz) : float32_t4(rgb.gb, K.xy);
    float32_t4 q = (rgb.r < p.x) ? float32_t4(p.xyw, rgb.r) : float32_t4(rgb.r, p.yzx);
    
    float32_t d = q.x - min(q.w, q.y);
    float32_t e = 1.0e-10f;
    float32_t3 hsv;
    hsv.x = abs(q.z + (q.w - q.y) / (6.0f * d + e));
    hsv.y = d / (q.x + e);
    hsv.z = q.x;
    return hsv;
}


float32_t WrapValue(float32_t value,float32_t minRange,float32_t maxRange){
	float32_t range=maxRange-minRange;
	float32_t modValue=fmod(value-minRange,range);
	if(modValue<0.0f){
		modValue+=range;
	}
	return minRange+modValue;
}


PixelShaderOutPut main(VertexShaderOutPut input)
{
    PixelShaderOutPut output;
    float32_t2 samplePoint = input.texcoord;
    float32_t4 textureColor = tex.Sample(smp, input.texcoord);
    float32_t3 hsv = RGBToHSV(textureColor.rgb);
    hsv.x+=desc.hue;
    hsv.y+=desc.saturation;
    hsv.z+=desc.value;
    hsv.x = WrapValue(hsv.x, 0.0f, 1.0f);
    hsv.y = saturate(hsv.y);
    hsv.z = saturate(hsv.z);
    textureColor.rgb = HSVToRGB(hsv);
    output.color = textureColor;
    return output;
}
