#include "../Fullscreen.hlsli"

#define NUM_TEXTURES 2

Texture2D<float4> blurTexture0 : register(t0);
Texture2D<float4> blurTexture1 : register(t1);
//Texture2D<float4> blurTexture2 : register(t2);
//Texture2D<float4> blurTexture3 : register(t3);
//Texture2D<float4> blurTexture4 : register(t4);
SamplerState smp : register(s0);

struct PixelShaderOutPut
{
    float32_t4 color : SV_TARGET0;
};

PixelShaderOutPut main(VertexShaderOutPut input)
{
	PixelShaderOutPut output;

    float32_t4 bloom = float4(0.0f, 0.0f, 0.0f, 0.0f);
    bloom += blurTexture0.Sample(smp, input.texcoord);
    bloom += blurTexture1.Sample(smp, input.texcoord);
    //bloom += blurTexture2.Sample(smp, input.texcoord);
    //bloom += blurTexture3.Sample(smp, input.texcoord);
    bloom /= NUM_TEXTURES;
    bloom.a = 1.0f;

    output.color = bloom;
    
    return output;
}