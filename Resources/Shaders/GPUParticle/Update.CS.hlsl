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

StructuredBuffer<Emitter> emitter : register(t0);
StructuredBuffer<MeshEmitter> meshEmitter : register(t1);
StructuredBuffer<TransformEmitter> transformEmitter : register(t2);

[numthreads(threadBlockSize, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    for(uint32_t i=0;i<processNum ;i++){
        uint index = DTid.x * processNum + i;
        if(index > maxParticleNum){
            return;
        }
        if(input[index].isAlive)
        {
            float t = float(input[index].particleLifeTime.time) / float(input[index].particleLifeTime.maxTime);
    
            // 移動
            if(!input[index].translate.isEasing){
                input[index].translate.translate += input[index].velocity;
            }else{
                input[index].translate.translate = lerp(input[index].translate.easing.min, input[index].translate.easing.max, t);
            }
    
            float32_t4x4 translateMatrix = MakeTranslationMatrix(input[index].translate.translate);
    
            // スケール
            input[index].scale = lerp(input[index].scaleRange.min, input[index].scaleRange.max, t);
            float32_t4x4 scaleMatrix = MakeScaleMatrix(input[index].scale);
            
            // 回転
            input[index].rotate += input[index].rotateVelocity;
            float32_t4x4 particleRotate = MakeRotationMatrixZ(input[index].rotate);
            
            // 親の移動を含めた最終的な位置を計算
            float32_t3 finalPosition;
            float32_t4x4 parentWorldMatrix;
            if(input[index].parent.isParent) {
                // 親のワールド行列を取得
                if(input[index].parent.emitterType==0){
                    parentWorldMatrix = emitter[input[index].parent.emitterCount].parent.worldMatrix;
                } else if(input[index].parent.emitterType==1){
                    //parentWorldMatrix = meshEmitter[input[index].parent.emitterCount].parent.worldMatrix;
                }else {
                    //parentWorldMatrix = transformEmitter[input[index].parent.emitterCount].parent.worldMatrix;
                }
                float32_t4 parentTranslation = mul(parentWorldMatrix, float32_t4(input[index].translate.translate, 1.0f));
                finalPosition = parentTranslation.xyz;
            } else {
                finalPosition = input[index].translate.translate;
            }

            // カメラの位置と向きを取得する
            float32_t3 cameraPos = gViewProjection.cameraPos;
            float32_t3 cameraDir = normalize(finalPosition - cameraPos);
    
            // カメラの方向を基にビルボードの回転行列を計算する
            float32_t3 upVector = float32_t3(0.0f, 1.0f, 0.0f);
            float32_t3 sideVector = cross(upVector, cameraDir); // カメラの方向と上方向の外積を取る
            float32_t3 newUpVector = cross(cameraDir, sideVector); // カメラの方向と側方向の外積を取る
    
            float32_t4x4 billboardMatrix =
            {
                float32_t4(sideVector, 0.0f),
                float32_t4(newUpVector, 0.0f),
                float32_t4(cameraDir, 0.0f),
                float32_t4(0.0f, 0.0f, 0.0f, 1.0f)
            };

            float32_t4x4 finalRotateMatrix;
            if(input[index].parent.isParent){
                // 親の回転行列（移動を含まない）
                float32_t4x4 parentRotateMatrix = float32_t4x4(
                    parentWorldMatrix[0], parentWorldMatrix[1], parentWorldMatrix[2], float32_t4(0, 0, 0, 1)
                );
                finalRotateMatrix = mul(particleRotate, parentRotateMatrix);
            } else {
                finalRotateMatrix = particleRotate;
            }
            float32_t4x4 rotateMatrix = mul(finalRotateMatrix, billboardMatrix); // パーティクルの回転を適用
    
            input[index].color = lerp(input[index].colorRange.min, input[index].colorRange.max, t);
    
            if(input[index].particleLifeTime.time >= input[index].particleLifeTime.maxTime)
            {
                input[index].isAlive = false;
                particleIndexCommands.Append(index);
            }
            else
            {
                if(input[index].parent.isParent){
                    input[index].worldMatrix = mul(mul(mul(scaleMatrix, rotateMatrix), translateMatrix), parentWorldMatrix);
                }
                else{
                    input[index].worldMatrix = mul(mul(scaleMatrix, rotateMatrix), translateMatrix);
                }
                outputDrawIndexCommands.Append(index);
            }
            input[index].particleLifeTime.time++;
        }
    }
}
