#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <string>
#include <iostream>

class Window
{
private:
    GLFWwindow *window;
    int width, height;
    std::string title;

public:
    Window(int width = 1280, int height = 960, const std::string &title = "OpenGL Volume Renderer");
    ~Window();

    bool Initialize();
    GLFWwindow *GetGLFWwindow() const { return window; }
};
