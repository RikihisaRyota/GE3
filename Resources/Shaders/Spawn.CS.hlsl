#include "GPUParticle.hlsli"

RWStructuredBuffer<Particle> Output : register(u0);

ConstantBuffer<Emitter> gEmitter : register(b0);

ConsumeStructuredBuffer<uint> particleIndexCommands : register(u1);

void CreateParticle(uint index)
{
    Output[index].scale = float3(0.1f, 0.1f, 0.1f);
    Output[index].rotate = float3(0.0f, 0.0f, 0.0f);
    Output[index].translate = gEmitter.position;
    Output[index].translate.x += random(gEmitter.min.x, gEmitter.max.x, float(index) * 2.0f);
    Output[index].translate.y += random(gEmitter.min.y, gEmitter.max.y, float(index) * 1.0f);
    Output[index].translate.z += random(gEmitter.min.z, gEmitter.max.z, float(index) * 3.0f);
   
    Output[index].isAlive = true;
    Output[index].isHit = false;
    Output[index].aliveTime = random(30.0f*1.0f, 30.0f*2.0f, float(index) * 1414531.0f);
    Output[index].velocity.x = random(0.1f, 0.3f, float(index) * 3.0f);
}

[numthreads(threadBlockSize, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint index = particleIndexCommands.Consume();
    CreateParticle(index);
}
