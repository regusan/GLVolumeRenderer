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
#include <optional>
#include "Volume.hpp"
#include "PointCloud.hpp"
#include "Shader.hpp"
#include "Window.hpp"
#include "ImGuiManager.hpp"
#include "FrameBuffer.hpp"
#include "Camera.hpp"
// #include "RadianceCache.hpp"

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
    glClearColor(0.1f, 0.1f, 0.1f, 0.1f);

    // シェーダー読み込み
    Shader pointCloudShader("shader/VolumePointCloud.vert", "shader/VolumePointCloud.frag");
    Shader raycastShader("shader/VolumeMarching.vert", "shader/VolumeMarching.frag");
    Shader raycastMaxShader("shader/VolumeMarching.vert", "shader/VolumeCasting-Max.frag");
    Shader &primaryShader = raycastShader;

    // Shader radianceCacheShader = Shader("shader/ComputeShaderTest.glsl");
    // RadianceCache radianceCache(256, radianceCacheShader);
    // radianceCache.Update();

    std::ifstream volumeFile(volumeFilepath, std::ios::binary);
    if (!volumeFile.is_open())
    {
        cerr << "[ERROR] Failed to open file: " << volumeFilepath << endl;
        return -1;
    }

    // ボリュームデータの定義
    Volume volume = Volume(volumeFile);
    volume.UploadBuffer();
    optional<PointCloud> pointCloud;

    float gameTime = 0;
    float deltaSecond = 1.0f / 60.0f;

    glm::mat4 model = glm::mat4(1.0f);

    /// OpenGLの描画を行うメインウィンドウのバッファ
    FrameBuffer oglBuffer(100, 100);
    imguiManager.Initialize(window.GetGLFWwindow(), oglBuffer);
    imguiManager.fileBuffer = volumeFilepath;

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
            glUniform1i(glGetUniformLocation(primaryShader.GetProgramID(), "volumeResolution"),
                        volume.size);
            glUniform3f(glGetUniformLocation(primaryShader.GetProgramID(), "ambientLight"),
                        imguiManager.ambientLight.x, imguiManager.ambientLight.y, imguiManager.ambientLight.z);
            imguiManager.light.UploadBuffer(primaryShader.GetProgramID(), "light");
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // 全バッファの初期化
        oglBuffer.bind();
        oglBuffer.Clear();

        // レンダリング方式切り替え
        if (imguiManager.currentShaderIndex == 0)
        { // レイキャスティングで描画
            primaryShader = raycastShader;
            primaryShader.Use();
            volume.Draw();
        }
        else if (imguiManager.currentShaderIndex == 1)
        { // レイキャスティングで描画(Max)
            /*             primaryShader = raycastMaxShader;
                        primaryShader.Use();
                        volume.Draw(); */
        }
        else if (imguiManager.currentShaderIndex == 2)
        { // ポイントクラウドで描画
            if (pointCloud.has_value() == false)
            {
                /// 初めてポイントクラウドになったときのみポイントクラウドへの変換を実行
                pointCloud = PointCloud(volume);
                pointCloud->UploadBuffer();
            }
            primaryShader = pointCloudShader;
            primaryShader.Use();
            pointCloud->Draw(camera.view * model);
        }
        oglBuffer.unbind();

        { // ImGuiフレームの開始
            imguiManager.cameraPos = camera.GetPos();
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
