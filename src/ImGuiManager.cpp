#include "ImGuiManager.hpp"
#include <glm/gtc/type_ptr.hpp>
using namespace std;
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

    if (ImGui::BeginTable("KeyBindingsTable", 2))
    {
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("Action");
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("Key");

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("Move");
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("Mouse Middle Btn + Mouse Movement");

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("Rotate Camera");
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("Mouse Right Btn + Mouse Movement");

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("Zoom In/Out");
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("Mouse Wheel");

        ImGui::EndTable();
    }

    // FPS履歴の更新
    fpsHistory.push_back(ImGui::GetIO().Framerate);
    if (fpsHistory.size() > 300)
        fpsHistory.erase(fpsHistory.begin());
    float minfps = *min_element(fpsHistory.begin(), fpsHistory.end());
    float maxfps = *max_element(fpsHistory.begin(), fpsHistory.end());
    string frameRate = "fps History:\t" + to_string(static_cast<int>(ImGui::GetIO().Framerate)) + "fps";

    // FPSグラフの描画
    ImGui::PlotLines("", fpsHistory.data(), fpsHistory.size(), 0, frameRate.data(), minfps / 1.5, maxfps * 1.5, ImVec2(-1, 80));

    // Near/Far Clip 調整
    ImGui::SliderFloat("Near Clip", &nearClip, 0.001f, 3.0f);
    ImGui::SliderFloat("Far Clip", &farClip, 0.01f, 3.0f);

    ImGui::SliderFloat("Point Size", &pointSize, 0.1f, 2.0f);

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
    const char *shaderNames[] = {"Ray Casting", "Ray Casting(Max)", "Point Cloud"};
    if (ImGui::Combo("Select Shader", &currentShaderIndex, shaderNames, IM_ARRAYSIZE(shaderNames)))
    {
    }

    // アルファ値調整
    ImGui::SliderFloat2("Alpha Min-Max", alphaMinMax, 0.0f, 1.0f);

    ImGui::DragFloat3("CameraPosition", glm::value_ptr(cameraPos), 0.01f);

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