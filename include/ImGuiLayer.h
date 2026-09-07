#pragma once
#include <glm/glm.hpp>
#include <string>

struct GLFWwindow; // forward declaration: alcanza con un puntero, no
                    // hace falta el header completo de GLFW acá.

// ============================================================
// ImGuiLayer: encapsula TODA la inicialización/apagado de ImGui
// (CreateContext, los dos backends -- GLFW y OpenGL3 --, y el orden
// correcto de NewFrame/Render de cada frame) para que main.cpp no
// tenga que lidiar con esos detalles -- el mismo principio de
// encapsulamiento que ya usan Renderer y DebugDraw.
//
// Lo que SIGUE viviendo en main.cpp es el contenido de la UI de la
// aplicación (el panel "Controles", sus checkboxes, etc.): eso es
// lógica propia de cada programa, no boilerplate de la librería, así
// que no le corresponde a esta clase.
// ============================================================
class ImGuiLayer {
public:
    // Crea el contexto de ImGui e inicializa los backends de GLFW y
    // OpenGL3. Se llama UNA sola vez, después de crear la ventana y el
    // contexto de OpenGL (igual que Renderer::Init() y DebugDraw::Init()).
    void Init(GLFWwindow* window);

    // Libera el contexto y apaga ambos backends. Se llama UNA sola
    // vez, antes de glfwTerminate().
    void Shutdown();

    // Arranca un nuevo frame de UI: hay que llamarlo antes de
    // cualquier ImGui::Begin/End de la aplicación, y antes de
    // DrawText2D.
    void BeginFrame();

    // Cierra el frame de UI y lo manda a dibujar. Se llama UNA vez por
    // frame, al final -- después de dibujar toda la escena 3D y el
    // overlay 2D del Renderer, para que la UI quede siempre encima de
    // todo.
    void EndFrame();

    // Texto 2D directo sobre la pantalla, en coordenadas de píxel
    // (origen arriba-izquierda, igual que Renderer::DrawSquare2D):
    // aparece "flotando" encima de todo, sin necesitar un
    // ImGui::Begin/End propio. Por dentro usa el "foreground draw
    // list" de ImGui -- por eso solo puede llamarse ENTRE BeginFrame()
    // y EndFrame().
    void DrawText2D(float x, float y, const std::string& text,
                     const glm::vec3& color = glm::vec3(1.0f));
};
