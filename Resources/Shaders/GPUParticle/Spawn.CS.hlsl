#include "GPUParticle.hlsli"

// struct EmitterCountBuffer
// {
//     uint32_t count;
// };

// ConstantBuffer<EmitterCountBuffer> gEmitterCount : register(b0);

RWStructuredBuffer<Particle> Output : register(u0);

StructuredBuffer<Emitter> gEmitter : register(t0);

ConsumeStructuredBuffer<uint> particleIndexCommands : register(u1);

RWStructuredBuffer<CreateParticle> createParticle : register(u2);

void LifeTime(uint index, uint32_t emitterIndex)
{
    
    Output[index].particleLifeTime.maxTime = random(gEmitter[emitterIndex].particleLifeSpan.range.min, gEmitter[emitterIndex].particleLifeSpan.range.max, float(index) * 1414531.0f);
    Output[index].particleLifeTime.time = 0;
}

void Scale(uint index,  uint32_t emitterIndex)
{
    Output[index].scaleRange.min.x = random(gEmitter[emitterIndex].scale.range.start.min.x, gEmitter[emitterIndex].scale.range.start.max.x, float(index) * 1.2136912f);
    Output[index].scaleRange.min.y = random(gEmitter[emitterIndex].scale.range.start.min.y, gEmitter[emitterIndex].scale.range.start.max.y, float(index) * 1.1749621f);
    Output[index].scaleRange.min.z = random(gEmitter[emitterIndex].scale.range.start.min.z, gEmitter[emitterIndex].scale.range.start.max.z, float(index) * 1.24412f);
    
    Output[index].scaleRange.max.x = random(gEmitter[emitterIndex].scale.range.end.min.x, gEmitter[emitterIndex].scale.range.end.max.x, float(index) * 1.42356f);
    Output[index].scaleRange.max.y = random(gEmitter[emitterIndex].scale.range.end.min.y, gEmitter[emitterIndex].scale.range.end.max.y, float(index) * 1.3247f);
    Output[index].scaleRange.max.z = random(gEmitter[emitterIndex].scale.range.end.min.z, gEmitter[emitterIndex].scale.range.end.max.z, float(index) * 1.257212f);
    
    Output[index].scale = Output[index].scaleRange.min;
}

void Rotate(uint index, uint32_t emitterIndex)
{
    Output[index].rotateVelocity = gEmitter[emitterIndex].rotateAnimation.rotate;
    
}

void Translate(uint index,  uint32_t emitterIndex)
{
    Output[index].translate = gEmitter[emitterIndex].area.position;
    Output[index].translate.x += random(gEmitter[emitterIndex].area.range.min.x, gEmitter[emitterIndex].area.range.max.x, float(index) * 2.21341f);
    Output[index].translate.y += random(gEmitter[emitterIndex].area.range.min.y, gEmitter[emitterIndex].area.range.max.y, float(index) * 3.4214f);
    Output[index].translate.z += random(gEmitter[emitterIndex].area.range.min.z, gEmitter[emitterIndex].area.range.max.z, float(index) * 4.2108124f);
    
}

void Velocity(uint index,  uint32_t emitterIndex)
{
    Output[index].velocity.x = random(gEmitter[emitterIndex].velocity3D.range.min.x, gEmitter[emitterIndex].velocity3D.range.max.x, float(index) * 1.1423589f);
    Output[index].velocity.y = random(gEmitter[emitterIndex].velocity3D.range.min.y, gEmitter[emitterIndex].velocity3D.range.max.y, float(index) * 1.19786457f);
    Output[index].velocity.z = random(gEmitter[emitterIndex].velocity3D.range.min.z, gEmitter[emitterIndex].velocity3D.range.max.z, float(index) * 1.36266f);
}

void Color(uint index,  uint32_t emitterIndex)
{
    Output[index].color.r = random(gEmitter[emitterIndex].color.range.start.min.r, gEmitter[emitterIndex].color.range.start.min.r, float(index) * 1.2142f);
    Output[index].color.g = random(gEmitter[emitterIndex].color.range.start.min.g, gEmitter[emitterIndex].color.range.start.min.g, float(index) * 3.14531215f);
    Output[index].color.b = random(gEmitter[emitterIndex].color.range.start.min.b, gEmitter[emitterIndex].color.range.start.min.b, float(index) * 2.124173180f);
    Output[index].color.a = random(gEmitter[emitterIndex].color.range.start.min.a, gEmitter[emitterIndex].color.range.start.min.a, float(index) * 1.1238102f);
}

void Create(uint index,  uint32_t emitterIndex)
{
    Output[index].textureInidex = gEmitter[emitterIndex].textureIndex;
    
    Output[index].isAlive = true;
    
    LifeTime(index, emitterIndex);
    
    Scale(index, emitterIndex);
    
    Rotate(index, emitterIndex);
    
    Translate(index, emitterIndex);
    
    Velocity(index, emitterIndex);
    
    Color(index, emitterIndex);   
}


[numthreads(threadBlockSize, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    for (int i = 0; i < emitterSize; i++)
    {
        // ここで同期処理をしたい
        // 並列処理で複数のスレットがcreateParticle[i].createParticleNumの計算を行っていて
        // 現在のcreateParticle[i].createParticleNum の中身を知りたい
        // createParticle[i].createParticleNum の値を安全に読み取る
        InterlockedAdd(createParticle[i].createParticleNum, -1);
        int32_t createNum=createParticle[i].createParticleNum; 
        if (createNum > 0)
        {
                int index = particleIndexCommands.Consume();
                uint32_t emitterIndex = createParticle[i].emitterNum;
                Create(index, emitterIndex);
                break;
        }
    }
}