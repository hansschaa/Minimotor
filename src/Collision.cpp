#include "Collision.h"

bool ColisionAABB(const glm::vec3& posA, const glm::vec3& scaleA,
                   const glm::vec3& posB, const glm::vec3& scaleB) {
    glm::vec3 aMin = posA - scaleA * 0.5f;
    glm::vec3 aMax = posA + scaleA * 0.5f;
    glm::vec3 bMin = posB - scaleB * 0.5f;
    glm::vec3 bMax = posB + scaleB * 0.5f;

    bool superponeX = aMin.x < bMax.x && aMax.x > bMin.x;
    bool superponeY = aMin.y < bMax.y && aMax.y > bMin.y;
    bool superponeZ = aMin.z < bMax.z && aMax.z > bMin.z;
    return superponeX && superponeY && superponeZ;
}

bool ColisionAABB2D(const glm::vec3& posA, const glm::vec3& scaleA,
                     const glm::vec3& posB, const glm::vec3& scaleB) {
    glm::vec3 aMin = posA - scaleA * 0.5f;
    glm::vec3 aMax = posA + scaleA * 0.5f;
    glm::vec3 bMin = posB - scaleB * 0.5f;
    glm::vec3 bMax = posB + scaleB * 0.5f;

    // Mismo cálculo que ColisionAABB, salvo que Y ni se mira.
    bool superponeX = aMin.x < bMax.x && aMax.x > bMin.x;
    bool superponeZ = aMin.z < bMax.z && aMax.z > bMin.z;
    return superponeX && superponeZ;
}
