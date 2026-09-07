#include "Player.h"
#include <GLFW/glfw3.h>
#include <iostream>

Player::Player(GLFWwindow* window) : window(window) {
    position = glm::vec3(-3.0f, 0.0f, 0.0f);
    scale    = glm::vec3(0.8f);
    color    = glm::vec3(0.2f, 0.5f, 1.0f); // azul
    tag      = "Player";
}

void Player::Update(float deltaTime) {
    // Lee el teclado directamente adentro de la clase: Player es dueño
    // de su propio input. Movemos sobre el plano X/Z (Q/E quedan libres
    // por si quieren agregar altura como desafío).
    velocity = glm::vec3(0.0f);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) velocity.z -= speed;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) velocity.z += speed;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) velocity.x -= speed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) velocity.x += speed;

    // Reutilizamos la integración de movimiento de la clase base
    // (position += velocity * deltaTime) en vez de reescribirla acá:
    // Player solo decide CUÁNTO vale 'velocity' este frame, no CÓMO se
    // usa para mover -- esa parte ya la resuelve GameObject::Update.
    GameObject::Update(deltaTime);
}

void Player::OnTriggerEnter3D(GameObject* other) {
    (void)other;
    std::cout << "[Player] superposicion detectada." << std::endl;
}
