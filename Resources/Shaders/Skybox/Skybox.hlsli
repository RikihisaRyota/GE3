struct VertexShaderOutput
{
    float32_t4 position : SV_POSITION;
    float32_t3 texcoord : TEXCOORD0;
};

struct WorldTransform
{
    float32_t4x4 world;
    float32_t4x4 inverseMatWorld;
};

struct ViewProjection
{
    float32_t4x4 view;
    float32_t4x4 projection;
    float32_t4x4 inverseView;
    float32_t3 cameraPos;
};