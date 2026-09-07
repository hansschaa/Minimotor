#include "Bullet.h"
#include "Engine.h"
#include <iostream>

Bullet::Bullet(glm::vec3 startPosition, glm::vec3 startVelocity) {
    position = startPosition;
    velocity = startVelocity;
    scale    = glm::vec3(0.3f);
    color    = glm::vec3(1.0f, 0.85f, 0.2f); // amarillo
    tag      = "Bullet";
}

void Bullet::OnTriggerEnter3D(GameObject* other) {
    (void)other;
    std::cout << "[Bullet] superposicion detectada -- me destruyo." << std::endl;

    // Uso natural de Engine::Destroy -- la Bullet se saca a sí misma
    // del Engine apenas toca algo, en vez de quedarse dando vueltas
    // para siempre. 'engine' es el puntero que Engine::AddObject dejó
    // cargado; 'this' sigue siendo válido durante el resto de este
    // frame -- el objeto recién se borra de verdad al principio del
    // próximo Engine::Update().
    engine->Destroy(this);
}
