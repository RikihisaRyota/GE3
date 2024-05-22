struct VertexShaderOutput
{
    float32_t4 position : SV_POSITION;
    float32_t3 normal : NORMAL0;
    float32_t2 texcoord : TEXCOORD0;
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
struct Material
{
    float32_t4 color;
    float32_t environmentCoefficient;
};

struct DirectionLight
{
    float32_t4 color;
    float32_t3 direction;
    float32_t intensity;
    float32_t sharpness;
};

struct PointLight
{
    float32_t4 color;
    float32_t3 position;
    float32_t intensity;
    float32_t radius;
    float32_t decay;
    float32_t sharpness;
    float32_t pad;
};