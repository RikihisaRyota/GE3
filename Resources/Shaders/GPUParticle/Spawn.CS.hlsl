#include "GPUParticle.hlsli"

RWStructuredBuffer<Particle> Output : register(u0);

StructuredBuffer<Emitter> gEmitter : register(t0);

ConsumeStructuredBuffer<uint> particleIndexCommands : register(u1);

RWStructuredBuffer<CreateParticle> createParticle : register(u2);

void Create(uint index, CreateParticle particle)
{
    
    Output[index].aliveTime = random(60.0f * 1.0f, 60.0f * 2.0f, float(index) * 1414531.0f);
    Output[index].scale = float3(0.2f, 0.2f, 0.2f);
    Output[index].rotate = float3(0.0f, 0.0f, 0.0f);
    Output[index].translate = gEmitter[particle.emitterNum].position;
    Output[index].textureInidex = gEmitter[particle.emitterNum].textureInidex;
    Output[index].translate.x += random(gEmitter[particle.emitterNum].min.x, gEmitter[particle.emitterNum].max.x, float(index) * 2.21341f);
    Output[index].translate.y += random(gEmitter[particle.emitterNum].min.y, gEmitter[particle.emitterNum].max.y, float(index) * 3.4214f);
    Output[index].translate.z += random(gEmitter[particle.emitterNum].min.z, gEmitter[particle.emitterNum].max.z, float(index) * 4.2108124f);
    Output[index].velocity = normalize(Output[index].translate);
    Output[index].isAlive = true;
    Output[index].isHit = false;
}

[numthreads(threadBlockSize, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint index = particleIndexCommands.Consume();
    CreateParticle particle;
    for (int i = 0; i < emitterSize; i++)
    {
        // ここで同期処理をしたい
        // 並列処理で複数のスレットがcreateParticle[i].createParticleNumの計算を行っていて
        // 現在のcreateParticle[i].createParticleNum の中身を知りたい
        // createParticle[i].createParticleNum の値を安全に読み取る
        InterlockedAdd(createParticle[i].createParticleNum, -1);
        if (createParticle[i].createParticleNum > 0)
        {
            particle = createParticle[i];
            //createParticle[i].createParticleNum--;
            break;
        }
    }
    Create(index, particle);
}
