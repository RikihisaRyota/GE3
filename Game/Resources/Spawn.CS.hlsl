struct ParticleInfo
{
    float3 position;
    float speed;
};
ConstantBuffer<ParticleInfo> gParticleInfo : register(b0);

RWStructuredBuffer<ParticleInfo> Output : register(u0);

[numthreads(1, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    Output[DTid.x].position = gParticleInfo.position;
    Output[DTid.x].speed = gParticleInfo.speed;
}