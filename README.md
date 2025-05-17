# GLVolumeRenderer

GLVolumeRendererは、NxNxNの3次元バイナリボリュームデータを可視化するためのレンダリングツールです。


## 特徴
*   **3Dボリュームレンダリング**: NxNxN形式のバイナリボリュームデータを3次元で表示します。

## スクリーンショット


## 依存ライブラリ

このプロジェクトをビルド・実行するためには、以下のライブラリが必要です。

*   **OpenGL**: グラフィックスレンダリング用。バージョンX.X以上を推奨。
*   **GLEW (OpenGL Extension Wrangler Library)**: OpenGL拡張機能を管理します。
*   **GLFW (Graphics Library Framework)**: ウィンドウ作成、コンテキスト管理、入力処理を行います。
*   **GLM (OpenGL Mathematics)**: OpenGLのための数学ライブラリ（ベクトル、行列など）。
*   **(その他、もしあれば)**: 例: ImGui (GUI作成用ライブラリ) など

各ライブラリのインストール方法は、それぞれの公式サイトを参照してください。

## インストールの流れと実行例

### 1. リポジトリのクローン

まず、このリポジトリをローカルマシンにクローンします。

```bash
git clone https://github.com/regusan/GLVolumeRenderer.git
cd GLVolumeRenderer
```

### 2. 依存ライブラリの準備

```bash
sudo apt-get update
sudo apt-get install libglew-dev libglfw3-dev libglm-dev
```

### 3. ビルド
```bash
make
```
### 4. 実行


```bash
./volumen
```
## 使用方法 (オプション)

*(アプリケーション起動後の基本的な操作方法などを記述します。)*
*   マウス右ドラッグ: 視点回転
*   マウスホイール: ズームイン/ズームアウト