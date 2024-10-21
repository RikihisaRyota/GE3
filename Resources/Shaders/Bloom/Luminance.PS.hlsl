#include "../Fullscreen.hlsli"

Texture2D<float32_t4> tex : register(t0); // 0番スロットに設定されたテクスチャ
struct Desc {
    float32_t threshold; // 輝度のしきい値
};

ConstantBuffer<Desc> descBuffer : register(b0);
SamplerState smp : register(s0); // 0番スロットに設定されたサンプラー

struct PixelShaderOutPut {
    float32_t4 color : SV_TARGET0;
};

// 輝度を計算し、しきい値を超えた部分のみ残す
float32_t3 ExtractBrightParts(float32_t3 color, float32_t threshold) {
    // 輝度の計算 (加重平均を使用)
    float32_t luminance = dot(color, float32_t3(0.2126f, 0.7152f, 0.0722f));

    // しきい値を超える場合のみ色を残す
    if (luminance > threshold) {
        return color;
    } else {
        return float32_t3(0.0f, 0.0f, 0.0f); 
    }
}

PixelShaderOutPut main(VertexShaderOutPut input) {
    PixelShaderOutPut output;

    float32_t3 sampledColor = tex.Sample(smp, input.texcoord).rgb;

    float32_t3 outputColor = ExtractBrightParts(sampledColor, descBuffer.threshold);

    output.color.rgb = outputColor;
    output.color.a = 1.0f;

    return output;
}
