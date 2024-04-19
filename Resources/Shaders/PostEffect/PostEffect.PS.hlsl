#include "../Fullscreen.hlsli"
#include "PostEffect.hlsli"

Texture2D<float4> tex : register(t0); // 0番スロットに設定されたテクスチャ
SamplerState smp : register(s0); // 0番スロットに設定されたサンプラー

struct PixelShaderOutPut
{
    float4 color : SV_TARGET0;
};

float Random(float2 coord)
{
    return frac(sin(dot(coord, float2(8.7819, 3.255))) * 437.645);
}

PixelShaderOutPut main(VertexShaderOutPut input)
{
    // グローエフェクトの閾値
    float glowThreshold = 0.3f;
    // グローエフェクトの強度
    float glowIntensity = 1.2f;

    float saturation = 2.0f;
    float contrast = 1.5f;

    float motionBlurAmount = 0.02f;
    
    PixelShaderOutPut output;
    float2 samplePoint = input.texcoord;
    output.color = tex.Sample(smp, input.texcoord);
    //float4 textureColor = output.color;
	//output.color.rgb = Smoothing(samplePoint,tex,smp);
    
    //float2 center = float2(0.5f, 0.5f); // 画面中央
    //float radius = clamp(gTime.time,0.0f,1.0f); // 円の半径
    //float distance = length((input.texcoord - center) * float2(1280.0f / 720.0f, 1.0f));
    //if (distance <= radius)
    //{
    //    // 円の内側
    //    output.color = tex.Sample(smp, input.texcoord);
    //}
    //else
    //{
    //    // 円の外側
    //    output.color = float4(1.0f, 1.0f, 1.0f, 1.0f);
    //}
    //// モーションブラー
    //float2 motionBlurOffset = float2(sin(input.texcoord.y * motionBlurAmount), cos(input.texcoord.x * motionBlurAmount));
    //output.color = tex.Sample(smp, motionBlurOffset);
    //float4 textureColor = output.color;
    // 反転
    //output.color.rgb = (1.0f - output.color.r, 1.0f - output.color.g, 1.0f - output.color.b);
    // 白黒
    //float averageColor = (output.color.r + output.color.g + output.color.b) / 3.0f;
    //output.color.rgb = (averageColor, averageColor, averageColor);
    // ノイズ
    //float noise = Random(input.texcoord * gTime.time) - 0.5f;
    //output.color.rgb += float3(noise, noise, noise);
    // ゆがませる
    //static const float Distortion = 0.05f;
    //samplePoint -= float2(0.5f, 0.5f);
    //float distPower = pow(length(samplePoint), Distortion);
    //samplePoint *= float2(distPower, distPower);
    //samplePoint += float2(0.5f, 0.5f);
    //output.color = tex.Sample(smp, samplePoint);
    // RGBずらし
    //samplePoint = input.texcoord;
    //samplePoint.r += 0.005f;
    //output.color.r = tex.Sample(smp, samplePoint).r;
    // 走査線
    //float sinv = sin(input.texcoord.y * 2 + gTime.time * -0.1);
    //float steped = step(0.99, sinv * sinv);
    //output.color.rgb -= (1 - steped) * abs(sin(input.texcoord.y * 50.0 + gTime.time * 1.0)) * 0.05;
    //output.color.rgb -= (1 - steped) * abs(sin(input.texcoord.y * 100.0 - gTime.time * 2.0)) * 0.08;
    //output.color.rgb += steped * 0.1;
    // ビネット
    //float vignette = length(float2(0.5, 0.5) - input.texcoord);
    //vignette = clamp(vignette - 0.2, 0, 1);
    //output.color.rgb -= vignette;
    //// ブルーム
    //float luminance = (textureColor.r + textureColor.g + textureColor.b) / 3.0f;
    //
    //// グローエフェクト
    //if (luminance > glowThreshold)
    //{
    //    output.color += textureColor * glowIntensity;
    //}
    //
    //// 彩度とコントラストの調整
    //float3 color = saturate((textureColor.rgb - 0.5f) * contrast + 0.5f) * saturation;
    //output.color += float4(color, textureColor.a);
    //
    //
    //
    // カラーグレーディング
    //float4 gradedColor = textureColor;
    // gradedColor.rgb *= float3(0.1f, 0.1f, 0.6f); // 例として青みを強調
    //.color += gradedColor;
    return output;
}