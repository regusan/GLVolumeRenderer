#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include <glm/glm.hpp>
#include <iostream>
#include <ostream>
#include <fstream>
#include <functional>
#include "FrameBuffer.hpp"
#include <vector>
#include "PointLight.hpp"

class ImGuiManager
{
protected:
    FrameBuffer frameBuffer;
    glm::ivec2 mainWindowSize = {640, 320};

public:
    ImGuiManager();
    ~ImGuiManager();

    virtual void Initialize(GLFWwindow *window, FrameBuffer &frameBuffer);
    virtual void BeginFrame();
    virtual void EndFrame();
    virtual void RenderUI() = 0;
    void RenderDockSpace(); // ドッキングスペース描画
    glm::ivec2 GetMainWindowSize() const { return mainWindowSize; };
};

class CustomImGuiManager : public ImGuiManager
{
    using ButtonCallback = std::function<void()>;

private:
public:
    std::vector<float> fpsHistory = std::vector<float>(100, 0);
    float nearClip = 0.01f;
    float farClip = 100.0f;
    float alphaMinMax[2] = {0.0f, 1.0f};
    float pointSize = 1.0f;
    std::string filePath = "";
    std::string fileBuffer;
    glm::vec3 cameraPos;
    PointLight light;
    glm::vec3 ambientLight = glm::vec3(0.3f);

    virtual void RenderUI() override;
    int currentShaderIndex = 0;
    ButtonCallback callback;
};