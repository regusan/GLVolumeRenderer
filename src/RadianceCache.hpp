#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

#include <iostream>

#include "Shader.hpp"

class RadianceCache
{
private:
    GLuint textureID;
    Shader computeShader;
    size_t size;
    static GLuint UploadBuffer(const size_t size);

public:
    RadianceCache(const size_t size, const Shader &shader);
    ~RadianceCache();
    void Update()
    {
        glUseProgram(computeShader.GetProgramID());

        // メモリバリアを挿入 (必要な場合)
        // テクスチャへの書き込みが完了するのを待つ、など
        // glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        // Compute Shader を実行
        // BUF_WIDTH / 8: X方向のワークグループ数 (全体のサイズ / ローカルサイズ)
        // BUF_HEIGHT / 8: Y方向のワークグループ数
        // BUF_DEPTH / 8: Z方向のワークグループ数
        glDispatchCompute(this->size / 8, this->size / 8, this->size / 8);

        // 計算結果が他の処理 (例: レンダリング) で使われる前に、
        // 書き込みが完了するのを保証するためのメモリバリア
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        glUseProgram(0);

        // これで textureID にはCompute Shaderによって計算された値が書き込まれている
        // このテクスチャを通常のレンダリングパイプラインでサンプリングできる
    }
};

RadianceCache::RadianceCache(const size_t size, const Shader &shader) : size(size), computeShader(shader)
{
}

RadianceCache::~RadianceCache()
{
}
GLuint RadianceCache::UploadBuffer(const size_t size)
{
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_3D, textureID);

    // テクスチャのストレージを確保
    // GL_R32F: 1チャンネル、32ビット浮動小数点数フォーマット
    // 他にも GL_RGBA8, GL_RGBA32F など様々なフォーマットが利用可能
    glTexStorage3D(GL_TEXTURE_3D, 1, GL_R32F, size, size, size);

    // フィルタリングやラップモードを設定 (必須ではないが推奨)
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    // テクスチャをイメージユニット 1 にバインド
    // GL_READ_WRITE: 読み書き両用
    // GL_R32F: シェーダー側でのフォーマット指定と一致させる
    glBindImageTexture(1, textureID, 0, GL_TRUE, 0, GL_READ_WRITE, GL_R32F);

    glBindTexture(GL_TEXTURE_3D, 0); // バインド解除

    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR)
    {
        std::cerr << "OpenGL Error: " << err << std::endl;
    }
    return textureID;
}