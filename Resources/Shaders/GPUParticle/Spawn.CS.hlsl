#include "GPUParticle.hlsli"

RWStructuredBuffer<Particle> Output : register(u0);

StructuredBuffer<Emitter> gEmitter : register(t0);

ConsumeStructuredBuffer<uint> particleIndexCommands : register(u1);

RWStructuredBuffer<CreateParticle> createParticle : register(u2);

void LifeTime(uint index, CreateParticle particle)
{
    Output[index].particleLifeTime.maxTime = random(gEmitter[particle.emitterNum].particleLifeSpan.range.min, gEmitter[particle.emitterNum].particleLifeSpan.range.max, float(index) * 1414531.0f);
    Output[index].particleLifeTime.time = 0;
}

void Scale(uint index, CreateParticle particle)
{
    Output[index].scaleRange.min.x = random(gEmitter[particle.emitterNum].scale.range.start.min.x, gEmitter[particle.emitterNum].scale.range.start.max.x, float(index) * 1.2136912f);
    Output[index].scaleRange.min.y = random(gEmitter[particle.emitterNum].scale.range.start.min.y, gEmitter[particle.emitterNum].scale.range.start.max.y, float(index) * 1.1749621f);
    Output[index].scaleRange.min.z = random(gEmitter[particle.emitterNum].scale.range.start.min.z, gEmitter[particle.emitterNum].scale.range.start.max.z, float(index) * 1.24412f);
    
    Output[index].scaleRange.max.x = random(gEmitter[particle.emitterNum].scale.range.end.min.x, gEmitter[particle.emitterNum].scale.range.end.max.x, float(index) * 1.42356f);
    Output[index].scaleRange.max.y = random(gEmitter[particle.emitterNum].scale.range.end.min.y, gEmitter[particle.emitterNum].scale.range.end.max.y, float(index) * 1.3247f);
    Output[index].scaleRange.max.z = random(gEmitter[particle.emitterNum].scale.range.end.min.z, gEmitter[particle.emitterNum].scale.range.end.max.z, float(index) * 1.257212f);
    
    Output[index].scale = Output[index].scaleRange.min;
}

void Rotate(uint index, CreateParticle particle)
{
    Output[index].rotateVelocity = gEmitter[particle.emitterNum].rotateAnimation.rotate;
    
}

void Translate(uint index, CreateParticle particle)
{
    Output[index].translate = gEmitter[particle.emitterNum].area.position;
    Output[index].translate.x += random(gEmitter[particle.emitterNum].area.range.min.x, gEmitter[particle.emitterNum].area.range.max.x, float(index) * 2.21341f);
    Output[index].translate.y += random(gEmitter[particle.emitterNum].area.range.min.y, gEmitter[particle.emitterNum].area.range.max.y, float(index) * 3.4214f);
    Output[index].translate.z += random(gEmitter[particle.emitterNum].area.range.min.z, gEmitter[particle.emitterNum].area.range.max.z, float(index) * 4.2108124f);
    
}

void Velocity(uint index, CreateParticle particle)
{
    Output[index].velocity.x = random(gEmitter[particle.emitterNum].velocity3D.range.min.x, gEmitter[particle.emitterNum].velocity3D.range.max.x, float(index) * 1.1423589f);
    Output[index].velocity.y = random(gEmitter[particle.emitterNum].velocity3D.range.min.y, gEmitter[particle.emitterNum].velocity3D.range.max.y, float(index) * 1.19786457f);
    Output[index].velocity.z = random(gEmitter[particle.emitterNum].velocity3D.range.min.z, gEmitter[particle.emitterNum].velocity3D.range.max.z, float(index) * 1.36266f);
}

void Color(uint index, CreateParticle particle)
{
    Output[index].color.r = random(gEmitter[particle.emitterNum].color.range.start.min.r, gEmitter[particle.emitterNum].color.range.start.min.r, float(index) * 1.2142f);
    Output[index].color.g = random(gEmitter[particle.emitterNum].color.range.start.min.g, gEmitter[particle.emitterNum].color.range.start.min.g, float(index) * 3.14531215f);
    Output[index].color.b = random(gEmitter[particle.emitterNum].color.range.start.min.b, gEmitter[particle.emitterNum].color.range.start.min.b, float(index) * 2.124173180f);
    Output[index].color.a = random(gEmitter[particle.emitterNum].color.range.start.min.a, gEmitter[particle.emitterNum].color.range.start.min.a, float(index) * 1.1238102f);
}

void Create(uint index, CreateParticle particle)
{
    
    LifeTime(index, particle);
    
    Scale(index, particle);
    
    Rotate(index, particle);
    
    Translate(index, particle);
    
    Velocity(index, particle);
    
    Color(index, particle);
    
    Output[index].textureInidex = gEmitter[particle.emitterNum].textureIndex;
    
    Output[index].isAlive = true;
}

[numthreads(threadBlockSize, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint index = particleIndexCommands.Consume();
    CreateParticle particle;
    for (int i = 0; i < emitterSize; i++)
    {
        // ここで同期処理をしたい
        // 並列処理で複数のスレットがcreateParticle[i].createParticleNumの計算を行っていて
        // 現在のcreateParticle[i].createParticleNum の中身を知りたい
        // createParticle[i].createParticleNum の値を安全に読み取る
        InterlockedAdd(createParticle[i].createParticleNum, -1);
        if (createParticle[i].createParticleNum > 0)
        {
            particle = createParticle[i];
            break;
        }
    }
    Create(index, particle);
}
