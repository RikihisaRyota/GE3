#include "GPUParticle.hlsli"

RWStructuredBuffer<Particle> Output : register(u0);

[numthreads(1, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    float angle = radians(float(DTid.x) * 1.0f);
    float radius = float(DTid.x) * 0.02f;
    
    Output[DTid.x].scale = float3(1.0f, 1.0f, 1.0f);
    Output[DTid.x].rotate = float3(0.0f, 0.0f, 0.0f);
    Output[DTid.x].tarnslate = float3(cos(angle) * radius, sin(angle) * radius, 0.0f);
    
    Output[DTid.x].matWorld = MakeAffine(Output[DTid.x].scale, Output[DTid.x].rotate, Output[DTid.x].tarnslate);
    
    Output[DTid.x].velocity = -Output[DTid.x].tarnslate;
}