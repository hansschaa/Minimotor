#pragma once
#include "GameObject.h"

struct GLFWwindow; // forward declaration: alcanza con un puntero, no hace
                    // falta el header completo de GLFW acá.

// ============================================================
// Player: un GameObject controlado por teclado.
// ============================================================
class Player : public GameObject {
public:
    explicit Player(GLFWwindow* window);

    void Update(float deltaTime) override;
    void OnTriggerEnter3D(GameObject* other) override;

private:
    GLFWwindow* window; // para leer el teclado en Update()
    float speed = 3.0f; // unidades de mundo por segundo
};
