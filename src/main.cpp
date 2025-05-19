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

using namespace std;

// スクロール用変数
GLfloat scale = 1.0f;

// スクロールコールバック

void scroll_callback(GLFWwindow *, double, double yoffset)
{
    scale += yoffset * 0.1;
    std::cout << "Scale: " << scale << std::endl;
}

int main()
{

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

    // スクロールコールバック
    glfwSetScrollCallback(window.GetGLFWwindow(), scroll_callback);

    // シェーダー読み込み
    Shader shader("shader/vert.glsl", "shader/frag.glsl");
    // Shader shader("shader/id-viewer-vert.glsl", "shader/frag.glsl");
    shader.Use();

    // シェーダーハンドル
    GLuint modelMatrixID = glGetUniformLocation(shader.GetProgramID(), "model");
    GLuint viewMatrixID = glGetUniformLocation(shader.GetProgramID(), "view");
    GLuint projectionMatrixID = glGetUniformLocation(shader.GetProgramID(), "projection");
    GLuint alphaRangeLocation = glGetUniformLocation(shader.GetProgramID(), "alphaRange");

    // string volumeFilepath = "NonShareVolume/256_256_256B.dat";
    //    string volumeFilepath = "NonShareVolume/256_256_256E.dat";
    //      string volumeFilepath = "NonShareVolume/256_256_256K.dat";
    // string volumeFilepath = "NonShareVolume/256_256_256S.dat";
    // string volumeFilepath = "NonShareVolume/512_512_512M.dat";
    // string volumeFilepath = "NonShareVolume/512_512_512W.dat";
    string volumeFilepath = "NonShareVolume/512_512_512C.dat";

    //  string volumeFilepath = "volume/shape1.dat";
    //    string volumeFilepath = "volume/shape2.dat";

    std::ifstream volumeFile(volumeFilepath, std::ios::binary);
    if (!volumeFile.is_open())
    {
        cerr << "[ERROR] Failed to open file: " << volumeFilepath << endl;
        return -1;
    }
    strcpy(imguiManager.fileBuffer, volumeFilepath.c_str());

    auto volume = Volume(volumeFile);
    auto pointCloud = PointCloud(volume);
    cout << volume << endl;

    float gameTime = 0;
    float deltaSecond = 1.0f / 60.0f;

    glm::mat4 model = glm::mat4(1.0f);
    {
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
        // 回転アニメーション
        model = glm::rotate(model, glm::radians<float>(gameTime * 5), glm::vec3(0.0f, 1.0f, 0.0f));
        // 拡縮アニメーション
        // model = glm::scale(model, glm::vec3(fabs(sin(gameTime * 0.1)) + 0.5, fabs(sin(gameTime * 0.1)) + 0.5, 1.0f));
        model = glm::scale(model, glm::vec3(scale));
    }

    FrameBuffer finalBuffer(100, 100);
    imguiManager.Initialize(window.GetGLFWwindow(), finalBuffer);

    cout << "Start Main loop" << endl;
    // フレームループ
    while (!glfwWindowShouldClose(window.GetGLFWwindow()))
    {
        auto start = std::chrono::high_resolution_clock::now();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 行列設定
        glm::mat4 view = glm::lookAt(glm::vec3(0, 3, 3), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
        int width, height;
        glfwGetWindowSize(window.GetGLFWwindow(), &width, &height);
        glm::mat4 projection = glm::perspective(glm::radians<float>(80),
                                                static_cast<float>(imguiManager.GetMainWindowSize().x) / static_cast<float>(imguiManager.GetMainWindowSize().y),
                                                imguiManager.nearClip, imguiManager.farClip);

        finalBuffer.bind();
        glUniformMatrix4fv(modelMatrixID, 1, GL_FALSE, &model[0][0]);
        glUniformMatrix4fv(viewMatrixID, 1, GL_FALSE, &view[0][0]);
        glUniformMatrix4fv(projectionMatrixID, 1, GL_FALSE, &projection[0][0]);
        glUniform2f(alphaRangeLocation, imguiManager.alphaMin, imguiManager.alphaMax);

        finalBuffer.Clear();
        pointCloud.Draw();
        finalBuffer.unbind();

        {
            // ImGuiフレームの開始
            imguiManager.BeginFrame();
            imguiManager.RenderUI();
            imguiManager.EndFrame();
        }

        // ダブルバッファのスワップ
        glfwSwapBuffers(window.GetGLFWwindow());
        glfwPollEvents();

        // マウスの左ボタンが押されているか確認
        if (glfwGetMouseButton(window.GetGLFWwindow(), GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
        {
            std::cout << "Left Mouse Button is pressed." << std::endl;
            // マウスの位置を取得
            double xpos, ypos;
            glfwGetCursorPos(window.GetGLFWwindow(), &xpos, &ypos);
            std::cout << "Mouse Position: (" << xpos << ", " << ypos << ")" << std::endl;
            {
                model = glm::mat4(1);
                model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
                // 回転アニメーション
                model = glm::rotate(model, glm::radians<float>(xpos * 1), glm::vec3(0.0f, 1.0f, 0.0f));
                model = glm::rotate(model, glm::radians<float>(ypos * 1), glm::vec3(1.0f, 0.0f, 0.0f));
                // 拡縮アニメーション
                // model = glm::scale(model, glm::vec3(fabs(sin(gameTime * 0.1)) + 0.5, fabs(sin(gameTime * 0.1)) + 0.5, 1.0f));
                model = glm::scale(model, glm::vec3(scale));
            }
        }

        auto end = std::chrono::high_resolution_clock::now();
        deltaSecond = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() / 1000000.0;
        gameTime += deltaSecond;
    }
    return 0;
}
