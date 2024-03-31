# 実装した機能

- Dx12のオブジェクトのカプセル化
- シェーダーリフレクションからリソースを自動的に生成、それらを描画時に自動的にバインド
- リソースに対して名前の指定だけで値を設定する。
- 任意の構造体をVertexVufferとして生成。試してはいないけどインスタンシングでも機能するはず
- index描画も同様に可能
- StructuredBufferの作成とMeshShaderの実行

# 実装したい機能

- 画像読み込みのマルチスレッド化
- RenderComponentのECS化
- マルチパスレンダリング
    - 試してないけど多分できる
- fbx読み込み機能

# 実行に必要なもの

- /External/DirectXShaderCompiler内にdxcのライブラリを配置
 - [Release](https://github.com/microsoft/DirectXShaderCompiler/releases)(2024_03_22)
- ルートディレクトリにresフォルダを配置（僕のGoogleDriveにsvnがあるよ）