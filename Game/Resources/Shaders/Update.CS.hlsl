#include "GPUParticle.hlsli"

RWStructuredBuffer<Particle> Input : register(u0);

struct IndirectCommand
{
    uint2 cbvAddress;
    uint4 drawArguments;
};
StructuredBuffer<IndirectCommand> inputCommands : register(t0); 
AppendStructuredBuffer<IndirectCommand> outputCommands : register(u1);

struct ParticleInfo
{
    float speed;
};
ConstantBuffer<ParticleInfo> Info : register(b0);

[numthreads(1, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    Input[DTid.x].velocity = normalize(-Input[DTid.x].translate);
    Input[DTid.x].translate.z -= 0.2f;
    if (Input[DTid.x].translate.z >= 0.0f)
    {
        outputCommands.Append(inputCommands[DTid.x]);
    }
}