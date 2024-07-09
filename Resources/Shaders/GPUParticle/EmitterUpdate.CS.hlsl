#include "GPUParticle.hlsli"

RWStructuredBuffer<Emitter> inputEmitter : register(u0);
AppendStructuredBuffer<CreateParticleNum> createParticle : register(u1);
RWStructuredBuffer<uint> createParticleCounter : register(u2);

[numthreads(emitterSize, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint index = DTid.x;
    // 生きているエミッターのみ
    if (index < emitterSize && inputEmitter[index].isAlive)
    {
        // 生成
        if (inputEmitter[index].time.particleTime >= inputEmitter[index].frequency.interval)
        {
            // 生成
            CreateParticleNum particle;
            particle.emitterNum = index;
            particle.createParticleNum = inputEmitter[index].createParticleNum;
            // Interlocked
            InterlockedAdd(createParticleCounter[0], inputEmitter[index].createParticleNum);
            createParticle.Append(particle);
            inputEmitter[index].time.particleTime = 0;
        } else {
            inputEmitter[index].time.particleTime++;
        }
        // ループしないやつ
        if(!inputEmitter[index].frequency.isLoop){
            if(inputEmitter[index].time.emitterTime >= inputEmitter[index].frequency.emitterLife){
                inputEmitter[index].isAlive = false;
                inputEmitter[index].emitterCount = -1;
                inputEmitter[index].time.particleTime = 0;
                inputEmitter[index].time.emitterTime = 0;
            }else{
                inputEmitter[index].time.emitterTime++;
            }
        }
    }
}