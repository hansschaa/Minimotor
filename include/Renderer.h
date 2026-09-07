#pragma once
#include <glm/glm.hpp>

// ============================================================
// Renderer: encapsula el VAO/VBO de cada malla, el vertex/fragment
// shader y las matrices View/Projection detrás de unas pocas
// funciones. La idea es que GameObject::Draw() pueda pedir "dibujame
// un cubo acá, con este color" sin tener que repetir 40 líneas de
// OpenGL en cada clase derivada.
//
// Ninguna otra clase (GameObject, Engine, Player, Enemy, Bullet)
// necesita saber CÓMO dibuja Renderer -- solo llaman a
// Renderer::DrawCube(...), el mismo principio de "ocultar detalles
// detrás de una función con nombre claro" que ya usa DebugDraw.
//
// Todas las formas que dibuja este módulo (DrawCube / DrawSphere /
// DrawSquare / DrawCircle) comparten el MISMO shader y el mismo
// pipeline: viven en el mundo 3D y se ven a través de la cámara (view +
// projection en perspectiva). DrawCube/DrawSphere son mallas "con
// volumen"; DrawSquare/DrawCircle son chatas, apoyadas en el plano XZ
// (como una hoja de papel en el piso) -- pero en los cuatro casos la
// cámara las afecta igual: se ven más chicas a la distancia y pueden
// quedar tapadas detrás de otro GameObject. No hay ninguna forma "de
// pantalla" que se dibuje aparte de la cámara -- todo lo que pinta
// Renderer es un objeto más de la escena.
// ============================================================
namespace Renderer {

    // Compila el shader 3D y arma el VAO/VBO del cubo y de la esfera.
    // Se llama UNA sola vez, al principio de main() (después de crear
    // el contexto de OpenGL), igual que DebugDraw::Init().
    void Init();

    // Se llama UNA vez por frame, antes de dibujar cualquier
    // GameObject: sube 'view' y 'projection' al shader 3D (las mismas
    // matrices para TODOS los objetos del frame).
    void BeginFrame(const glm::mat4& view, const glm::mat4& projection);

    // Arma la matriz Modelo a partir de position/rotation/scale
    // (traslación, rotación en los 3 ejes en grados, escala -- en ese
    // orden, T * R * S), la sube al shader junto con 'color', y dibuja
    // el cubo. Cada GameObject llama
    // a esto una vez por frame, desde su propio Draw().
    void DrawCube(const glm::vec3& position, const glm::vec3& rotationDeg,
                  const glm::vec3& scale, const glm::vec3& color);

    // Igual que DrawCube, pero con una malla esférica (UV sphere) en
    // vez de un cubo. 'scale' funciona igual que en DrawCube: una
    // esfera de scale (1,1,1) tiene diámetro 1 -- el mismo tamaño
    // "unitario" que el cubo, para que sea fácil combinarlas.
    void DrawSphere(const glm::vec3& position, const glm::vec3& rotationDeg,
                     const glm::vec3& scale, const glm::vec3& color);

    // Un cuadrado y un círculo chatos, apoyados en el plano XZ (normal
    // hacia +Y, igual que el piso en el que se mueven Player/Enemy/
    // Bullet). Usan 'position'/'rotationDeg'/'scale'/'color' con
    // exactamente el mismo significado que DrawCube/DrawSphere -- de
    // hecho reusan el mismo shader -- por eso la cámara en perspectiva
    // los afecta igual que a cualquier otro objeto: se ven más chicos
    // lejos y pueden quedar ocultos detrás de otro GameObject. Pensalos
    // como "un cubo/esfera aplastados".

    void DrawSquare(const glm::vec3& position, const glm::vec3& rotationDeg,
                     const glm::vec3& scale, const glm::vec3& color);

    void DrawCircle(const glm::vec3& position, const glm::vec3& rotationDeg,
                     const glm::vec3& scale, const glm::vec3& color);

    void Shutdown();
}
