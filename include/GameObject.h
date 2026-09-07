#pragma once
#include <glm/glm.hpp>
#include <string>
#include <unordered_set>

class Engine; // forward declaration: alcanza con un puntero, no hace
              // falta el header completo de Engine acá.

// Qué dibuja GameObject::Draw() -- independiente de cómo colisiona
// (ver TipoColisionador más abajo): una esfera puede perfectamente
// colisionar como una caja.
enum class FormaVisual { Cubo, Esfera, Cuadrado, Circulo };

// Qué test usa Engine::CheckCollisions() para este objeto:
//   Caja3D -- AABB en X/Y/Z (ColisionAABB). Para objetos con volumen
//             real (Cubo, Esfera).
//   Plano2D -- AABB/círculo en X/Z, ignorando Y (ColisionAABB2D). Para
//              objetos chatos que viven apoyados en el piso (Cuadrado,
//              Circulo).
// Si dos objetos en contacto tienen tipos distintos, Engine usa Caja3D
// (el AABB de siempre) como caso general.
enum class TipoColisionador { Caja3D, Plano2D };

// ============================================================
// GameObject: la clase base de todo el mini-engine.
// ============================================================
// Todo lo que el Engine sabe manejar (Player, Enemy, Bullet, y
// cualquier tipo nuevo que agreguen) hereda de acá. El Engine SOLO
// conoce esta interfaz -- nunca un tipo derivado.
class GameObject {
public:
    glm::vec3 position = glm::vec3(0.0f);
    glm::vec3 rotation = glm::vec3(0.0f);  // grados, uno por eje
    glm::vec3 scale    = glm::vec3(1.0f);
    glm::vec3 velocity = glm::vec3(0.0f);  // unidades de mundo por segundo
    glm::vec3 color    = glm::vec3(1.0f);  // color simple para distinguir tipos a simple vista

    // Identificador para agrupar objetos (por ejemplo, "Enemy", "Player")
    // sin necesitar dynamic_cast ni preguntar el tipo concreto. Cada
    // clase derivada fija el suyo en su constructor.
    std::string tag = "";

    // Por defecto, cubo con caja de colisión 3D -- el mismo
    // comportamiento que tenían todos los objetos antes de que
    // existieran estos dos campos.
    FormaVisual forma = FormaVisual::Cubo;
    TipoColisionador colisionador = TipoColisionador::Caja3D;

    // Puntero NO dueño al Engine que lo administra -- lo fija
    // Engine::AddObject automáticamente. Les permite llamar a
    // engine->Destroy(...) desde el propio GameObject (por ejemplo, desde
    // OnTriggerEnter3D) sin que nadie tenga que pasárselo a mano.
    Engine* engine = nullptr;

    // Contactos AABB activos en el frame actual. Lo administra
    // Engine::CheckCollisions(); no lo modifiquen a mano.
    std::unordered_set<GameObject*> enContactoCon;

    // Update/Draw/OnTrigger* son virtuales (no puros): GameObject ya
    // tiene una implementación por defecto razonable para todas, y cada
    // clase derivada puede quedarse con ese comportamiento o
    // sobreescribirlo (override) si necesita algo distinto.
    virtual void Update(float deltaTime);
    virtual void Draw() const;
    virtual void OnTriggerEnter3D(GameObject* other);
    virtual void OnTriggerStay3D(GameObject* other);
    virtual void OnTriggerExit3D(GameObject* other);

    // Destructor virtual: OBLIGATORIO en cualquier clase base que se
    // vaya a destruir a través de un puntero a la base (que es
    // exactamente lo que hace std::unique_ptr<GameObject> cuando el
    // objeto real es un Enemy o un Bullet). Sin esto, borrar un
    // unique_ptr<GameObject> que en realidad apunta a un Enemy NO
    // llamaría al destructor de Enemy -- solo al de GameObject.
    virtual ~GameObject() = default;
};
