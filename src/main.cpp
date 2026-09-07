#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <memory>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Notar lo que YA NO aparece acá: los headers de los backends de
// ImGui (imgui_impl_glfw.h / imgui_impl_opengl3.h). main.cpp sigue
// usando la API "de aplicación" de ImGui (Begin/Text/Checkbox/End, más
// abajo) para su propio panel, pero toda la inicialización, el
// arranque/cierre de cada frame y el apagado quedaron encapsulados en
// ImGuiLayer (ver ImGuiLayer.h/.cpp).
#include "imgui/imgui.h"

#include "DebugDraw.h"
#include "Renderer.h"
#include "ImGuiLayer.h"
#include "Engine.h"
#include "Player.h"
#include "Enemy.h"
#include "Bullet.h"

using namespace std;

// ============================================================
// Mini-engine: GameObject + Engine + Player/Enemy/Bullet, con
// std::unique_ptr para la propiedad de los objetos, deltaTime para el
// movimiento, y AABB + OnTriggerEnter3D/Stay3D/Exit3D para la
// detección de eventos.
// ============================================================

const int SCREEN_WIDTH = 900;
const int SCREEN_HEIGHT = 600;
const string motorVersion = "1.1";

float GetDeltaTime() {
    static double tiempoAnterior = glfwGetTime();
    double tiempoActual = glfwGetTime();
    float deltaTime = static_cast<float>(tiempoActual - tiempoAnterior);
    tiempoAnterior = tiempoActual;
    return deltaTime;
}

// Mueve la cámara con las flechas, y la resetea con R.
void processInput(GLFWwindow* w, glm::vec3& cameraPosition, const glm::vec3& posInicial) {
    float velocidad = 0.05f;
    if (glfwGetKey(w, GLFW_KEY_UP)    == GLFW_PRESS) cameraPosition.y += velocidad;
    if (glfwGetKey(w, GLFW_KEY_DOWN)  == GLFW_PRESS) cameraPosition.y -= velocidad;
    if (glfwGetKey(w, GLFW_KEY_LEFT)  == GLFW_PRESS) cameraPosition.x -= velocidad;
    if (glfwGetKey(w, GLFW_KEY_RIGHT) == GLFW_PRESS) cameraPosition.x += velocidad;
    if (glfwGetKey(w, GLFW_KEY_R)     == GLFW_PRESS) cameraPosition = posInicial;
}

int main() {

    if (!glfwInit()) return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    const string windowName = "Minimotor v" + motorVersion;

    GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, windowName.c_str(), nullptr, nullptr);
    if (!window) {
        std::cerr << "No se pudo crear la ventana" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    glewExperimental = GL_TRUE;
    GLenum glewError = glewInit();
    if (glewError != GLEW_OK && glewError != GLEW_ERROR_NO_GLX_DISPLAY) {
        std::cerr << "No se pudo inicializar GLEW: " << glewGetErrorString(glewError) << std::endl;
        return -1;
    }
    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

    // ImGuiLayer reemplaza a las ~6 líneas de inicialización de ImGui
    // (CreateContext + los dos backends) que antes vivían sueltas acá.
    ImGuiLayer ui;
    ui.Init(window);

    DebugDraw::Init();
    Renderer::Init();

    glEnable(GL_DEPTH_TEST);

    // ---------- Cámara -- movible a mano con las flechas ----------
    const glm::vec3 CAMERA_POS_INICIAL(0.0f, 4.0f, 9.0f);
    glm::vec3 cameraPosition = CAMERA_POS_INICIAL;
    const glm::vec3 cameraTarget(0.0f, 0.0f, 0.0f);
    const glm::vec3 cameraUp(0.0f, 1.0f, 0.0f);

    float aspect = static_cast<float>(SCREEN_WIDTH) / SCREEN_HEIGHT;
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);

    Engine engine;

    // make_unique + move: el Engine pasa a ser el único dueño de cada
    // objeto. No necesitamos guardarnos ningún puntero crudo acá -- ni
    // Enemy ni la cámara dependen de la posición del Player.
    engine.AddObject(std::make_unique<Player>(window));

    // Este Enemy y este Bullet quedan igual que siempre (cubo, caja de
    // colisión 3D) -- se cruzan en z=1.5 y disparan Enter/Exit por el
    // camino de ColisionAABB de toda la vida.
    engine.AddObject(std::make_unique<Enemy>(glm::vec3(-1.0f, 0.0f, 1.5f)));
    engine.AddObject(std::make_unique<Bullet>(glm::vec3(4.5f, 0.0f, 1.5f), glm::vec3(-1.5f, 0.0f, 0.0f)));

    // Este segundo par, en cambio, prueba el camino NUEVO: se ven como
    // un cuadrado y un círculo chatos (FormaVisual), y colisionan en
    // 2D sobre el plano XZ (TipoColisionador::Plano2D) en vez de con
    // el AABB 3D de siempre -- mismo cruce de trayectorias que antes
    // (Enemy en z=-1.5, Bullet viniendo desde la izquierda por esa
    // misma z), pero ahora pasando por ColisionAABB2D. Enemy y Bullet
    // no necesitaron cambiar una sola línea de código para esto: los
    // dos campos son de GameObject, la clase base.
    auto enemyCuadrado = std::make_unique<Enemy>(glm::vec3(2.0f, 0.0f, -1.5f));
    enemyCuadrado->forma = FormaVisual::Cuadrado;
    enemyCuadrado->colisionador = TipoColisionador::Plano2D;
    engine.AddObject(std::move(enemyCuadrado));

    auto bulletCirculo = std::make_unique<Bullet>(glm::vec3(-4.5f, 0.0f, -1.5f), glm::vec3(1.8f, 0.0f, 0.0f));
    bulletCirculo->forma = FormaVisual::Circulo;
    bulletCirculo->colisionador = TipoColisionador::Plano2D;
    engine.AddObject(std::move(bulletCirculo));

    // Una esfera puramente decorativa: FormaVisual::Esfera para el
    // dibujo, pero TipoColisionador::Caja3D (el valor por defecto) --
    // colisiona como una caja, no como una esfera geométrica real.
    auto esferaDecorativa = std::make_unique<GameObject>();
    esferaDecorativa->position = glm::vec3(0.0f, 2.2f, 0.0f);
    esferaDecorativa->scale    = glm::vec3(1.0f);
    esferaDecorativa->color    = glm::vec3(0.6f, 0.4f, 1.0f);
    esferaDecorativa->forma    = FormaVisual::Esfera;
    esferaDecorativa->tag      = "EsferaDecorativa";
    engine.AddObject(std::move(esferaDecorativa));

    bool showAxes = true;
    bool showAABB = true;

    while (!glfwWindowShouldClose(window)) {

        ui.BeginFrame();

        ImGui::Begin("Controles");
        ImGui::Text("Objetos vivos: %d", (int)engine.GetObjects().size());
        ImGui::Text("WASD mueve al Player (cubo azul)");
        ImGui::Text("Flechas mueven la camara, R la resetea");
        ImGui::Separator();
        ImGui::Checkbox("Ejes", &showAxes);
        ImGui::Checkbox("Cajas AABB (Debug Draw)", &showAABB);
        ImGui::End();

        float deltaTime = GetDeltaTime();

        processInput(window, cameraPosition, CAMERA_POS_INICIAL);

        engine.Update(deltaTime);
        engine.CheckCollisions();

        glm::mat4 view = glm::lookAt(cameraPosition, cameraTarget, cameraUp);

        glClearColor(0.10f, 0.11f, 0.13f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        Renderer::BeginFrame(view, projection);
        engine.Render();

        if (showAxes) {
            DebugDraw::Axes(glm::vec3(0.0f), 1.5f);
        }
        if (showAABB) {
            // Reutilizamos DebugDraw::AABB para "ver" exactamente la
            // caja que usa ColisionAABB.
            for (const auto& obj : engine.GetObjects()) {
                glm::vec3 half = obj->scale * 0.5f;
                DebugDraw::AABB(obj->position - half, obj->position + half, glm::vec3(0.3f, 0.8f, 1.0f));
            }
        }
        DebugDraw::Render(view, projection);

        ui.EndFrame();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    Renderer::Shutdown();
    DebugDraw::Shutdown();
    ui.Shutdown();
    glfwTerminate();
    return 0;

    // Notar lo que NO aparece en ningún lado de este archivo: ni un
    // 'delete', ni un 'new', ni un solo 'if (tipo == Player)'. El
    // Engine se encarga de todo eso -- y cuando 'engine' sale de scope
    // acá abajo, su destructor destruye el vector, que destruye cada
    // unique_ptr, que destruye cada GameObject. RAII de punta a punta.
}
