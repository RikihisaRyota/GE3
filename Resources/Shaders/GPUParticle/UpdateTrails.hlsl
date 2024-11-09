#include "GPUParticleShaderStructs.h"

AppendStructuredBuffer<int> trailsStock : register(u0);
AppendStructuredBuffer<GPUParticleShaderStructs::TrailsIndex> trailsCounter : register(u1);
RWStructuredBuffer<GPUParticleShaderStructs::TrailsData> trailsData : register(u2);
RWStructuredBuffer<GPUParticleShaderStructs::TrailsPosition> trailsPosition : register(u3);
StructuredBuffer<GPUParticleShaderStructs::Particle> particles : register(t0);

struct ViewProjection
{
    float32_t4x4 view; // ビュー変換行列
    float32_t4x4 projection; // プロジェクション変換行列
    float32_t4x4 inverseView;
    float32_t3 cameraPos; // カメラのワールド座標
    float32_t pad;
};
ConstantBuffer<ViewProjection> gViewProjection : register(b0);

[numthreads(GPUParticleShaderStructs::ComputeThreadBlockSize, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    int32_t trailsIndex = DTid.x;
    GPUParticleShaderStructs::TrailsData data = trailsData[trailsIndex];
    if(data.isAlive){
        GPUParticleShaderStructs::Particle particle = particles[data.particleIndex];
        // パーティクルが生きているか
        if(particle.isAlive){
            // パーティクルの位置
            float32_t3 particlePosition = particle.matWorld[3].xyz;
            // ビルボード計算
            float32_t3 cameraDir = normalize(gViewProjection.cameraPos - particlePosition);
            float32_t3 rightDir = normalize(cross(cameraDir, float32_t3(0.0f, 1.0f, 0.0f)));  
            float32_t3 upDir = cross(rightDir, cameraDir);  
            if(data.time >= data.interval) {        
                GPUParticleShaderStructs::TrailsPosition pos;
                pos.position = particlePosition;
                pos.vertex[0].position = particlePosition + rightDir * data.width;
                pos.vertex[0].uv = float32_t2(0.0f,0.0f);
                pos.vertex[1].position = particlePosition - rightDir * data.width;
                pos.vertex[1].uv = float32_t2(0.0f,0.0f);
                trailsPosition[data.currentIndex] = pos;
                
                data.currentIndex++;
                if(data.currentIndex >= data.endIndex){
                    data.currentIndex = data.startIndex;
                }
                data.time = 0;
            } else {
                data.time++;
            }
            GPUParticleShaderStructs::TrailsIndex index;
            // 生きているindexを格納
            for(int i = data.startIndex;i < data.currentIndex;i++){
                // ビルボード再計算
                GPUParticleShaderStructs::TrailsPosition pos = trailsPosition[i];
                // パーティクルの位置
                float32_t3 position = trailsPosition[i].position;
                // ビルボード計算
                float32_t3 cameraDir = normalize(gViewProjection.cameraPos - position);
                float32_t3 rightDir = normalize(cross(cameraDir, float32_t3(0.0f, 1.0f, 0.0f)));  
                float32_t3 upDir = cross(rightDir, cameraDir);

                pos.vertex[0].position = pos.position + rightDir * data.width;
                pos.vertex[0].uv = float32_t2(0.0f,0.0f);
                pos.vertex[1].position = pos.position - rightDir * data.width;
                pos.vertex[1].uv = float32_t2(0.0f,0.0f);
                trailsPosition[i] = pos;
                index.positionIndex = i;
                index.trailsIndex = data.trailsIndex;
                trailsCounter.Append(index);
            }
        }else{
                // 死んでいたらindexを返し初期化
                trailsStock.Append(data.trailsIndex);
                data.trailsIndex = -1;
                data.isAlive = 0;
                for(uint32_t i = data.startIndex; i < data.endIndex;i++){
                    trailsPosition[i].position = float32_t3(0.0f,0.0f,0.0f);
                    //trailsPosition[i].texcoord = float32_t2(0.0f,0.0f);
                }
        }
        trailsData[trailsIndex] = data;
    }
}