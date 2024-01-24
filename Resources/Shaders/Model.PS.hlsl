#include "Model.hlsli"

Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    float4 textureColor = gTexture.Sample(gSampler, input.texcoord.xy);
    output.color = textureColor;
    // ディフーズ
    float3 diffuse = HalfRanbert(input.normal);
    // スペキュラー
    float3 specular = BlinnPhongReflection(input.normal);
    // アンビエント
    float3 ambient = float3(0.1f, 0.1f, 0.1f);
    // ポイントライト
    float3 pointLightDiffuse = PointLightHalfRanbert(input.normal);
    float3 pointLightSpecular = PointLightBlinnPhongReflection(input.normal);
    
    output.color.rgb = gMaterial.color.rgb * textureColor.rgb * ((diffuse + specular + ambient) + (pointLightDiffuse + pointLightSpecular));
    output.color.a = gMaterial.color.a;
    return output;
    
}
