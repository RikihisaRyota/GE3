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
        // 生成
        if (inputEmitter[index].frequency.time <= 0)
        {
            // 生成
            CreateParticle particle;
            particle.emitterNum = index;
            particle.createParticleNum = inputEmitter[index].createParticleNum;
            // Interlocked
            InterlockedAdd(createParticleCounter[0], inputEmitter[index].createParticleNum);
            createParticle.Append(particle);
            inputEmitter[index].frequency.time = inputEmitter[index].frequency.interval;
        } else {
            inputEmitter[index].frequency.time--;
        }
        if(!inputEmitter[index].frequency.isLoop){
            if(inputEmitter[index].frequency.lifeTime > 0){
                inputEmitter[index].frequency.lifeTime--;
            }else{
                inputEmitter[index].isAlive = false;
            }
        }
    }
}