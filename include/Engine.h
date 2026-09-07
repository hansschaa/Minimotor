#pragma once
#include <vector>
#include <memory>
#include "GameObject.h"

// ============================================================
// Engine: administración centralizada de los GameObject.
// ============================================================
// El Engine es dueño (owner) de todos los objetos del juego: nadie más
// que él los guarda, y cuando el Engine se destruye, todos sus objetos
// se destruyen con él -- automáticamente, sin que nadie tenga que
// escribir un solo delete.
//
// A propósito, el Engine NO tiene ningún método que pregunte "¿sos un
// Player?" o "¿sos un Enemy?": trabaja exclusivamente contra la
// interfaz de GameObject (Update/Draw/OnTrigger*). Eso es lo que
// permite agregar un tipo de objeto nuevo sin tocar ni una línea de
// esta clase.
class Engine {
public:
    // Recibe la propiedad (ownership) de 'object': quien llama a esta
    // función ya no debería seguir usando ese unique_ptr después de
    // pasarlo (por eso se recibe por VALOR y quien llama tiene que
    // usar std::move).
    void AddObject(std::unique_ptr<GameObject> object);

    void Update(float deltaTime);
    void CheckCollisions();
    void Render() const;

    // Marca 'obj' para destrucción: NO lo saca de 'objects' en el
    // momento -- eso sería peligroso si Destroy()
    // se llama desde adentro de un OnTriggerEnter3D, que a su vez se
    // llama desde adentro del loop de CheckCollisions() que todavía
    // está iterando sobre 'objects'. Solo lo anota en
    // 'pendingDestroy'; la eliminación real ocurre al principio del
    // próximo Engine::Update().
    void Destroy(GameObject* obj);

    // Acceso de solo lectura, para poder mostrar información (cantidad
    // de objetos) y dibujar las cajas AABB de debug desde main() sin
    // que el Engine deje de ser el único dueño de sus objetos.
    const std::vector<std::unique_ptr<GameObject>>& GetObjects() const { return objects; }

private:
    // std::vector<std::unique_ptr<GameObject>>: un vector de "dueños
    // únicos". Puede guardar Player, Enemy, Bullet -- cualquier cosa
    // que herede de GameObject -- todo mezclado en el mismo vector,
    // gracias al polimorfismo. Cada unique_ptr garantiza que el objeto
    // que apunta tiene UN SOLO dueño (este vector), y que se destruye
    // automáticamente (RAII) cuando se borra del vector o cuando el
    // vector mismo se destruye.
    std::vector<std::unique_ptr<GameObject>> objects;

    // Objetos marcados con Destroy() en el frame anterior, pendientes
    // de ser sacados de 'objects' -- ver Engine::Update().
    std::vector<GameObject*> pendingDestroy;
};
