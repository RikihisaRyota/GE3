#include "./Fullscreen.hlsli"

static const uint32_t kNumVertex=3;

static const float32_t4 kPositions[kNumVertex]={
	{-1.0f,1.0f,0.0f,1.0f},
	{3.0f,1.0f,0.0f,1.0f},
	{-1.0f,-3.0f,0.0f,1.0f},
};

static const float32_t2 kTexcoords[kNumVertex]={
	{0.0f,0.0f},
	{2.0f,0.0f},
	{0.0f,2.0f},
};

//struct VertexShaderOutPut
//{
//    float32_t4 position : SV_POSITION;
//    float32_t2 texcoord : TEXCOORD0;
//};

VertexShaderOutPut main(uint32_t vertexId: SV_VERTEXID)
{
    VertexShaderOutPut output;
    output.position = kPositions[vertexId];
    output.texcoord = kTexcoords[vertexId];
    return output;
}