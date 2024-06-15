#include "Model.hlsli"
#include "Lighting/Lighting.hlsli"

Texture2D<float32_t4> gTexture : register(t0);
TextureCube<float32_t4> gEnvironmentalTexture : register(t1);
SamplerState gSampler : register(s0);

ConstantBuffer<WorldTransform> gWorldTransform : register(b0);

ConstantBuffer<ViewProjection> gViewProjection : register(b1);

ConstantBuffer<Material> gMaterial : register(b2);

ConstantBuffer<DirectionLight> gDirectionLight : register(b3);

ConstantBuffer<PointLight> gPointLight : register(b4);

struct PixelShaderOutput
{
    float32_t4 color : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    float32_t3 worldPos = gWorldTransform.world._m30_m31_m32;
    float32_t4 textureColor = gTexture.Sample(gSampler, input.texcoord.xy);
    output.color = textureColor*gMaterial.color;
    // ディフーズ
    float32_t halfRanbert = HalfRanbert(input.normal, gDirectionLight.direction);
    float32_t3 diffuse = float32_t3(gDirectionLight.color.rgb * halfRanbert * gDirectionLight.intensity);
    // スペキュラー
    float32_t blinnPhongReflection = BlinnPhongReflection(input.normal, gViewProjection.cameraPos, worldPos, gDirectionLight.direction, gDirectionLight.sharpness);
    float32_t3 specular = float32_t3(gDirectionLight.color.rgb * blinnPhongReflection * float32_t3(1.0f, 1.0f, 1.0f));
    // アンビエント
    float32_t3 ambient = float32_t3(0.1f, 0.1f, 0.1f);
    // ポイントライト
    float32_t factor = Factor(worldPos, gPointLight.position, gPointLight.radius, gPointLight.decay);
    // 環境マップ
    float32_t3 cameraPosition=normalize(worldPos-gViewProjection.cameraPos);
    float32_t3 reflecedVector=reflect(cameraPosition,normalize(input.normal));
    float32_t4 environmentColor=gEnvironmentalTexture.Sample(gSampler,reflecedVector);

    float32_t pointLightHalfRanbert = PointLightHalfRanbert(input.normal, worldPos, gPointLight.position);
    float32_t3 pointLightDiffuse = float32_t3(gPointLight.color.rgb * pointLightHalfRanbert * factor * gPointLight.intensity);
    
    float32_t3 pointLightBlinnPhongReflection = PointLightBlinnPhongReflection(input.normal,worldPos, gPointLight.position, gDirectionLight.sharpness);
    float32_t3 pointLightSpecular = float32_t3(gPointLight.color.rgb * pointLightBlinnPhongReflection * factor * gPointLight.intensity);
    
    output.color.rgb = gMaterial.color.rgb * textureColor.rgb * ((diffuse + specular + ambient) + (pointLightDiffuse + pointLightSpecular));
    output.color.a = gMaterial.color.a;
    //output.color = environmentColor*gMaterial.environmentCoefficient;
    return output;
    
}
