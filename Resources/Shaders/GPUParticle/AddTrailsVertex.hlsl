#include "GPUParticleShaderStructs.h"

AppendStructuredBuffer<GPUParticleShaderStructs::TrailsVertexData> trailsVertexData : register(u0);
StructuredBuffer<GPUParticleShaderStructs::TrailsData> trailsData : register(t0);
StructuredBuffer<GPUParticleShaderStructs::TrailsPosition> trailsPosition : register(t1);

[numthreads(GPUParticleShaderStructs::ComputeThreadBlockSize, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    int32_t trailsIndex = DTid.x;
    GPUParticleShaderStructs::TrailsData data = trailsData[trailsIndex];
    if (data.isAlive) {
        GPUParticleShaderStructs::TrailsPosition posData0, posData1;
        GPUParticleShaderStructs::TrailsVertexData vertexData;
            // 現在の長さの合計からの長さ
            int32_t visibleLength = data.currentIndex - data.startIndex;
            if(data.loopNum > 0){
                visibleLength = GPUParticleShaderStructs::TrailsRange;
            }
            visibleLength = int32_t(visibleLength * data.lifeLimit);
            int32_t i = data.currentIndex - 1;
            while(visibleLength >= 0){
                posData0 = trailsPosition[i];
                int32_t nextIndex = i - 1;
                // ループしていなかったら
                if (nextIndex < int32_t(data.startIndex)) {
                    nextIndex = i;
                }
                posData1 = trailsPosition[nextIndex];
    
                vertexData.vertex[0] = posData0.vertex[0];
                vertexData.vertex[1] = posData0.vertex[1];
                vertexData.vertex[2] = posData1.vertex[0];
    
                trailsVertexData.Append(vertexData);
    
                vertexData.vertex[0] = posData0.vertex[1];
                vertexData.vertex[1] = posData1.vertex[0];
                vertexData.vertex[2] = posData1.vertex[1];
    
                trailsVertexData.Append(vertexData);
                // インデックスを逆順で進める（ラップ処理も含む）
                visibleLength--;
                i--;
                if (i < int32_t(data.startIndex) && data.loopNum > 0) {
                    i = data.endIndex - 1;
                }
            }
    }
}
