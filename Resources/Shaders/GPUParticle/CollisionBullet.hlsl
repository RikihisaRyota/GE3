#include "GPUParticle.hlsli"

struct BulletEmitter{
        ParticleAttributes collisionInfo;
        struct Bullet{
        float32_t3 position;
		float32_t radius;
		float32_t speed;
		float32_t3 pad;
        }bullet;

        struct Emitter{
            ParticleLifeSpan particleLifeSpan;
        }emitter;
};

StructuredBuffer<BulletEmitter> bullets : register(t0);

RWStructuredBuffer<Particle> particle : register(u0);

struct BulletCount{
    uint32_t count;
};

ConstantBuffer<BulletCount> bulletCount: register(b0);

struct Random
{
    uint32_t random;
};


ConstantBuffer<Random> gRandom : register(b1);

struct Circle {
    float32_t3 position;
    float32_t  radius;

};

bool Collision(Circle circleA,  Circle circleB) {
    // 2つの中心の間の距離を計算
    float32_t dx = circleB.position.x - circleA.position.x;
    float32_t dy = circleB.position.y - circleA.position.y;
    float32_t dz = circleB.position.z - circleA.position.z;
    float32_t distance = sqrt(dx * dx + dy * dy + dz * dz);
    
    // 半径の合計
    float32_t radiusSum = circleA.radius + circleB.radius;

    // 衝突判定
    return distance <= radiusSum;
}

[numthreads(threadBlockSize, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    uint32_t index=DTid.x;
    if(particle[index].isAlive&&!particle[index].isHit){
        uint32_t seed = setSeed(index * gRandom.random);
        Circle a,b;
        a.position=particle[index].worldMatrix[3].xyz;
        a.radius=particle[index].scale.x;
        for(uint32_t i=0;i<bulletCount.count;++i){
            if ((particle[index].collisionInfo.mask & bullets[i].collisionInfo.attribute) != 0 &&
                (bullets[i].collisionInfo.mask & particle[index].collisionInfo.attribute) != 0){
                b.position=bullets[i].bullet.position;
                b.radius=bullets[i].bullet.radius;
                if(Collision(a,b)){
                    float32_t3 v =particle[index].translate.translate - bullets[i].bullet.position;
                    particle[index].velocity = normalize(v) * bullets[i].bullet.speed;
                    particle[index].translate.translate += normalize(v) * particle[index].translate.radius;
                    particle[index].particleLifeTime.time = 0;
                    //particle[index].particleLifeTime.maxTime= randomRange(bullets[i].emitter.particleLifeSpan.range.min, bullets[i].emitter.particleLifeSpan.range.max, seed);
                    //particle[index].particleLifeTime.isEmitterLife = false;
                    //particle[index].translate.isEasing=false;
                    //particle[index].isHit=true;
                }
            }
        }
    }
}