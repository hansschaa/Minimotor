#pragma once
#include "GameObject.h"

// ============================================================
// Bullet: se mueve en línea recta a velocidad constante.
// ============================================================
class Bullet : public GameObject {
public:
    Bullet(glm::vec3 startPosition, glm::vec3 startVelocity);

    void OnTriggerEnter3D(GameObject* other) override;

    // OJO: Bullet NO sobreescribe Update() -- le alcanza con el
    // GameObject::Update() de la clase base (position += velocity *
    // deltaTime). No todas las clases derivadas necesitan sobreescribir
    // todo: solo lo que las hace distintas.
};
