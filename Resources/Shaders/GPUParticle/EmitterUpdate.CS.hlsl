#include "GPUParticle.hlsli"

RWStructuredBuffer<Emitter> inputEmitter : register(u0);
AppendStructuredBuffer<CreateParticle> createParticle : register(u1);
RWStructuredBuffer<uint> createParticleCounter : register(u2);

[numthreads(emitterSize, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint index = DTid.x;
    // 生きているエミッターのみ
    if (index < emitterSize && inputEmitter[index].isAlive)
    {
        inputEmitter[index].frequency.time--;
        inputEmitter[index].frequency.lifeTime--;
        // 生成
        if (inputEmitter[index].frequency.time >= 0)
        {
            // 生成
            CreateParticle particle;
            particle.emitterNum = index;
            particle.createParticleNum = inputEmitter[index].createParticleNum;
            // Interlocked
            InterlockedAdd(createParticleCounter[0], inputEmitter[index].createParticleNum);
            createParticle.Append(particle);
            inputEmitter[index].frequency.time = inputEmitter[index].frequency.interval;
        }
        // エミッターが死ぬ
        if (inputEmitter[index].frequency.lifeTime >= 0 && !inputEmitter[index].frequency.isLoop)
        {
            inputEmitter[index].isAlive = false;
        }
    }

}