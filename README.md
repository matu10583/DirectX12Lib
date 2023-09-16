# 実装した機能

- Dx12のオブジェクトのカプセル化
- シェーダーリフレクションからリソースを自動的に生成、それらを描画時に自動的にバインド
- リソースに対して名前の指定だけで値を設定する。
- 任意の構造体をVertexVufferとして生成。試してはいないけどインスタンシングでも機能するはず
- StructuredBufferの作成とMeshShaderの実行

# 実装したい機能

- 画像読み込みのマルチスレッド化
- RenderComponentのECS化
- マルチパスレンダリング
    - 試してないけど多分できる
- fbx読み込み機能

# 使ってる外部の機能

## DirectXTex

- テクスチャのミップマップ作製とかに使ってる

# stb

- pngの読み込み