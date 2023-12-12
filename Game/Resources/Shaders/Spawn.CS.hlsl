#include "GPUParticle.hlsli"

RWStructuredBuffer<Particle> Output : register(u0);

[numthreads(1, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    float angle = radians(float(DTid.x) * 1.0f);
    float radius = 10.0f;
    
    Output[DTid.x].scale = 0.5f;
    Output[DTid.x].rotate = float3(0.0f, 0.0f, 0.0f);
    Output[DTid.x].translate = float3(cos(angle) * radius, sin(angle) * radius, float(DTid.x)*0.02f);
    
    Output[DTid.x].velocity = normalize(-Output[DTid.x].translate);
}