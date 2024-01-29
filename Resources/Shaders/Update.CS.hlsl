#include "GPUParticle.hlsli"

RWStructuredBuffer<Particle> Input : register(u0);

StructuredBuffer<uint> inputCommands : register(t0);
AppendStructuredBuffer<uint> outputCommands : register(u1);

StructuredBuffer<Emitter> gEmitter : register(t1);

ConstantBuffer<EmitterCounterBuffer> gEmitterCounter : register(b0);

[numthreads(threadBlockSize, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint index = DTid.x;
    if (Input[index].isAlive)
    {
        Input[index].aliveTime -= 1.0f;
        if (Input[index].aliveTime <= 0.0f)
        {
            Input[index].isAlive = false;
        }
        Input[index].translate += Input[index].velocity * 0.1f;
        outputCommands.Append(index);
    }
}