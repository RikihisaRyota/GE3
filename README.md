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
* gltfを読み込んで描画
## 注目してほしいところ
GPUParticleに力を入れていてParticleを出せるだけでなくエディターを充実させることで楽にパーティクルを実装できるようにしました。\
エディターで変更したデータはjson形式でファルダに保存していてリアルタイムにLoad,Saveすることができます。
![Editor](https://github.com/user-attachments/assets/2d3ba0fc-c68d-4bcb-82a1-96accfd78f4f)
# GameScene
## 説明
GPUParticleを使用したボスとプレイヤーとの一対一の3Dシューティングゲームです。\
研究しているGPUParticleを大量に使いボスの形態変化や攻撃のパーティクルに注目してください。
![GameScene](https://github.com/user-attachments/assets/62440a37-9832-49e3-b625-39a50e1586b5)
