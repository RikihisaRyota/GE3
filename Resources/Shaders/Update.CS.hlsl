#include "GPUParticle.hlsli"

RWStructuredBuffer<Particle> Input : register(u0);

AppendStructuredBuffer<uint> particleIndexCommands : register(u1);

AppendStructuredBuffer<uint> outputDrawIndexCommands : register(u2);

[numthreads(threadBlockSize, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint index = DTid.x;
    if (Input[index].isAlive)
    {
        Input[index].aliveTime -= 1.0f;
        //Input[index].translate += Input[index].velocity * 0.1f;
        
        if (Input[index].aliveTime <= 0.0f)
        {
            Input[index].isAlive = false;
            particleIndexCommands.Append(index);
        }
        outputDrawIndexCommands.Append(index);
    }
}