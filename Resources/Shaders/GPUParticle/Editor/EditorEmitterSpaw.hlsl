#include "../GPUParticle.hlsli"

ConstantBuffer<Emitter> gEmitter : register(b0);

RWStructuredBuffer<Particle> Output : register(u0);

ConsumeStructuredBuffer<uint> particleIndexCommands : register(u1);

struct Random
{
    uint32_t random;
};


ConstantBuffer<Random> gRandom : register(b0);

ConsumeStructuredBuffer<uint> particleIndexCommands : register(u1);

RWStructuredBuffer<CreateParticle> createParticle : register(u2);

struct CounterParticle
{
    int32_t count;
};


RWStructuredBuffer<CounterParticle> particleIndexCounter : register(u3);


void LifeTime(uint index,inout uint32_t seed)
{
   Output[index].particleLifeTime.maxTime = randomRange(gEmitter.particleLifeSpan.range.min, gEmitter.particleLifeSpan.range.max, seed);
    Output[index].particleLifeTime.time = 0;
}

void Scale(uint index,inout uint32_t seed)
{
    Output[index].scaleRange.min.x = randomRange(gEmitter.scale.range.start.min.x, gEmitter.scale.range.start.max.x,seed);
    Output[index].scaleRange.min.y = randomRange(gEmitter.scale.range.start.min.y, gEmitter.scale.range.start.max.y,seed);
    Output[index].scaleRange.min.z = randomRange(gEmitter.scale.range.start.min.z, gEmitter.scale.range.start.max.z,seed);
    
    Output[index].scaleRange.max.x = randomRange(gEmitter.scale.range.end.min.x, gEmitter.scale.range.end.max.x,seed);
    Output[index].scaleRange.max.y = randomRange(gEmitter.scale.range.end.min.y, gEmitter.scale.range.end.max.y,seed);
    Output[index].scaleRange.max.z = randomRange(gEmitter.scale.range.end.min.z, gEmitter.scale.range.end.max.z,seed);
    
    Output[index].scale = Output[index].scaleRange.min;
}

void Rotate(uint index,inout uint32_t seed)
{
    Output[index].rotateVelocity = gEmitter.rotateAnimation.rotate;
    
}

void Translate(uint index,inout uint32_t seed)
{
    if(gEmitter.area.type==0){
        Output[index].translate = gEmitter.area.position;
        Output[index].translate += randomRange(gEmitter.area.aabb.range.min, gEmitter.area.aabb.range.max, seed);
    }else if(gEmitter.area.type==1){
        Output[index].translate = gEmitter.area.position;
        float32_t3 normal,direction;
        normal.x = randomRange(-1.0f, 1.0f,seed);
        normal.y = randomRange(-1.0f, 1.0f,seed);
        normal.z = randomRange(-1.0f, 1.0f,seed);
        direction=normalize(normal);
        direction*= randomRange(0.0f, gEmitter.area.sphere.radius, seed);
        Output[index].translate += direction;
    } else if(gEmitter.area.type==2){
            float32_t3 normal,direction,p;
            p = randomRangeSame(gEmitter.area.capsule.segment.origin, gEmitter.area.capsule.segment.diff,seed);
            normal.x = randomRange(-1.0f, 1.0f,seed);
            normal.y = randomRange(-1.0f, 1.0f,seed);
            normal.z = randomRange(-1.0f, 1.0f,seed);
            direction=normalize(normal);
            direction*= randomRange(0.0f, gEmitter.area.capsule.radius, seed);
            Output[index].translate =  pointOnCapsule(p + direction,gEmitter.area.capsule.segment.origin,gEmitter.area.capsule.segment.diff,gEmitter.area.capsule.radius ,randomRange(-1.0f, 0.0f,seed));
    }
}

void Velocity(uint index,inout uint32_t seed)
{
     Output[index].velocity = randomRange(gEmitter.velocity3D.range.min, gEmitter.velocity3D.range.max, seed);
}

void Color(uint index,inout uint32_t seed)
{
    Output[index].colorRange.min=randomRange(gEmitter.color.range.start.min, gEmitter.color.range.start.max, seed);
    Output[index].colorRange.max=randomRange(gEmitter.color.range.end.min, gEmitter.color.range.end.max, seed);
    Output[index].color =Output[index].colorRange.min;
}

void Create(uint index,inout uint32_t seed)
{
    Output[index].textureIndex = gEmitter[emitterIndex].textureIndex;
    
    Output[index].isAlive = 1;
    
    LifeTime(index, emitterIndex,seed);
    
    Scale(index, emitterIndex,seed);
    
    Rotate(index, emitterIndex,seed);
    
    Translate(index, emitterIndex,seed);
    
    Velocity(index, emitterIndex,seed);
    
    Color(index, emitterIndex,seed);   
}

[numthreads(threadBlockSize, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    int32_t createNum=-1; 
    InterlockedAdd(createParticle[i].createParticleNum, -1,createNum);
          
    if (createNum > 0)
    { 
        int32_t counter=-1;
        InterlockedAdd(particleIndexCounter[0].count, -1,counter);
        if(counter>0){
            int index = particleIndexCommands.Consume();
            uint32_t seed = setSeed(index * gRandom.random);
            uint32_t emitterIndex = createParticle[i].emitterNum;
            Create(index, emitterIndex,seed);
            break;
        }
    }
}