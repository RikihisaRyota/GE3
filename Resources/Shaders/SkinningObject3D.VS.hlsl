#include "Model.hlsli"

struct Well{
	float32_t4x4 skeletonSpaceMatrix;
	float32_t4x4 skeletonSpaceMInverseTransposeMatrix;
};

StructuredBuffer<Well> gMatrixPalette : register(t1);

ConstantBuffer<WorldTransform> gWorldTransform : register(b0);

ConstantBuffer<ViewProjection> gViewProjection : register(b1);

struct VertexShaderInput
{
    float3 position : POSITION0;
    float3 normal : NORMAL0;
    float2 texcoord : TEXCOORD0;
	float32_t4 weight : WEIGHT0;
    int32_t4 index : INDEX0;
};

struct Skinned{
	float32_t4 position;
	float32_t3 normal;
};

Skinned Skinning(VertexShaderInput input){
    Skinned skinned;
    
    skinned.position = mul(float32_t4(input.position,1.0f), gMatrixPalette[input.index.x].skeletonSpaceMatrix) * input.weight.x;
    skinned.position += mul(float32_t4(input.position,1.0f), gMatrixPalette[input.index.y].skeletonSpaceMatrix) * input.weight.y;
    skinned.position += mul(float32_t4(input.position,1.0f), gMatrixPalette[input.index.z].skeletonSpaceMatrix) * input.weight.z;
    skinned.position += mul(float32_t4(input.position,1.0f), gMatrixPalette[input.index.w].skeletonSpaceMatrix) * input.weight.w;
    skinned.position.w = 1.0f;

    skinned.normal = mul(input.normal, (float3x3)gMatrixPalette[input.index.x].skeletonSpaceMInverseTransposeMatrix) * input.weight.x;
    skinned.normal += mul(input.normal, (float3x3)gMatrixPalette[input.index.y].skeletonSpaceMInverseTransposeMatrix) * input.weight.y;
    skinned.normal += mul(input.normal, (float3x3)gMatrixPalette[input.index.z].skeletonSpaceMInverseTransposeMatrix) * input.weight.z;
    skinned.normal += mul(input.normal, (float3x3)gMatrixPalette[input.index.w].skeletonSpaceMInverseTransposeMatrix) * input.weight.w;
    skinned.normal = normalize(skinned.normal);
    
    return skinned;
}

VertexShaderOutput main(VertexShaderInput input)
{
    VertexShaderOutput output;
    Skinned skinned = Skinning(input);
    output.position = mul(mul(skinned.position, gWorldTransform.world), mul(gViewProjection.view, gViewProjection.projection));
    output.normal = normalize(mul(skinned.normal, (float3x3)gWorldTransform.inverseMatWorld));
    output.texcoord = input.texcoord;
    return output;
}