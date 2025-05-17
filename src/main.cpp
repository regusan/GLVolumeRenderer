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
using namespace std;

// スクロール用変数
GLfloat scale = 45.0f;

// スクロールコールバック

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
    scale += yoffset;
    std::cout << "Scale: " << scale << std::endl;
}

// シェーダーソース読み込み関数
std::string LoadShaderSource(const std::string &filepath)
{
    std::ifstream file(filepath);
    if (!file.is_open())
    {
        std::cerr << "Failed to open shader file: " << filepath << std::endl;
        return "";
    }

    std::stringstream shaderStream;
    shaderStream << file.rdbuf();
    return shaderStream.str();
}

// シェーダープログラムの作成
GLuint LoadShaderProgram(const std::string &vertexPath, const std::string &fragmentPath)
{
    auto compileShader = [](const std::string &source, GLenum type)
    {
        GLuint shader = glCreateShader(type);
        const char *src = source.c_str();
        glShaderSource(shader, 1, &src, nullptr);
        glCompileShader(shader);

        GLint success;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            char infoLog[512];
            glGetShaderInfoLog(shader, 512, nullptr, infoLog);
            std::cerr << "Shader Compilation Error: " << infoLog << std::endl;
        }
        return shader;
    };

    std::string vertexSource = LoadShaderSource(vertexPath);
    std::string fragmentSource = LoadShaderSource(fragmentPath);
    GLuint vertexShader = compileShader(vertexSource, GL_VERTEX_SHADER);
    GLuint fragmentShader = compileShader(fragmentSource, GL_FRAGMENT_SHADER);

    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return program;
}

// 点群の描画関数
void DrawPointCloud(GLuint vao, size_t pointCount)
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glPointSize(1.0f);
    glBindVertexArray(vao);
    glDrawArrays(GL_POINTS, 0, pointCount);
    glBindVertexArray(0);
}

int main()
{
    // GLFW初期化
    if (!glfwInit())
    {
        std::cerr << "[ERROR] GLFW initialization failed." << std::endl;
        return -1;
    }

    // ウィンドウ作成
    GLFWwindow *window = glfwCreateWindow(1280, 960, "OpenGL Point Cloud", nullptr, nullptr);
    if (!window)
    {
        std::cerr << "[ERROR] Window creation failed." << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glewExperimental = true;
    if (glewInit() != GLEW_OK)
    {
        std::cerr << "[ERROR] GLEW initialization failed." << std::endl;
        return -1;
    }

    // バージョン設定
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // 深度は無効化
    glDisable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // バックフェースカリングは無効化
    glDisable(GL_CULL_FACE);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);

    // スクロールコールバック
    glfwSetScrollCallback(window, scroll_callback);

    // シェーダー読み込み
    GLuint program = LoadShaderProgram("shader/vert.glsl", "shader/frag.glsl");

    glUseProgram(program);

    // シェーダーハンドル
    GLuint modelMatrixID = glGetUniformLocation(program, "model");
    GLuint viewMatrixID = glGetUniformLocation(program, "view");
    GLuint projectionMatrixID = glGetUniformLocation(program, "projection");

    string volumeFilepath = "volume/256_256_256B.dat";
    // string volumeFilepath = "volume/256_256_256E.dat";
    // string volumeFilepath = "volume/256_256_256K.dat";
    // string volumeFilepath = "volume/512_512_512M.dat";
    // string volumeFilepath = "volume/512_512_512W.dat";
    std::ifstream volumeFile(volumeFilepath, std::ios::binary);
    if (!volumeFile.is_open())
    {
        cerr << "[ERROR] Failed to open file: " << volumeFilepath << endl;
        return -1;
    }
    auto volume = Volume(volumeFile);
    auto pointCloud = PointCloud(volume);
    cout << "vertices size: " << volume.data.size() << endl;

    float gameTime = 0;
    float deltaSecond = 1.0f / 60.0f;

    glm::mat4 model = glm::mat4(1.0f);
    {
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
        // 回転アニメーション
        model = glm::rotate(model, glm::radians<float>(gameTime * 5), glm::vec3(0.0f, 1.0f, 0.0f));
        // 拡縮アニメーション
        // model = glm::scale(model, glm::vec3(fabs(sin(gameTime * 0.1)) + 0.5, fabs(sin(gameTime * 0.1)) + 0.5, 1.0f));
        model = glm::scale(model, glm::vec3(3));
    }

    // フレームループ
    while (!glfwWindowShouldClose(window))
    {
        auto start = std::chrono::high_resolution_clock::now();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        gameTime += deltaSecond;
        // 行列設定

        glm::mat4 view = glm::lookAt(glm::vec3(4, 3, 3), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
        int width, height;
        glfwGetWindowSize(window, &width, &height);
        glm::mat4 projection = glm::perspective(glm::radians(scale), static_cast<float>(width) / static_cast<float>(height), 0.1f, 100.0f);

        glUniformMatrix4fv(modelMatrixID, 1, GL_FALSE, &model[0][0]);
        glUniformMatrix4fv(viewMatrixID, 1, GL_FALSE, &view[0][0]);
        glUniformMatrix4fv(projectionMatrixID, 1, GL_FALSE, &projection[0][0]);

        // 点群描画
        DrawPointCloud(pointCloud.vao, pointCloud.vertices.size());

        // ダブルバッファのスワップ
        glfwSwapBuffers(window);
        glfwPollEvents();

        // マウスの左ボタンが押されているか確認
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS)
        {
            std::cout << "Left Mouse Button is pressed." << std::endl;
            // マウスの位置を取得
            double xpos, ypos;
            glfwGetCursorPos(window, &xpos, &ypos);
            std::cout << "Mouse Position: (" << xpos << ", " << ypos << ")" << std::endl;
            {
                model = glm::mat4(1.0f);
                model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
                // 回転アニメーション
                model = glm::rotate(model, glm::radians<float>(xpos * 1), glm::vec3(0.0f, 1.0f, 0.0f));
                model = glm::rotate(model, glm::radians<float>(ypos * 1), glm::vec3(1.0f, 0.0f, 0.0f));
                // 拡縮アニメーション
                // model = glm::scale(model, glm::vec3(fabs(sin(gameTime * 0.1)) + 0.5, fabs(sin(gameTime * 0.1)) + 0.5, 1.0f));
                model = glm::scale(model, glm::vec3(3));
            }
        }

        auto end = std::chrono::high_resolution_clock::now();
        deltaSecond = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() / 1000000.0;
        // cout << 1.0f / deltaSecond << "fps" << endl;
    }

    // 終了処理

    glDeleteProgram(program);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
