struct Particle
{
    float3 position;
    float3 velocity;
};
StructuredBuffer<Particle> gParticle : register(t0);

struct VertexShaderInput
{
    float4 position : POSITION0;
};

struct VertexShaderOutput
{
    float4 position : SV_POSITION;
};

VertexShaderOutput main(VertexShaderInput input,uint instanceID:SV_InstanceID)
{
    VertexShaderInput output;
    output.position = float4(gParticle[instanceID].position,0.0f);
    return output;
}