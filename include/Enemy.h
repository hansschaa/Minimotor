#pragma once
#include "GameObject.h"

// ============================================================
// Enemy: patrulla en línea recta y rebota en los bordes.
// ============================================================
class Enemy : public GameObject {
public:
    explicit Enemy(glm::vec3 startPosition);

    void Update(float deltaTime) override;
    void OnTriggerEnter3D(GameObject* other) override;
    void OnTriggerExit3D(GameObject* other) override;

private:
    float limiteX = 3.5f; // patrulla entre -limiteX y +limiteX
};
