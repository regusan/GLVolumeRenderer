#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include <iostream>
#include <ostream>
#include <fstream>
#include <functional>

class ImGuiManager
{
public:
    ImGuiManager();
    ~ImGuiManager();

    virtual void Initialize(GLFWwindow *window);
    virtual void BeginFrame();
    virtual void EndFrame();
    virtual void RenderUI() = 0;
    void RenderDockSpace(); // ドッキングスペース描画
};

class CustomImGuiManager : public ImGuiManager
{
    using ButtonCallback = std::function<void()>;

private:
public:
    float nearClip = 0.1f;
    float farClip = 100.0f;
    float alphaMin = 0.0f;
    float alphaMax = 1.0f;
    std::string filePath = "";
    char fileBuffer[256];
    virtual void RenderUI() override;
    ButtonCallback callback;
};