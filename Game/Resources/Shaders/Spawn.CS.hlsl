struct Particle
{
    float3 position;
    float3 velocity;
};

RWStructuredBuffer<Particle> Output : register(u0);

[numthreads(1, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    Output[DTid.x].position.x = cos(DTid.x) * DTid.x * 0.5f;
    Output[DTid.x].position.y = sin(DTid.x) * DTid.x * 0.5f;
    Output[DTid.x].velocity = -Output[DTid.x].position;
    Output[DTid.x].velocity = normalize(Output[DTid.x].velocity);

}