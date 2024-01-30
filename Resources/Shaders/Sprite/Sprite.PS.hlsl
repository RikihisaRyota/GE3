#include "Sprite.hlsli"

Texture2D<float4> tex : register(t0); // 0番スロットに設定されたテクスチャ
SamplerState smp : register(s0); // 0番スロットに設定されたサンプラー

ConstantBuffer<Material> gMaterial : register(b1);

float4 main(VSOutput input) : SV_TARGET
{
    return gMaterial.color * tex.Sample(smp, input.uv);
}