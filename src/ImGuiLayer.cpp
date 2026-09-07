#include "ImGuiLayer.h"

#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_opengl3.h"

void ImGuiLayer::Init(GLFWwindow* window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
}

void ImGuiLayer::Shutdown() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void ImGuiLayer::BeginFrame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void ImGuiLayer::EndFrame() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void ImGuiLayer::DrawText2D(float x, float y, const std::string& text, const glm::vec3& color) {
    // El "foreground draw list" es una superficie de dibujo de ImGui
    // que queda por ENCIMA de todas las ventanas -- no hace falta
    // abrir un ImGui::Begin("...") para usarla, por eso el texto
    // aparece "flotando" en pantalla en vez de metido en un panel.
    // Solo es válido llamarlo entre BeginFrame() (que arranca el frame
    // de ImGui) y EndFrame() (que lo manda a dibujar).
    ImU32 colorU32 = ImGui::ColorConvertFloat4ToU32(ImVec4(color.r, color.g, color.b, 1.0f));
    ImGui::GetForegroundDrawList()->AddText(ImVec2(x, y), colorU32, text.c_str());
}
