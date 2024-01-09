#include "GPUParticle.hlsli"

RWStructuredBuffer<Particle> Input : register(u0);

struct DrawIndex
{
    uint IndexCountPerInstance;
    uint InstanceCount;
    uint StartIndexLocation;
    int BaseVertexLocation;
    uint StartInstanceLocation;
    float pad;
};

struct IndirectCommand
{
    uint2 cbvAddress;
    DrawIndex drawArguments;
};
StructuredBuffer<IndirectCommand> inputCommands : register(t0);
AppendStructuredBuffer<IndirectCommand> outputCommands : register(u1);

struct ParticleInfo
{
    float speed;
};
ConstantBuffer<ParticleInfo> Info : register(b0);

struct Ball
{
    float3 position;
    float size;
};
StructuredBuffer<Ball> ball : register(t1);

struct BallCount
{
    int ballCount;
};
ConstantBuffer<BallCount> ballCount: register(b1);

ConstantBuffer<ParticleArea> gParticleArea : register(b2);

[numthreads(threadBlockSize, 1, 1)]
void main(uint3 groupId : SV_GroupID, uint groupIndex : SV_GroupIndex)
{
    uint index = (groupId.x * threadBlockSize) + groupIndex;
    
    Input[index].translate += Input[index].velocity * Info.speed;
    if (!Input[index].isHit)
    {
        if (Input[index].translate.x <= gParticleArea.min.x ||
        Input[index].translate.x >= gParticleArea.max.x)
        {
            Input[index].velocity.x *= -1.0f;
        }
        if (Input[index].translate.y <= gParticleArea.min.y ||
        Input[index].translate.y >= gParticleArea.max.y)
        {
            Input[index].velocity.y *= -1.0f;
        }
        if (Input[index].translate.z <= gParticleArea.min.z ||
        Input[index].translate.z >= gParticleArea.max.z)
        {
            Input[index].velocity.z *= -1.0f;
        }
        for (int i = 0; i < ballCount.ballCount; i++)
        {
            float x = (Input[index].translate.x - ball[i].position.x) * (Input[index].translate.x - ball[i].position.x);
            float y = (Input[index].translate.y - ball[i].position.y) * (Input[index].translate.y - ball[i].position.y);
            float z = (Input[index].translate.z - ball[i].position.z) * (Input[index].translate.z - ball[i].position.z);
    
            if (sqrt(x + y + z) <= Input[index].scale.x + ball[i].size)
            {
                Input[index].isHit = true;
                Input[index].velocity = normalize(Input[index].translate - ball[i].position);
                Input[index].velocity *= 10.0f;

            }
            
        }
    }
    else
    {
        Input[index].aliveTime--;
        if (Input[index].aliveTime <= 0.0f)
        {
            Input[index].isAlive = false;
        }
    }
    if (Input[index].isAlive)
    {
        outputCommands.Append(inputCommands[index]);
    }
}