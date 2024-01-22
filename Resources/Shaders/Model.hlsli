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
