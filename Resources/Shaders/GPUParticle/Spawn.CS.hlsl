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

void LifeTime(uint index, uint32_t emitterIndex,inout uint32_t seed)
{
    
    Output[index].particleLifeTime.maxTime = randomRange(gEmitter[emitterIndex].particleLifeSpan.range.min, gEmitter[emitterIndex].particleLifeSpan.range.max, seed);
    Output[index].particleLifeTime.time = 0;
}

void Scale(uint index,  uint32_t emitterIndex,inout uint32_t seed)
{
    Output[index].scaleRange.min.x = randomRange(gEmitter[emitterIndex].scale.range.start.min.x, gEmitter[emitterIndex].scale.range.start.max.x,seed);
    Output[index].scaleRange.min.y = randomRange(gEmitter[emitterIndex].scale.range.start.min.y, gEmitter[emitterIndex].scale.range.start.max.y,seed);
    Output[index].scaleRange.min.z = randomRange(gEmitter[emitterIndex].scale.range.start.min.z, gEmitter[emitterIndex].scale.range.start.max.z,seed);
    
    Output[index].scaleRange.max.x = randomRange(gEmitter[emitterIndex].scale.range.end.min.x, gEmitter[emitterIndex].scale.range.end.max.x,seed);
    Output[index].scaleRange.max.y = randomRange(gEmitter[emitterIndex].scale.range.end.min.y, gEmitter[emitterIndex].scale.range.end.max.y,seed);
    Output[index].scaleRange.max.z = randomRange(gEmitter[emitterIndex].scale.range.end.min.z, gEmitter[emitterIndex].scale.range.end.max.z,seed);
    
    Output[index].scale = Output[index].scaleRange.min;
}

void Rotate(uint index, uint32_t emitterIndex,inout uint32_t seed)
{
    Output[index].rotateVelocity = gEmitter[emitterIndex].rotateAnimation.rotate;
}

void Translate(uint index,  uint32_t emitterIndex,inout uint32_t seed)
{
    Output[index].translate = gEmitter[emitterIndex].area.position;
    if(gEmitter[emitterIndex].area.type==0){
        Output[index].translate.x += randomRange(gEmitter[emitterIndex].area.aabb.range.min.x, gEmitter[emitterIndex].area.aabb.range.max.x, seed);
        Output[index].translate.y += randomRange(gEmitter[emitterIndex].area.aabb.range.min.y, gEmitter[emitterIndex].area.aabb.range.max.y, seed);
        Output[index].translate.z += randomRange(gEmitter[emitterIndex].area.aabb.range.min.z, gEmitter[emitterIndex].area.aabb.range.max.z, seed);
    }else if(gEmitter[emitterIndex].area.type==1){
        float32_t3 normal,direction;
        normal.x = randomRange(-gEmitter[emitterIndex].area.sphere.radius, gEmitter[emitterIndex].area.sphere.radius,seed);
        normal.y = randomRange(-gEmitter[emitterIndex].area.sphere.radius, gEmitter[emitterIndex].area.sphere.radius,seed);
        normal.z = randomRange(-gEmitter[emitterIndex].area.sphere.radius, gEmitter[emitterIndex].area.sphere.radius,seed);
        direction=normalize(normal);
        direction*= randomRange(-gEmitter[emitterIndex].area.sphere.radius, gEmitter[emitterIndex].area.sphere.radius, seed);
        Output[index].translate += direction;
    }
    
}

void Velocity(uint index,  uint32_t emitterIndex,inout uint32_t seed)
{
    Output[index].velocity.x = randomRange(gEmitter[emitterIndex].velocity3D.range.min.x, gEmitter[emitterIndex].velocity3D.range.max.x, seed);
    Output[index].velocity.y = randomRange(gEmitter[emitterIndex].velocity3D.range.min.y, gEmitter[emitterIndex].velocity3D.range.max.y, seed);
    Output[index].velocity.z = randomRange(gEmitter[emitterIndex].velocity3D.range.min.z, gEmitter[emitterIndex].velocity3D.range.max.z, seed);
}

void Color(uint index,  uint32_t emitterIndex,inout uint32_t seed)
{
    Output[index].color.r = randomRange(gEmitter[emitterIndex].color.range.start.min.r, gEmitter[emitterIndex].color.range.start.min.r, seed);
    Output[index].color.g = randomRange(gEmitter[emitterIndex].color.range.start.min.g, gEmitter[emitterIndex].color.range.start.min.g,seed);
    Output[index].color.b = randomRange(gEmitter[emitterIndex].color.range.start.min.b, gEmitter[emitterIndex].color.range.start.min.b, seed);
    Output[index].color.a = randomRange(gEmitter[emitterIndex].color.range.start.min.a, gEmitter[emitterIndex].color.range.start.min.a,seed);
}

void Create(uint index,  uint32_t emitterIndex,uint32_t seed)
{
    Output[index].textureInidex = gEmitter[emitterIndex].textureIndex;
    
    Output[index].isAlive = 1;
    
    LifeTime(index, emitterIndex,seed);
    
    Scale(index, emitterIndex,seed);
    
    Rotate(index, emitterIndex,seed);
    
    Translate(index, emitterIndex,seed);
    
    Velocity(index, emitterIndex,seed);
    
    Color(index, emitterIndex,seed);   
}


[numthreads(1, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    for (int i = 0; i < emitterSize; i++)
    {
        if(createParticle[i].createParticleNum > 0){

        // ここで同期処理をしたい
        // 並列処理で複数のスレットがcreateParticle[i].createParticleNumの計算を行っていて
        // 現在のcreateParticle[i].createParticleNum の中身を知りたい
        // createParticle[i].createParticleNum の値を安全に読み取る
        int32_t createNum; 
        InterlockedAdd(createParticle[i].createParticleNum, -1,createNum);
        if (createNum > 0)
        {
                int index = particleIndexCommands.Consume();
                uint32_t seed = setSeed(index * 21412);
                uint32_t emitterIndex = createParticle[i].emitterNum;
                Create(index, emitterIndex,seed);
                Output[index].isAlive = 1;
                break;
        }
        }
    }
}