#include "../Fullscreen.hlsli"

#define NUM_TEXTURES 4

// テクスチャの個別定義
#if NUM_TEXTURES >= 1
    Texture2D<float4> blurTexture0 : register(t0);
#endif
#if NUM_TEXTURES >= 2
    Texture2D<float4> blurTexture1 : register(t1);
#endif
#if NUM_TEXTURES >= 3
    Texture2D<float4> blurTexture2 : register(t2);
#endif
#if NUM_TEXTURES >= 4
    Texture2D<float4> blurTexture3: register(t3);
#endif
Texture2D<float4> originalTexture : register(t4);
SamplerState smp : register(s0);

struct PixelShaderOutPut
{
    float32_t4 color : SV_TARGET0;
};

PixelShaderOutPut main(VertexShaderOutPut input)
{
    PixelShaderOutPut output;

    float32_t4 bloom = float4(0.0f, 0.0f, 0.0f, 0.0f);

    #if NUM_TEXTURES >= 1
        bloom += blurTexture0.Sample(smp, input.texcoord);
    #endif

    #if NUM_TEXTURES >= 2
        bloom += blurTexture1.Sample(smp, input.texcoord);
    #endif

    #if NUM_TEXTURES >= 3
        bloom += blurTexture2.Sample(smp, input.texcoord);
    #endif

    #if NUM_TEXTURES >= 4
        bloom += blurTexture3.Sample(smp, input.texcoord);
    #endif
    
    bloom += originalTexture.Sample(smp, input.texcoord);
    
    bloom /= NUM_TEXTURES + 1;
    bloom.a = 1.0f;

    output.color = bloom;

    return output;
}
