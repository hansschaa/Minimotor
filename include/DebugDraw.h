#pragma once
#include <glm/glm.hpp>

// ============================================================
// Debug Draw — módulo de visualización de geometría de debug.
//
// Esto NO es ImGui. ImGui dibuja una interfaz 2D flotante (paneles,
// sliders, checkboxes) encima de la escena. Este módulo dibuja
// GEOMETRÍA REAL, dentro del mundo 3D: líneas, flechas, ejes, cajas
// de colisión (AABB) y esferas de colisión, todo en wireframe, usando
// las mismas matrices View y Projection que usa el resto de la escena.
//
// ImGui nunca llama a estas funciones directamente. El programa
// (main.cpp) es quien decide, según el estado de unos checkboxes de
// ImGui (booleans de C++, nada más), si llama o no a DebugDraw::Axes(),
// DebugDraw::AABB(), etc.
// ============================================================
namespace DebugDraw {

    // Crea el VAO/VBO y compila el shader propio de Debug Draw.
    // Se llama UNA sola vez, después de inicializar OpenGL/GLEW
    // (nunca dentro del while principal).
    void Init();

    // Libera los recursos de OpenGL. Se llama UNA sola vez, antes de
    // glfwTerminate().
    void Shutdown();

    // ---- Funciones que ACUMULAN geometría (todavía no dibujan nada) ----

    // La primitiva más simple de todo el módulo: un segmento entre dos
    // puntos del mundo. Todo lo demás (Arrow, Axes, AABB, Sphere) es,
    // en el fondo, un conjunto de llamadas a Line().
    void Line(const glm::vec3& start, const glm::vec3& end, const glm::vec3& color);

    // Un vector dibujado como flecha: sale de 'origin', en la dirección
    // 'dir' (no hace falta pasarla normalizada, Arrow la normaliza
    // adentro), con el largo VISUAL 'length'.
    void Arrow(const glm::vec3& origin, const glm::vec3& dir, float length, const glm::vec3& color);

    // Los 3 ejes (X rojo, Y verde, Z azul) saliendo de 'position'.
    void Axes(const glm::vec3& position, float size);

    // Caja alineada a los ejes: se define con solo dos puntos (min y max)
    // y Debug Draw reconstruye las 8 esquinas y traza las 12 aristas.
    void AABB(const glm::vec3& min, const glm::vec3& max, const glm::vec3& color);

    // Esfera de colisión aproximada con 3 círculos perpendiculares
    // (planos XY, XZ, YZ) — no es una malla real, es una aproximación
    // visual barata de calcular.
    void Sphere(const glm::vec3& center, float radius, const glm::vec3& color);

    // ---- Dibuja TODO lo acumulado en este frame y vacía el buffer ----
    // Se llama UNA vez por frame, después de renderizar la escena y
    // antes de ImGui::Render().
    void Render(const glm::mat4& view, const glm::mat4& projection);

}
