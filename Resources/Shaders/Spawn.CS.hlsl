#include "GPUParticle.hlsli"

RWStructuredBuffer<Particle> Output : register(u0);

RWStructuredBuffer<Emitter> gEmitter : register(u1);

struct EmitterCounterBuffer
{
    uint emitterCounter;
};

ConstantBuffer<EmitterCounterBuffer> gEmitterCounter : register(b0);

void CreateParticle(uint emitterIndex, uint index)
{
    Output[index].scale = float3(0.5f, 0.5f, 0.5f);
    Output[index].rotate = float3(0.0f, 0.0f, 0.0f);
    Output[index].translate.x = random(gEmitter[emitterIndex].min.x, gEmitter[emitterIndex].max.x, float(index) * 2.0f);
    Output[index].translate.y = random(gEmitter[emitterIndex].min.y, gEmitter[emitterIndex].max.y, float(index) * 1.0f);
    Output[index].translate.z = random(gEmitter[emitterIndex].min.z, gEmitter[emitterIndex].max.z, float(index) * 3.0f);
    Output[index].isAlive = true;
    Output[index].isHit = false;
    Output[index].aliveTime = random(360.0f, 600.0f, float(index) * 1414531.0f);
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

[numthreads(1, 1, 1)]
void main(uint3 groupId : SV_GroupID, uint groupIndex : SV_GroupIndex)
{
    uint index = (groupId.x * 1) + groupIndex;
    uint cureateParticleNum = 0;
    uint emitterCounter = gEmitterCounter.emitterCounter;
    for (int emitterIndex = 0; emitterIndex < emitterCounter; emitterIndex++)
    {
        gEmitter[emitterIndex].frequencyTime++;

        if (gEmitter[emitterIndex].frequencyTime >= gEmitter[emitterIndex].frequency)
        {
            for (int i = 0; i < gEmitter[emitterIndex].maxParticleNum; i++)
            {
                if (!Output[i].isAlive)
                {
                    CreateParticle(emitterIndex,i);
                    cureateParticleNum++;
                    
                    if (cureateParticleNum > gEmitter[emitterIndex].createParticleNum)
                    {
                        gEmitter[emitterIndex].frequencyTime = 0;
                        break;
                    }
                }
            }
        }
    }
}
