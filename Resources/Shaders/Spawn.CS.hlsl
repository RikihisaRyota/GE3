#include "GPUParticle.hlsli"

RWStructuredBuffer<Particle> Output : register(u0);

ConstantBuffer<Emitter> gEmitter : register(b0);

ConsumeStructuredBuffer<uint> particleIndexCommands : register(u1);

void CreateParticle(uint index)
{
    
    Output[index].aliveTime = random(30.0f * 1.0f, 30.0f * 2.0f, float(index) * 1414531.0f);
    float seed = Output[index].aliveTime;
    Output[index].scale = float3(0.5f, 0.5f, 0.5f);
    Output[index].rotate = float3(0.0f, 0.0f, 0.0f);
    Output[index].translate = gEmitter.position;
    Output[index].translate.x += random(gEmitter.min.x, gEmitter.max.x, float(index) * seed*2.21341f);
    Output[index].translate.y += random(gEmitter.min.y, gEmitter.max.y, float(index) * seed*3.4214f);
    Output[index].translate.z += random(gEmitter.min.z, gEmitter.max.z, float(index) * seed*4.2108124f);
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
