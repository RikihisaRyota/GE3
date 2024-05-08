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
    PixelShaderOutPut output;
    float32_t2 samplePoint = input.texcoord;
    float32_t4 textureColor=tex.Sample(smp, input.texcoord);
    output.color = textureColor;
    //output.color.rgb = Grayscale(textureColor.rgb);
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

    return output;
}