
struct Well {
	float32_t4x4 skeletonSpaceMatrix;
	float32_t4x4 skeletonSpaceInverseTransposeMatrix;
};
StructuredBuffer<Well>  gMatrixPalette:register(t0);

struct Vertex
{
    float32_t3 position;
    float32_t3 normal;
    float32_t2 texcoord;
};
StructuredBuffer<Vertex> gInputVertices:register(t1);

struct VertexInfluence{
    float32_t4 weight;
    int32_t4 index;
};
StructuredBuffer<VertexInfluence> gInfluence:register(t2);

RWStructuredBuffer<Vertex> gOutputVertices:register(u0);

struct SkinningInfomation{
    uint32_t numVertices;
};
ConstantBuffer<SkinningInfomation> gSkinningInfomation:register(b0);

Vertex Skinning(Vertex input,VertexInfluence influence){
    Vertex skinned;
    
    skinned.position = mul(input.position, (float32_t3x3) gMatrixPalette[influence.index.x].skeletonSpaceMatrix) * influence.weight.x;
    skinned.position += mul(input.position,  (float32_t3x3)gMatrixPalette[influence.index.y].skeletonSpaceMatrix) * influence.weight.y;
    skinned.position += mul(input.position,  (float32_t3x3)gMatrixPalette[influence.index.z].skeletonSpaceMatrix) * influence.weight.z;
    skinned.position += mul(input.position,  (float32_t3x3)gMatrixPalette[influence.index.w].skeletonSpaceMatrix) * influence.weight.w;
    //skinned.position.w = 1.0f;

    skinned.normal = mul(input.normal, (float32_t3x3)gMatrixPalette[influence.index.x].skeletonSpaceInverseTransposeMatrix) * influence.weight.x;
    skinned.normal += mul(input.normal, (float32_t3x3)gMatrixPalette[influence.index.y].skeletonSpaceInverseTransposeMatrix) * influence.weight.y;
    skinned.normal += mul(input.normal, (float32_t3x3)gMatrixPalette[influence.index.z].skeletonSpaceInverseTransposeMatrix) * influence.weight.z;
    skinned.normal += mul(input.normal, (float32_t3x3)gMatrixPalette[influence.index.w].skeletonSpaceInverseTransposeMatrix) * influence.weight.w;
    skinned.normal = normalize(skinned.normal);
    
    skinned.texcoord=input.texcoord;

    return skinned;
}


[numthreads(1024, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    uint32_t vertexIndex=DTid.x;
    if(vertexIndex<gSkinningInfomation.numVertices){
        Vertex input=gInputVertices[vertexIndex];
        VertexInfluence influence=gInfluence[vertexIndex];
        Vertex output=Skinning(input,influence);
        gOutputVertices[vertexIndex]=output;
    }
}