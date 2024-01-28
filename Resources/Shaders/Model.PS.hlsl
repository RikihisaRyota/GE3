#include "Model.hlsli"
#include "Lighting/Lighting.hlsli"

Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

ConstantBuffer<WorldTransform> gWorldTransform : register(b0);

ConstantBuffer<ViewProjection> gViewProjection : register(b1);

ConstantBuffer<Material> gMaterial : register(b2);

ConstantBuffer<DirectionLight> gDirectionLight : register(b3);

ConstantBuffer<PointLight> gPointLight : register(b4);

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    float3 worldPos = gWorldTransform.world._m30_m31_m32;
    float4 textureColor = gTexture.Sample(gSampler, input.texcoord.xy);
    output.color = textureColor;
    // ディフーズ
    float halfRanbert = HalfRanbert(input.normal, gDirectionLight.direction);
    float3 diffuse = float3(gDirectionLight.color.rgb * halfRanbert * gDirectionLight.intensity);
    // スペキュラー
    float blinnPhongReflection = BlinnPhongReflection(input.normal, gViewProjection.cameraPos, worldPos,gDirectionLight.direction);
    float3 specular = float3(gDirectionLight.color.rgb * gDirectionLight.sharpness * blinnPhongReflection * float3(1.0f, 1.0f, 1.0f));
    // アンビエント
    float3 ambient = float3(0.1f, 0.1f, 0.1f);
    // ポイントライト
    float pointLightHalfRanbert = PointLightHalfRanbert(input.normal, worldPos, gPointLight.position);
    float3 pointLightDiffuse = float3(gPointLight.color.rgb * pointLightHalfRanbert * gPointLight.intensity);
    float3 pointLightBlinnPhongReflection = PointLightBlinnPhongReflection(input.normal, worldPos, gPointLight.position);
    float3 pointLightSpecular = float3(gPointLight.color.rgb * pointLightBlinnPhongReflection * gPointLight.intensity);
    
    output.color.rgb = gMaterial.color.rgb * textureColor.rgb * ((diffuse + specular + ambient) + (pointLightDiffuse + pointLightSpecular));
    output.color.a = gMaterial.color.a;
    return output;
    
}
