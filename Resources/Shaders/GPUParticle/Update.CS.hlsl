#include "GPUParticle.hlsli"
struct ViewProjection
{
    float32_t4x4 view; // ビュー変換行列
    float32_t4x4 projection; // プロジェクション変換行列
    float32_t4x4 inverseView;
    float32_t3 cameraPos; // カメラのワールド座標
    float32_t pad;
};
ConstantBuffer<ViewProjection> gViewProjection : register(b0);

RWStructuredBuffer<Particle> input : register(u0);

AppendStructuredBuffer<uint> particleIndexCommands : register(u1);

AppendStructuredBuffer<uint> outputDrawIndexCommands : register(u2);

[numthreads(threadBlockSize, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    //uint index = DTid.x ;
    for(uint32_t i=0;i<processNum ;i++){
    uint index = DTid.x * processNum + i;
    if( index > maxParticleNum){
        return;
    }
    if (input[index].isAlive)
    {
        float t = float(input[index].particleLifeTime.time) / float(input[index].particleLifeTime.maxTime);

        // 移動
        //float gravity=0.01f;
        //input[index].velocity.y-=gravity;
        input[index].translate += input[index].velocity;

        float32_t4x4 translateMatrix=MakeTranslationMatrix(input[index].translate);

        // スケール
        input[index].scale = lerp(input[index].scaleRange.min, input[index].scaleRange.max, t);
        
        float32_t4x4 scaleMatrix=MakeScaleMatrix(input[index].scale );
        
        // 回転
        // カメラの位置と向きを取得する
        float32_t3 cameraPos = gViewProjection.cameraPos;
        float32_t3 cameraDir = normalize(input[index].translate-cameraPos);

        // カメラの方向を基にビルボードの回転行列を計算する
        float32_t3 upVector = float32_t3(0.0f, 1.0f, 0.0f);
        float32_t3 sideVector = cross(upVector,cameraDir); // カメラの方向と上方向の外積を取る
        float32_t3 newUpVector = cross(cameraDir,sideVector); // カメラの方向と側方向の外積を取る

        float32_t4x4 billbordMatrix =
        {
            float32_t4(sideVector, 0.0f),
            float32_t4(newUpVector, 0.0f),
            float32_t4(cameraDir, 0.0f),
            float32_t4(0.0f, 0.0f, 0.0f, 1.0f)
        };

        input[index].rotate += input[index].rotateVelocity;
        
        float32_t4x4 particleRotate = MakeRotationMatrixZ(input[index].rotate);

        float32_t4x4 rotateMatrix=mul(particleRotate,billbordMatrix); 

        input[index].color = lerp(input[index].colorRange.min, input[index].colorRange.max, t);

        if (input[index].particleLifeTime.time >= input[index].particleLifeTime.maxTime)
        {
            input[index].isAlive = false;
            particleIndexCommands.Append(index);
        }else{
            input[index].worldMatrix=mul(mul(scaleMatrix,rotateMatrix),translateMatrix);
            outputDrawIndexCommands.Append(index);
        }
        input[index].particleLifeTime.time++;
    }
}
}