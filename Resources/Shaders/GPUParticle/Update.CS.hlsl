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
        float t = float(Input[index].particleLifeTime.time) / float(Input[index].particleLifeTime.maxTime);
        Input[index].particleLifeTime.time++;
        Input[index].scale = lerp(Input[index].scaleRange.min, Input[index].scaleRange.max, t);
        
        Input[index].rotate += Input[index].rotateVelocity;
        
        Input[index].translate += Input[index].velocity;
        
        if (Input[index].particleLifeTime.time >= Input[index].particleLifeTime.maxTime)
        {
            Input[index].isAlive = false;
            particleIndexCommands.Append(index);
        }else{
            outputDrawIndexCommands.Append(index);
        }
    }
}