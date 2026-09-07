#include "Enemy.h"
#include <iostream>

Enemy::Enemy(glm::vec3 startPosition) {
    position = startPosition;
    velocity = glm::vec3(1.2f, 0.0f, 0.0f);
    scale    = glm::vec3(0.9f);
    color    = glm::vec3(0.9f, 0.2f, 0.2f); // rojo
    tag      = "Enemy"; // permite agruparlos sin dynamic_cast
}

void Enemy::Update(float deltaTime) {
    GameObject::Update(deltaTime);
    if (position.x > limiteX || position.x < -limiteX) {
        velocity.x *= -1.0f;
    }
}

void Enemy::OnTriggerEnter3D(GameObject* other) {
    (void)other;
    color = glm::vec3(1.0f, 1.0f, 0.2f); // destello amarillo
    std::cout << "[Enemy] superposicion detectada." << std::endl;
}

void Enemy::OnTriggerExit3D(GameObject* other) {
    (void)other;
    // Exit es el complemento natural de Enter: acá aprovechamos para
    // volver al color original, ahora que 'other' ya no está en
    // enContactoCon. Fíjense que Enemy NO sobreescribe
    // OnTriggerStay3D: le alcanza con el comportamiento vacío por
    // defecto de GameObject mientras el contacto se mantiene -- entre
    // otras cosas, evita imprimir un mensaje por consola en CADA frame
    // que dos objetos se siguen tocando.
    color = glm::vec3(0.9f, 0.2f, 0.2f); // rojo otra vez
    std::cout << "[Enemy] fin de superposicion." << std::endl;
}
