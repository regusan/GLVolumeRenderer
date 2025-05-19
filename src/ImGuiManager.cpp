#include "ImGuiManager.hpp"

ImGuiManager::ImGuiManager() {}

ImGuiManager::~ImGuiManager()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void ImGuiManager::Initialize(GLFWwindow *window, FrameBuffer &frameBuffer)
{
    this->frameBuffer = frameBuffer;
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // ドッキングを有効化

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
}

void ImGuiManager::BeginFrame()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void ImGuiManager::EndFrame()
{
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void CustomImGuiManager::RenderUI()
{
    RenderDockSpace();
    ImGui::Begin("Control Panel");
    ImGui::Text("FPS: %.2f", ImGui::GetIO().Framerate);

    // Near/Far Clip 調整
    ImGui::SliderFloat("Near Clip", &nearClip, 0.01f, 10.0f);
    ImGui::SliderFloat("Far Clip", &farClip, 0.01f, 100.0f);

    ImGui::InputText("File Path", fileBuffer, 256);
    // ファイル読み取り
    if (ImGui::Button("Load Volume"))
    {
        filePath = std::string(fileBuffer);
        std::ifstream file(filePath);
        if (file.is_open())
            std::cout << "File loaded: " << filePath << std::endl;
        else
            std::cerr << "Failed to load file: " << filePath << std::endl;
        callback();
    }

    // アルファ値調整
    ImGui::SliderFloat("Alpha Min", &alphaMin, 0.0f, 1.0f);
    ImGui::SliderFloat("Alpha Max", &alphaMax, 0.0f, 1.0f);

    ImGui::End();

    // OpenGL描画用のウィンドウ
    ImGui::Begin("OpenGL Render");
    this->mainWindowSize = ImGui::GetContentRegionAvail();
    ImVec2 pos = ImGui::GetCursorScreenPos();

    // we access the ImGui window size

    // we rescale the framebuffer to the actual window size here and reset the glViewport
    this->frameBuffer.rescale(this->mainWindowSize.x, this->mainWindowSize.y);
    glViewport(0, 0, this->mainWindowSize.x, this->mainWindowSize.y);

    // we get the screen position of the window
    // ImVec2 pos = ImGui::GetCursorScreenPos();

    // and here we can add our created texture as image to ImGui
    // unfortunately we need to use the cast to void* or I didn't find another way tbh
    ImGui::GetWindowDrawList()->AddImage(
        this->frameBuffer.getTextureID(),
        ImVec2(pos.x, pos.y),
        ImVec2(pos.x + this->mainWindowSize.x, pos.y + this->mainWindowSize.y),
        ImVec2(0, 1),
        ImVec2(1, 0));

    ImGui::End();
}

void ImGuiManager::RenderDockSpace()
{
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
                                    ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                                    ImGuiWindowFlags_NoBringToFrontOnFocus |
                                    ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_MenuBar;

    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

    ImGui::Begin("Main Docking Space", nullptr, window_flags);
    ImGui::PopStyleVar(2);

    ImGuiID dockspace_id = ImGui::GetID("MyDockspace");
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);
    ImGui::End();
}