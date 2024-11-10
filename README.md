# Engine
## 説明
このエンジンはDirectX12で自作し、制作人数は1人で開発期間は約２年です。\
GPUParticleについて研究していてComputeShaderを使用しGPU上でSpwn,Update,Drawを行っています。
## 機能
* GPUParticleの実装
* 3Dアニメーションの再生
* mp3、wavの再生
* エンジン側で当たり判定の実装。（AABB,Sphere,OBB,Capsule）
* ライティングの実装
## 注目してほしいところ
GPUParticleに力を入れていてParticleを出せるだけでなくエディターを充実させることで楽にパーティクルを実装できるようにしました。\
### GPUParticle機能紹介
#### エミッター
様々な形状のエミッターでパーティクルを発生できます
* 球\
  ![sphere](https://github.com/user-attachments/assets/554ebff7-1ab8-4159-afbe-5f4cf959dd83)
* AABB\
![aabb](https://github.com/user-attachments/assets/b29d65b1-9d66-4613-aa41-c1546e1fe103)
* Capsule\
  ![capsule](https://github.com/user-attachments/assets/9ed6aee4-e645-444c-860d-2febb512ff54)
* モデルの頂点\
  ![image](https://github.com/user-attachments/assets/526d9122-cffe-4d05-be91-51fcd9f2a134)
* モデルのメッシュ\
![image](https://github.com/user-attachments/assets/1808c076-29bc-4cd2-add1-ad110c2bea45)
その他のパーティクルの速度やスケール寿命などのパラメーターも調整できます。\
![emitter](https://github.com/user-attachments/assets/79097d46-94bd-4aed-84b4-e57afffaa368)
#### フィールド
パーティクルに影響を及ぼすフィールドを設置することができます。\
形状はエミッターと同様、球、AABB、Capsuleがあります。
* 引力
  フィールドの中心にパーティクルを引き込みます。\
  ![innryoku](https://github.com/user-attachments/assets/14b78aba-561e-4b7b-9492-2223e73c49d4)

* 風力
  パーティクルに任意のベクトルの速度を加算します。\
  ![huuryoku](https://github.com/user-attachments/assets/5390a2d9-c24c-4dbf-be01-9b29995032f0)

* 回転（速度）
  パーティクルの速度に対して回転します。\
  ![rotatevel](https://github.com/user-attachments/assets/1da3ceb3-a057-4f9d-9b98-6c713c4cedab)

* 回転（ポジション）
  パーティクルの位置に対して回転します。\
  ![rotatepos](https://github.com/user-attachments/assets/39163b48-9dfe-4126-8676-56ea04980f79)
フィールドとエミッターには属性をつけることができ、それにより影響するエミッターを区別することができます。\
  ![filerd](https://github.com/user-attachments/assets/fd9b863f-32c6-4500-b9e1-e5ca9c765c78)
# GameScene
## 説明
GPUParticleを使用したボスとプレイヤーとの一対一の3Dシューティングゲームです。\
研究しているGPUParticleを大量に使いボスの形態変化や攻撃のパーティクルに注目してください。
![GameScene](https://github.com/user-attachments/assets/62440a37-9832-49e3-b625-39a50e1586b5)
