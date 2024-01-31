struct WorldTransform {
    float4x4 mat;
};

struct Material
{
    float4 color;
};

struct VSOutput
{
    float4 svpos : SV_POSITION;
    float2 uv : TEXCOORD0;
};