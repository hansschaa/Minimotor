#pragma once
#include <glm/glm.hpp>

// ============================================================
// Colisión AABB en 3D (mínimo/máximo por eje, a partir de una posición
// y la mitad de un tamaño). Usa 'glm::vec3 scale' en vez de un 'float
// size' porque GameObject::scale es un vec3 -- permite un alto/ancho/
// profundidad distintos, no solo cubos.
//
// Recibe posición y escala por separado (no un GameObject*) a
// propósito: así esta función no depende de GameObject, y se puede
// probar o reutilizar de forma completamente aislada. Engine la usa
// para APLICARLA (recorrer pares de GameObject y preguntar si
// colisionan), no para definir la geometría de la colisión.
bool ColisionAABB(const glm::vec3& posA, const glm::vec3& scaleA,
                   const glm::vec3& posB, const glm::vec3& scaleB);

// ============================================================
// Colisión AABB en 2D: exactamente el mismo test que ColisionAABB,
// pero comparando solo X y Z -- Y se ignora por completo. Pensada para
// objetos CHATOS que viven apoyados en el piso (un Cuadrado o un
// Circulo plano, ver FormaVisual en GameObject.h): a esos no les
// importa la altura, así que compararla solo agregaría ruido (dos
// objetos podrían "no tocarse" en 3D por una diferencia de altura
// irrelevante, cuando en el plano en el que realmente viven sí se
// superponen).
//
// Mismo motivo que ColisionAABB para recibir posición/escala sueltas
// en vez de un GameObject*: queda desacoplada y fácil de probar sola.
bool ColisionAABB2D(const glm::vec3& posA, const glm::vec3& scaleA,
                     const glm::vec3& posB, const glm::vec3& scaleB);
