#include "GPUParticle.hlsli"

RWStructuredBuffer<Particle> Output : register(u0);

ConstantBuffer<Emitter> gEmitter : register(b0);

ConsumeStructuredBuffer<uint> particleIndexCommands : register(u1);

void CreateParticle(uint index)
{
    
    Output[index].aliveTime = random(60.0f * 4.0f, 60.0f * 6.0f, float(index) * 1414531.0f);
    Output[index].scale = float3(0.2f, 0.2f, 0.2f);
    Output[index].rotate = float3(0.0f, 0.0f, 0.0f);
    Output[index].translate = gEmitter.position;
    Output[index].translate.x += random(gEmitter.min.x, gEmitter.max.x, float(index) * 2.21341f);
    Output[index].translate.y += random(gEmitter.min.y, gEmitter.max.y, float(index) * 3.4214f);
    Output[index].translate.z += random(gEmitter.min.z, gEmitter.max.z, float(index) * 4.2108124f);
    Output[index].velocity = normalize(Output[index].translate);
    Output[index].isAlive = true;
    Output[index].isHit = false;
}

[numthreads(threadBlockSize, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint index = particleIndexCommands.Consume();
    CreateParticle(index);
}
