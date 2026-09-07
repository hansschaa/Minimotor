#include "GameObject.h"
#include "Renderer.h"

void GameObject::Update(float deltaTime) {
    // Integración simple: la posición avanza según la velocidad,
    // escalada por cuánto duró este frame. Al vivir acá, en la clase
    // BASE, cualquier GameObject nuevo se mueve "gratis" en línea recta
    // con solo declarar una velocity -- las clases derivadas pueden
    // llamar a GameObject::Update(deltaTime) y agregar comportamiento
    // propio encima (ver Enemy::Update), o reemplazarlo por completo
    // (ver Player::Update).
    position += velocity * deltaTime;
}

void GameObject::Draw() const {
    // Un solo switch acá, en la clase BASE, alcanza para que CUALQUIER
    // GameObject (de cualquier clase derivada) se pueda ver como
    // cualquiera de las cuatro formas con solo cambiar 'forma' -- no
    // hace falta sobreescribir Draw() en Player/Enemy/Bullet para eso.
    switch (forma) {
        case FormaVisual::Esfera:
            Renderer::DrawSphere(position, rotation, scale, color);
            break;
        case FormaVisual::Cuadrado:
            Renderer::DrawSquare(position, rotation, scale, color);
            break;
        case FormaVisual::Circulo:
            Renderer::DrawCircle(position, rotation, scale, color);
            break;
        case FormaVisual::Cubo:
        default:
            Renderer::DrawCube(position, rotation, scale, color);
            break;
    }
}

void GameObject::OnTriggerEnter3D(GameObject* other) {
    // Comportamiento por defecto: no hacer nada. GameObject no sabe
    // (ni le importa) qué significa "chocar" para un tipo concreto --
    // eso lo decide cada clase derivada que quiera reaccionar.
    (void)other;
}

void GameObject::OnTriggerStay3D(GameObject* other) {
    // Igual que OnTriggerEnter3D: por defecto no hace nada. La mayoría
    // de las clases derivadas ni siquiera necesitan sobreescribir esto
    // -- ver la nota en Enemy::OnTriggerEnter3D sobre por qué conviene
    // evitar trabajo (y sobre todo std::cout) en cada frame de
    // contacto.
    (void)other;
}

void GameObject::OnTriggerExit3D(GameObject* other) {
    // Comportamiento por defecto: no hacer nada.
    (void)other;
}
