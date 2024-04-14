#include "../GPUParticle.hlsli"

ConstantBuffer<Emitter> gEmitter : register(b0);

RWStructuredBuffer<Particle> Output : register(u0);

ConsumeStructuredBuffer<uint> particleIndexCommands : register(u1);

void LifeTime(uint index)
{
    Output[index].particleLifeTime.maxTime = random(gEmitter.particleLifeSpan.range.min, gEmitter.particleLifeSpan.range.max, float(index) * 1414531.0f);
    Output[index].particleLifeTime.time = 0;
}

void Scale(uint index)
{
    Output[index].scaleRange.min.x = random(gEmitter.scale.range.start.min.x, gEmitter.scale.range.start.max.x, float(index) * 1.2136912f);
    Output[index].scaleRange.min.y = random(gEmitter.scale.range.start.min.y, gEmitter.scale.range.start.max.y, float(index) * 1.1749621f);
    Output[index].scaleRange.min.z = random(gEmitter.scale.range.start.min.z, gEmitter.scale.range.start.max.z, float(index) * 1.24412f);
    
    Output[index].scaleRange.max.x = random(gEmitter.scale.range.end.min.x, gEmitter.scale.range.end.max.x, float(index) * 1.42356f);
    Output[index].scaleRange.max.y = random(gEmitter.scale.range.end.min.y, gEmitter.scale.range.end.max.y, float(index) * 1.3247f);
    Output[index].scaleRange.max.z = random(gEmitter.scale.range.end.min.z, gEmitter.scale.range.end.max.z, float(index) * 1.257212f);
    
    Output[index].scale = Output[index].scaleRange.min;
}

void Rotate(uint index)
{
    Output[index].rotateVelocity = gEmitter.rotateAnimation.rotate;
    
}

void Translate(uint index)
{
    Output[index].translate = gEmitter.area.position;
    Output[index].translate.x += random(gEmitter.area.range.min.x, gEmitter.area.range.max.x, float(index) * 2.21341f);
    Output[index].translate.y += random(gEmitter.area.range.min.y, gEmitter.area.range.max.y, float(index) * 3.4214f);
    Output[index].translate.z += random(gEmitter.area.range.min.z, gEmitter.area.range.max.z, float(index) * 4.2108124f);
    
}

void Velocity(uint index)
{
    Output[index].velocity.x = random(gEmitter.velocity3D.range.min.x, gEmitter.velocity3D.range.max.x, float(index) * 1.1423589f);
    Output[index].velocity.y = random(gEmitter.velocity3D.range.min.y, gEmitter.velocity3D.range.max.y, float(index) * 1.19786457f);
    Output[index].velocity.z = random(gEmitter.velocity3D.range.min.z, gEmitter.velocity3D.range.max.z, float(index) * 1.36266f);
}

void Color(uint index)
{
    Output[index].color.r = random(gEmitter.color.range.start.min.r, gEmitter.color.range.start.min.r, float(index) * 1.2142f);
    Output[index].color.g = random(gEmitter.color.range.start.min.g, gEmitter.color.range.start.min.g, float(index) * 3.14531215f);
    Output[index].color.b = random(gEmitter.color.range.start.min.b, gEmitter.color.range.start.min.b, float(index) * 2.124173180f);
    Output[index].color.a = random(gEmitter.color.range.start.min.a, gEmitter.color.range.start.min.a, float(index) * 1.1238102f);
}

void Create(uint index)
{
    LifeTime(index);
    
    Scale(index);
    
    Rotate(index);
    
    Translate(index);
    
    Velocity(index);
    
    Color(index);
    
    Output[index].textureInidex = gEmitter.textureIndex;
    
    Output[index].isAlive = true;
}

[numthreads(threadBlockSize, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint index = particleIndexCommands.Consume();
    Create(index);
}