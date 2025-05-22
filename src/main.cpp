#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <vector>
#include <fstream>
#include <sstream>
#include <chrono>
#include "Volume.hpp"
#include "PointCloud.hpp"
#include "Shader.hpp"
#include "Window.hpp"
#include "ImGuiManager.hpp"
#include "FrameBuffer.hpp"
#include "Camera.hpp"

using namespace std;

int main(int argc, char const *argv[])
{

    if (argc < 2)
    {
    }
    string volumeFilepath = argv[1];

    Window window;
    window.Initialize();

    CustomImGuiManager imguiManager;

    // 深度は無効化
    glDisable(GL_DEPTH_TEST);
    // glEnable(GL_DEPTH_TEST);
    // glDepthFunc(GL_LESS);

    // バックフェースカリングは無効化
    glDisable(GL_CULL_FACE);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);

    // シェーダー読み込み
    Shader pointCLoudShader("shader/VolumePointCloud-vert.glsl", "shader/VolumePointCloud-frag.glsl");
    Shader raymarchingShader("shader/VolumeMarching-vert.glsl", "shader/VolumeMarching-frag.glsl");
    Shader &primaryShader = raymarchingShader;
    primaryShader.Use();

    std::ifstream volumeFile(volumeFilepath, std::ios::binary);
    if (!volumeFile.is_open())
    {
        cerr << "[ERROR] Failed to open file: " << volumeFilepath << endl;
        return -1;
    }
    strcpy(imguiManager.fileBuffer, volumeFilepath.c_str());

    // ボリュームデータの定義
    Volume volume = Volume(volumeFile);
    volume.UploadBuffer();
    PointCloud pointCloud;

    float gameTime = 0;
    float deltaSecond = 1.0f / 60.0f;

    glm::mat4 model = glm::mat4(1.0f);

    /// OpenGLの描画を行うメインウィンドウのバッファ
    FrameBuffer oglBuffer(100, 100);
    imguiManager.Initialize(window.GetGLFWwindow(), oglBuffer);

    /// カメラインスタンス
    Camera camera(window.GetGLFWwindow());

    // フレームループ
    while (!glfwWindowShouldClose(window.GetGLFWwindow()))
    {

        auto start = std::chrono::high_resolution_clock::now();

        // IMGUIウィンドウのサイズに合わせてアスペクト比を変化
        glm::mat4 projection = glm::perspective(glm::radians<float>(80),
                                                static_cast<float>(imguiManager.GetMainWindowSize().x) / static_cast<float>(imguiManager.GetMainWindowSize().y),
                                                imguiManager.nearClip, imguiManager.farClip);
        glm::mat4 invViewProj = glm::inverse(projection * camera.view);
        glm::vec3 cameraPos = glm::vec3(glm::inverse(camera.view)[3]);
        { // パラメータのGPUへの転送

            glUniformMatrix4fv(glGetUniformLocation(primaryShader.GetProgramID(), "model"),
                               1, GL_FALSE, &model[0][0]);
            glUniformMatrix4fv(glGetUniformLocation(primaryShader.GetProgramID(), "view"),
                               1, GL_FALSE, &camera.view[0][0]);
            glUniformMatrix4fv(glGetUniformLocation(primaryShader.GetProgramID(), "projection"),
                               1, GL_FALSE, &projection[0][0]);
            glUniform2f(glGetUniformLocation(primaryShader.GetProgramID(), "alphaRange"),
                        imguiManager.alphaMinMax[0], imguiManager.alphaMinMax[1]);
            glUniform2f(glGetUniformLocation(primaryShader.GetProgramID(), "nearFarClip"),
                        imguiManager.nearClip, imguiManager.farClip);
            glUniform1f(glGetUniformLocation(primaryShader.GetProgramID(), "pointSize"),
                        imguiManager.pointSize);
            glUniform1i(glGetUniformLocation(primaryShader.GetProgramID(), "volumeTexture"), 0);

            glUniform3fv(glGetUniformLocation(primaryShader.GetProgramID(), "cameraPos"),
                         1, glm::value_ptr(cameraPos));
            glUniformMatrix4fv(glGetUniformLocation(primaryShader.GetProgramID(), "invViewProj"),
                               1, GL_FALSE, glm::value_ptr(invViewProj));
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // 全バッファの初期化
        oglBuffer.bind();
        oglBuffer.Clear();
        if (imguiManager.currentShaderIndex == 0)
        { // レイマーチングで描画
            primaryShader = raymarchingShader;
            primaryShader.Use();
            volume.Draw();
        }
        else
        { // ポイントクラウドで描画
            static bool bIsPointCloudInitialized = false;
            if (!bIsPointCloudInitialized)
            {
                /// 初めてポイントクラウドになったときのみポイントクラウドへの変換を実行
                /// XXX:Staticの使用
                pointCloud = PointCloud(volume);
                pointCloud.UploadBuffer();
            }
            primaryShader = pointCLoudShader;
            primaryShader.Use();
            pointCloud.Draw();
        }
        oglBuffer.unbind();

        { // ImGuiフレームの開始
            imguiManager.cameraPos = cameraPos;
            imguiManager.BeginFrame();
            imguiManager.RenderUI();
            imguiManager.EndFrame();
        }

        // ダブルバッファのスワップ
        glfwSwapBuffers(window.GetGLFWwindow());

        { // IO系処理
            glfwPollEvents();
            camera.Update();
        }
        { // 時間系処理
            auto end = std::chrono::high_resolution_clock::now();
            deltaSecond = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() / 1000000.0;
            gameTime += deltaSecond;
        }
    }
    return 0;
}
