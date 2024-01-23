struct VertexShaderOutput
{
    float4 position : SV_POSITION;
    float3 normal : NORMAL0;
    float2 texcoord : TEXCOORD0;
};

struct WorldTransform
{
    float4x4 world;
};

ConstantBuffer<WorldTransform> gWorldTransform : register(b0);

struct ViewProjection
{
    float4x4 view;
    float4x4 projection;
    float3 cameraPos;
};

ConstantBuffer<ViewProjection> gViewProjection : register(b1);

struct Material
{
    float4 color;
};

ConstantBuffer<Material> gMaterial : register(b2);

struct DirectionLight
{
    float4 color;
    float3 direction;
    float intensity;
    float sharpness;
};

ConstantBuffer<DirectionLight> gDirectionLight : register(b3);

struct PointLight
{
    float4 color;
    float3 position;
    float intensity;
    float sharpness;
};

ConstantBuffer<PointLight> gPointLight : register(b4);

float3 HalfRanbert(float3 normal)
{
    float NdotL = saturate(dot(normalize(normal), -gDirectionLight.direction));
    float cos = pow(NdotL * 0.5f + 0.5f, 2.0f);
    return float3(gDirectionLight.color.rgb * cos * gDirectionLight.intensity);
}

float3 Ranbert(float3 normal)
{
    float cos = saturate(dot(normalize(normal), -gDirectionLight.direction));
    return float3(gDirectionLight.color.rgb * cos * gDirectionLight.intensity);
}

float3 BlinnPhongReflection(float3 normal)
{
    float3 toEye = normalize(gViewProjection.cameraPos - gWorldTransform.world._m30_m31_m32);
    float3 halfVector = normalize(-gDirectionLight.direction + toEye);
    float NDotH = dot(normalize(normal), halfVector);
    
    float t = saturate(NDotH);
    t = pow(t, gDirectionLight.sharpness);
    return float3(gDirectionLight.color.rgb * gDirectionLight.sharpness * t * float3(1.0f, 1.0f, 1.0f));
}

float3 PhongReflection(float3 normal)
{
    float3 refVec = reflect(-gDirectionLight.direction, normalize(normal));
    float3 toEye = normalize(gViewProjection.cameraPos - gWorldTransform.world._m30_m31_m32);
    float t = saturate(dot(refVec, toEye));
    t = pow(t, gDirectionLight.sharpness);
    return float3(gDirectionLight.color.rgb * gDirectionLight.sharpness * t * float3(1.0f, 1.0f, 1.0f));
}

float3 PointLightDirection()
{
    return normalize(gWorldTransform.world._m30_m31_m32 - gPointLight.position);
}

float Factor()
{
    float distance = length((gPointLight.position - gWorldTransform.world._m30_m31_m32));
    
    return 1.0f / (distance * distance);
}

float3 PointLightHalfRanbert(float3 normal)
{
    float NdotL = saturate(dot(normalize(normal), -PointLightDirection()));
    float cos = pow(NdotL * 0.5f + 0.5f, 2.0f);
    return float3(gPointLight.color.rgb * cos * gPointLight.intensity);
}

float3 PointLightRanbert(float3 normal)
{
    float cos = saturate(dot(normalize(normal),-PointLightDirection()));
    return float3(gPointLight.color.rgb * cos * gPointLight.intensity);
}

float3 PointLightBlinnPhongReflection(float3 normal)
{
    float3 toEye = normalize(gPointLight.position - gWorldTransform.world._m30_m31_m32);
    float3 halfVector = normalize(-PointLightDirection() + toEye);
    float NDotH = dot(normalize(normal), halfVector);
    
    float t = saturate(NDotH);
    t = pow(t, gPointLight.sharpness);
    return float3(gPointLight.color.rgb * gPointLight.sharpness * t * float3(1.0f, 1.0f, 1.0f));
}

float3 PointLightPhongReflection(float3 normal)
{
    float3 refVec = reflect(-PointLightDirection(), normalize(normal));
    float3 toEye = normalize(gPointLight.position- gWorldTransform.world._m30_m31_m32);
    float t = saturate(dot(refVec, toEye));
    t = pow(t, gPointLight.sharpness);
    return float3(gPointLight.color.rgb * gPointLight.sharpness * t * float3(1.0f, 1.0f, 1.0f));
}