#include "GPUParticle.hlsli"

struct Bullet{
        float32_t3 position;
		float32_t radius;
		float32_t speed;
		float32_t3 pad;
};

StructuredBuffer<Bullet> bullets : register(t0);

RWStructuredBuffer<Particle> particle : register(u0);

struct BulletCount{
    uint32_t count;
};

ConstantBuffer<BulletCount> bulletCount: register(b0);

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
    if(particle[index].isAlive){

    Circle a,b;
    a.position=particle[index].translate;
    a.radius=particle[index].scale.x;
    for(uint32_t i=0;i<bulletCount.count;++i){
        b.position=bullets[i].position;
        b.radius=bullets[i].radius;
        if(Collision(a,b)){
            float32_t3 v =particle[index].translate- bullets[i].position;
            particle[index].velocity = normalize(v)* bullets[i].speed;
            particle[index].particleLifeTime.time=0;
            particle[index].particleLifeTime.maxTime=360;
        }
    }
    }
}