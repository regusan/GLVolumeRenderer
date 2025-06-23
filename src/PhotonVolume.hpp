
#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

#include <iostream>
#include <vector>
#include <iomanip>

#include "Shader.hpp"

class PhotonVolume
{
private:
    GLuint textureID;
    Shader computeShader;
    size_t size;
    // UploadBuffer を非 static に変更
    GLuint UploadBuffer(const size_t size);

public:
    PhotonVolume(const size_t size, const Shader &shader);
    ~PhotonVolume();
    void Update();
    void PrintData();                                 // <--- 追加: データを表示する関数
    GLuint GetTextureID() const { return textureID; } // <--- 追加: テクスチャID取得用
};

// コンストラクタ: UploadBuffer を呼び出し textureID を初期化
PhotonVolume::PhotonVolume(const size_t size, const Shader &shader)
    : size(size), computeShader(shader)
{
    this->textureID = UploadBuffer(this->size); // <--- UploadBuffer を呼び出し
}

PhotonVolume::~PhotonVolume()
{
    glDeleteTextures(1, &textureID); // <--- デストラクタでテクスチャを削除
}

GLuint PhotonVolume::UploadBuffer(const size_t size)
{
    GLuint texID; // ローカル変数に変更
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_3D, texID);

    glTexStorage3D(GL_TEXTURE_3D, 1, GL_R32F, size, size, size);

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    // バインド解除 (ImageTexture のバインドは Update で行う)
    glBindTexture(GL_TEXTURE_3D, 0);

    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR)
    {
        std::cerr << "OpenGL Error during UploadBuffer: " << err << std::endl;
    }
    return texID; // ID を返す
}

void PhotonVolume::Update()
{
    glUseProgram(computeShader.GetProgramID());

    // テクスチャをイメージユニット 1 にバインド (シェーダーの binding=0 に合わせる)
    glBindImageTexture(1, this->textureID, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_R32F); // <--- ユニット番号0, WRITE_ONLYに変更
    glBindImageTexture(0, 0, 0, GL_TRUE, 0, GL_READ_ONLY, GL_R32F);
    glDispatchCompute(this->size / 8, this->size / 8, this->size / 8);

    // 書き込み完了を保証
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    // イメージユニットのバインドを解除 (任意だが推奨)
    glBindImageTexture(0, 0, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_R32F);

    glUseProgram(0);
}

void PhotonVolume::PrintData()
{
    if (textureID == 0)
    {
        std::cerr << "PrintData Error: Texture ID is 0. Was it initialized?" << std::endl;
        return;
    }

    std::cout << "Reading data from 3D texture (Size: "
              << size << "x" << size << "x" << size << ")..." << std::endl;

    glFinish();

    size_t totalElements = size * size * size;
    std::vector<float> data(totalElements);

    glBindTexture(GL_TEXTURE_3D, this->textureID);
    glGetTexImage(GL_TEXTURE_3D, 0, GL_RED, GL_FLOAT, data.data());
    glBindTexture(GL_TEXTURE_3D, 0);

    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR)
    {
        std::cerr << "OpenGL Error during glGetTexImage: " << err << std::endl;
        return;
    }

    std::cout << "Texture Data (Z=0 Slice, Abbreviated):" << std::endl;
    std::cout << std::fixed << std::setprecision(4);

    // --- 省略表示のための設定 ---
    const size_t show_count = 3;         // 先頭と末尾に表示する要素数
    const size_t limit = show_count * 2; // この数を超えたら省略する
    size_t z_slice_to_print = 0;
    // -------------------------

    bool omit_y = size > limit;  // Y方向 (行) を省略するか
    bool y_dots_printed = false; // Y方向の省略記号 (...) を表示したか

    for (size_t y = 0; y < size; ++y)
    {
        // この行 (y) を表示するかどうかを判断
        bool should_print_y = !omit_y || (y < show_count) || (y >= size - show_count);

        if (should_print_y)
        {
            // --- 行の表示処理 ---
            bool omit_x = size > limit;  // X方向 (列) を省略するか
            bool x_dots_printed = false; // X方向の省略記号 (...) を表示したか

            for (size_t x = 0; x < size; ++x)
            {
                // この列 (x) を表示するかどうかを判断
                bool should_print_x = !omit_x || (x < show_count) || (x >= size - show_count);

                if (should_print_x)
                {
                    // 実際の値を表示
                    size_t index = (z_slice_to_print * size * size) + (y * size) + x;
                    std::cout << data[index] << " ";
                }
                else if (!x_dots_printed)
                {
                    // X方向の省略記号 (...) を一度だけ表示
                    std::cout << "... ";
                    x_dots_printed = true;
                }
                // これ以外の (should_print_x == false && x_dots_printed == true) 場合は何も表示しない
            }
            std::cout << std::endl; // 行末で改行
            // --- 行の表示処理ここまで ---
        }
        else if (!y_dots_printed)
        {
            // Y方向の省略記号 (...) を一度だけ表示
            // X方向の表示に合わせて、それっぽく表示
            bool omit_x = size > limit;
            for (size_t x = 0; x < size; ++x)
            {
                bool should_print_x = !omit_x || (x < show_count) || (x >= size - show_count);
                if (should_print_x)
                {
                    std::cout << ":      "; // 桁数に合わせて調整
                }
                else if (x == show_count)
                {
                    std::cout << "... ";
                }
            }
            std::cout << std::endl;
            y_dots_printed = true;
        }
        // これ以外の (should_print_y == false && y_dots_printed == true) 場合は何も表示しない
    }

    std::cout << "Data reading finished." << std::endl;
}
