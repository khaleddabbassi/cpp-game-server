#include "server_system.h"
#include "nativesevercmds.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <deque>
#include <mutex>
#include <queue>
#include <thread>
#include <chrono>
#include <condition_variable>

std::deque<std::string> logLines;
const size_t maxLogLines = 1000;
std::queue<std::string> guiLogQueue;
std::mutex guiLogMutex;
std::mutex commandMutex;
std::queue<std::string> commandQueue;
std::condition_variable commandCondVar;
std::atomic<bool> guiRunning{true};

void addGuiLog(const std::string& message) {
    std::lock_guard<std::mutex> lock(guiLogMutex);
    guiLogQueue.push(message);
}

void setupLowContrastTheme() {
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;
    colors[ImGuiCol_WindowBg] = ImVec4(0.13f, 0.13f, 0.13f, 1.00f);
    colors[ImGuiCol_Text] = ImVec4(0.72f, 0.72f, 0.72f, 1.00f);
    colors[ImGuiCol_Button] = ImVec4(0.23f, 0.23f, 0.23f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.28f, 0.28f, 0.28f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.23f, 0.23f, 0.23f, 1.00f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.13f, 0.13f, 0.13f, 1.00f);
    style.FrameRounding = 4.0f;
    style.WindowRounding = 4.0f;
    style.ScrollbarRounding = 4.0f;
    style.GrabRounding = 4.0f;
}

void runGui() {
    if (!glfwInit()) return;
    GLFWwindow* window = glfwCreateWindow(800, 600, "dark sun engine", nullptr, nullptr);
    if (!window) { glfwTerminate(); return; }
    glfwMakeContextCurrent(window);
    if (glewInit() != GLEW_OK) { glfwDestroyWindow(window); glfwTerminate(); return; }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");
    setupLowContrastTheme();

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar;

    addGuiLog("graphical interface initialized");

    char commandInput[512] = "";
    bool autoScroll = true;

    while (!glfwWindowShouldClose(window) && running) {
        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Pull logs into local deque
        {
            std::lock_guard<std::mutex> lock(guiLogMutex);
            while (!guiLogQueue.empty()) {
                logLines.push_back(">> " + guiLogQueue.front());
                guiLogQueue.pop();
                if (logLines.size() > maxLogLines) logLines.pop_front();
            }
        }

        int winWidth, winHeight;
        glfwGetWindowSize(window, &winWidth, &winHeight);
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2(static_cast<float>(winWidth), static_cast<float>(winHeight)));

        if (ImGui::Begin("Server Log", nullptr, flags)) {
            // Log display
            ImGui::BeginChild("Log", ImVec2(0, ImGui::GetContentRegionAvail().y - 40), true, ImGuiWindowFlags_HorizontalScrollbar);
            for (const auto& line : logLines) ImGui::TextUnformatted(line.c_str());
            if (autoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY() - 1.0f) ImGui::SetScrollHereY(1.0f);
            ImGui::EndChild();

            // Auto-scroll checkbox
            ImGui::Checkbox("Auto-scroll", &autoScroll);

            // Command input
            bool enterPressed = ImGui::InputText("##CommandInput", commandInput, sizeof(commandInput),
                                                 ImGuiInputTextFlags_EnterReturnsTrue);
            ImGui::SameLine();
            if (enterPressed) {
                std::string cmdStr(commandInput);
                if (!cmdStr.empty()) {
                    std::string serverMessage = "say server " + cmdStr;
                    enqueueMessage(serverMessage, nullptr);
                    commandInput[0] = '\0';
                }
                ImGui::SetKeyboardFocusHere(-1); // Keep focus on input after pressing Enter
            }
        }
        ImGui::End();

        // Render
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.2f, 0.22f, 0.25f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);

        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }

    running = false;

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();

    serverCondVar.notify_all();
    commandCondVar.notify_all();
}
