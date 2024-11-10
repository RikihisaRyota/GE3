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
            //uint32_t loopNum = 0;
            //// パーティクルの位置
            //float32_t3 particlePosition = particle.matWorld[3].xyz;
            //// ビルボード計算
            //float32_t3 cameraDir = normalize(gViewProjection.cameraPos - particlePosition);
            //float32_t3 rightDir = normalize(cross(cameraDir, float32_t3(0.0f, 1.0f, 0.0f)));  
            //float32_t3 upDir = cross(rightDir, cameraDir);  
            //if(data.time >= data.interval) {        
            //    //GPUParticleShaderStructs::TrailsPosition pos;
            //    //pos.position = particlePosition;
            //    //pos.vertex[0].position = particlePosition + rightDir * data.width;
            //    //pos.vertex[0].uv = float32_t2(0.0f,0.0f);
            //    //pos.vertex[1].position = particlePosition - rightDir * data.width;
            //    //pos.vertex[1].uv = float32_t2(1.0f,0.0f);
            //    //trailsPosition[data.currentIndex] = pos;
            //    //
            //    //data.currentIndex++;
            //    //if(data.currentIndex >= data.endIndex){
            //    //    data.currentIndex = data.startIndex;
            //    //    loopNum++;
            //    //}
            //    //data.time = 0;
            //} else {
            //    //data.time++;
            //}
//
        //    // lifeLimit が 0 以下の場合はスキップ
        //    if (data.lifeLimit <= 0.0f) {
        //        return;
        //    }
        //    //// 現在の長さの合計からの長さ
        //    //int32_t visibleLength = int32_t((data.currentIndex + loopNum * GPUParticleShaderStructs::TrailsRange) * data.lifeLimit);
        //    //int32_t maxVisibleLength = visibleLength;
        //    //// トレイルの表示範囲に基づいて処理
        //    //int32_t i = data.currentIndex;
        //    //while (visibleLength >= 0) {
        //    //    // ビルボード再計算
        //    //    GPUParticleShaderStructs::TrailsPosition pos = trailsPosition[i];
        //    //    float32_t3 position = pos.position;
////
        //    //    // ビルボード計算
        //    //    float32_t3 cameraDir = normalize(gViewProjection.cameraPos - position);
        //    //    float32_t3 rightDir = normalize(cross(cameraDir, float32_t3(0.0f, 1.0f, 0.0f)));
        //    //    float32_t3 upDir = cross(rightDir, cameraDir);
////
        //    //    // UV の補間値 t の計算0からスタート
        //    //    // normalizedIndex の計算が 0 ～ 1 の範囲に収まるようにする
        //    //    float32_t normalizedIndex = float32_t(visibleLength) / float32_t(maxVisibleLength);
        //    //    normalizedIndex = clamp(normalizedIndex, 0.0f, 1.0f);  // 0 ～ 1 に制限
        //    //
        //    //    // UV の補間値 t の計算（0.0f ～ 1.0f に補間）
        //    //    float32_t t = lerp(0.0f, 1.0f, normalizedIndex);  // 0.0f が先、1.0f が後ろに進む方向
////
        //    //    // トレイルの頂点の位置と UV を設定
        //    //    pos.vertex[0].position = pos.position + rightDir * data.width;
        //    //    pos.vertex[0].uv = float32_t2(0.0f, t);
        //    //    pos.vertex[1].position = pos.position - rightDir * data.width;
        //    //    pos.vertex[1].uv = float32_t2(1.0f, t);
////
        //    //    // 更新したトレイルの位置を保存
        //    //    trailsPosition[i] = pos;
////
        //    //    // インデックス情報を更新
        //    //    GPUParticleShaderStructs::TrailsIndex index = (GPUParticleShaderStructs::TrailsIndex)0;
        //    //    index.positionIndex = i;
        //    //    index.trailsIndex = data.trailsIndex;
        //    //    trailsCounter.Append(index);
////
        //    //    // インデックスを逆順で進める（ラップ処理も含む）
        //    //    visibleLength--;
        //    //    i--;
        //    //    if (i < data.startIndex) {
        //    //        i = data.endIndex - 1;
        //    //    }
        //    //}
        } else {
        //    // 死んでいたら index を返し初期化
        //    trailsStock.Append(data.trailsIndex);
        //    data.trailsIndex = -1;
        //    data.isAlive = 0;
        //    GPUParticleShaderStructs::TrailsPosition pos = (GPUParticleShaderStructs::TrailsPosition)0;
        //    for(uint32_t i = data.startIndex; i < data.endIndex; i++){
        //        trailsPosition[i] = pos;
        //    }
        }
        trailsData[trailsIndex] = data;
    }
}
