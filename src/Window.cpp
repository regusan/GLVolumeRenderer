#include "Window.hpp"

// コンストラクタ
Window::Window(int width, int height, const std::string &title)
    : width(width), height(height), title(title), window(nullptr) {}

// デストラクタ
Window::~Window()
{
    if (window)
    {
        glfwDestroyWindow(window);
        glfwTerminate();
    }
}

// 初期化
bool Window::Initialize()
{
    if (!glfwInit())
    {
        std::cerr << "[ERROR] GLFW initialization failed." << std::endl;
        return false;
    }

    window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    if (!window)
    {
        std::cerr << "[ERROR] Window creation failed." << std::endl;
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(window);
    glewExperimental = true;
    if (glewInit() != GLEW_OK)
    {
        std::cerr << "[ERROR] GLEW initialization failed." << std::endl;
        return false;
    }
    // バージョン設定
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    return true;
}
