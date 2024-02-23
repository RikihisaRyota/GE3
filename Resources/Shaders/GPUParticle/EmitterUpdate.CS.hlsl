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
        inputEmitter[index].time++;
        
        if (inputEmitter[index].time >= inputEmitter[index].interval)
        {
            // 生成
            CreateParticle particle;
            particle.emitterNum = index;
            particle.createParticleNum = inputEmitter[index].createParticleNum;
            InterlockedAdd(createParticleCounter[0], inputEmitter[index].createParticleNum);
            createParticle.Append(particle);
            // エミッターがループするか
            if (inputEmitter[index].isLoop)
            {
                inputEmitter[index].time = 0;
            }
            else
            {
                inputEmitter[index].isAlive = false;
            }
        }

    }

}