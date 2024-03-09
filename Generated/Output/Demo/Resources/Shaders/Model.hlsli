struct VertexShaderOutput
{
    float4 position : SV_POSITION;
    float3 normal : NORMAL0;
    float2 texcoord : TEXCOORD0;
};

struct WorldTransform
{
    float4x4 world;
    float4x4 inverseMatWorld;
};

struct ViewProjection
{
    float4x4 view;
    float4x4 projection;
    float4x4 inverseView;
    float3 cameraPos;
};

struct Material
{
    float4 color;
};

struct DirectionLight
{
    float4 color;
    float3 direction;
    float intensity;
    float sharpness;
};

struct PointLight
{
    float4 color;
    float3 position;
    float intensity;
    float radius;
    float decay;
    float sharpness;
    float pad;
};