#include "GPUParticle.hlsli"

RWStructuredBuffer<Particle> Output : register(u0);

StructuredBuffer<Emitter> gEmitter : register(t0);

struct Random
{
    uint32_t random;
};


ConstantBuffer<Random> gRandom : register(b0);

ConsumeStructuredBuffer<uint> particleIndexCommands : register(u1);

RWStructuredBuffer<CreateParticleNum> createParticle : register(u2);

struct CounterParticle
{
    int32_t count;
};
RWStructuredBuffer<CounterParticle> particleIndexCounter : register(u3);

[numthreads(1, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID, uint3 GTid : SV_GroupThreadID, uint3 GID : SV_GroupID)
{
    // グループスレッドIDを使用して、オリジナルエミッターのインデックスを取得
    uint32_t origalIndex = GTid.x;

    // グループIDを使用(dispach数)
    uint32_t emitterNum  = GID.y;
    if(createParticle[emitterNum].createParticleNum > 0){
        int32_t createNum=-1; 
        InterlockedAdd(createParticle[emitterNum].createParticleNum, -1,createNum);
        if (createNum > 0)
        { 
            int32_t counter=-1;
            InterlockedAdd(particleIndexCounter[0].count, -1,counter);
            if(counter>0){
                int index = particleIndexCommands.Consume();
                uint32_t seed = setSeed(index * gRandom.random);
                uint32_t emitterIndex=createParticle[emitterNum].emitterNum;
                Emitter emitter=gEmitter[emitterIndex];

                CreateParticle(Output[index], emitter,seed);
            }
        }
    }
}