#include "GPUParticle.hlsli"

RWStructuredBuffer<Particle> Output : register(u0);

ConstantBuffer<ParticleArea> gParticleArea : register(b0);


[numthreads(threadBlockSize, 1, 1)]
void main(uint3 groupId : SV_GroupID, uint groupIndex : SV_GroupIndex)
{
    uint index = (groupId.x * threadBlockSize) + groupIndex;
    float angle = radians(float(index) * 5.2f);
    float radius = 15.0f;
    
    Output[index].scale = float3(0.5f, 0.5f, 0.5f);
    Output[index].rotate = float3(0.0f, 0.0f, 0.0f);
    Output[index].translate.x = random(gParticleArea.min.x, gParticleArea.max.x, float(index) * 2314.0f);
    Output[index].translate.y = random(gParticleArea.min.y, gParticleArea.max.y, float(index) * 3217.0f);
    Output[index].translate.z = random(gParticleArea.min.z, gParticleArea.max.z, float(index) * 413129.0f);
    Output[index].isAlive = true;
    Output[index].isHit = false;
    Output[index].aliveTime = random(360.0f,600.0f,float(index) * 1414531.0f);
    if (length(Output[index].translate) != 0.0f)
    {
        Output[index].velocity = normalize(Output[index].translate);
    }
    else
    {
        Output[index].velocity.x = random(-1.0f, 1.0f, float(index) * 21421.0f);
        Output[index].velocity.y = random(-1.0f, 1.0f, float(index) * 3435316.0f);
        Output[index].velocity.z = random(-1.0f, 1.0f, float(index) * 3523142.0f);
        Output[index].velocity = normalize(Output[index].velocity);
    }

}