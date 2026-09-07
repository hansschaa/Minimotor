#include "Engine.h"
#include "Collision.h"
#include <utility>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>

void Engine::AddObject(std::unique_ptr<GameObject> object) {
    // Antes de mover el objeto adentro del vector, le dejamos un
    // puntero NO dueño de vuelta hacia este Engine -- así, desde
    // adentro de cualquier GameObject (por ejemplo, desde
    // OnTriggerEnter3D, ver Bullet::OnTriggerEnter3D), pueden llamar a
    // engine->Destroy(this) sin que nadie se los tenga que pasar a
    // mano.
    object->engine = this;

    // push_back mueve el unique_ptr adentro del vector -- no copia el
    // GameObject (un unique_ptr no se puede copiar, a propósito: eso
    // es justamente lo que garantiza que tenga un único dueño). Quien
    // llamó a AddObject(std::move(...)) se queda con un unique_ptr
    // vacío (nullptr); el Engine es, desde este momento, el único
    // dueño real del objeto.
    objects.push_back(std::move(object));
}

void Engine::Update(float deltaTime) {
    // Barrido de destrucción diferida -- se procesa ACÁ, al principio
    // del frame, todo lo que se marcó con Destroy() durante el frame
    // ANTERIOR (típicamente desde un OnTriggerEnter3D, en pleno
    // CheckCollisions() del frame pasado). Nunca se borra nada de
    // 'objects' a mitad de un CheckCollisions() ni de este mismo
    // Update(): eso invalidaría los índices i/j que todavía se están
    // usando, o el iterador del propio range-for de más abajo.
    if (!pendingDestroy.empty()) {
        // unordered_set en vez de dejarlo como vector: así, más abajo,
        // consultamos membresía con .count() -- O(1) en promedio -- en
        // vez de recorrer 'pendingDestroy' linealmente por cada objeto
        // sobreviviente.
        std::unordered_set<GameObject*> aDestruir(pendingDestroy.begin(), pendingDestroy.end());
        pendingDestroy.clear();

        // Antes de borrar de verdad, purgamos cualquier referencia a
        // estos objetos del 'enContactoCon' de TODOS los
        // sobrevivientes. Si no lo hiciéramos, el próximo
        // CheckCollisions() compararía el 'enContactoCon' viejo contra
        // el nuevo, encontraría que uno de estos punteros "desapareció
        // sin avisar", y llamaría a OnTriggerExit3D(other) con un
        // puntero colgante (dangling) -- 'other' ya no existiría en
        // memoria en ese momento.
        for (auto& obj : objects) {
            if (aDestruir.count(obj.get()) == 0) {
                for (GameObject* muerto : aDestruir) {
                    obj->enContactoCon.erase(muerto);
                }
            }
        }

        // Sacar los marcados del vector, en dos pasos (el "erase-remove
        // idiom"):
        //
        // 1) std::remove_if reordena 'objects' EN EL LUGAR: mueve todos
        //    los elementos que la lambda considera "para borrar" (la
        //    lambda devuelve true para esos) hacia el final del vector,
        //    y deja los que sobreviven, en orden, al principio. El
        //    vector no cambia de tamaño todavía -- remove_if devuelve
        //    un iterador que apunta a dónde empieza esa "cola" de
        //    elementos ya movidos (y ahora inútiles).
        // 2) objects.erase(ese_iterador, objects.end()) recién ahí
        //    borra físicamente esa cola. Como los elementos son
        //    unique_ptr<GameObject>, borrarlos dispara su destructor
        //    (RAII) y liberan el GameObject -- sin un solo delete
        //    explícito en todo este archivo.
        objects.erase(
            std::remove_if(objects.begin(), objects.end(),
                [&aDestruir](const std::unique_ptr<GameObject>& obj) {
                    return aDestruir.count(obj.get()) > 0; // true = hay que borrarlo
                }),
            objects.end());
    }

    // Un solo loop, una sola línea de lógica -- y sin embargo, cada
    // objeto se actualiza distinto: object->Update(...)
    // llama a la versión de Update() del tipo REAL de cada objeto
    // (Player, Enemy o Bullet), no a la de GameObject, gracias a que
    // Update() es virtual. Esto es polimorfismo en acción: el Engine
    // no sabe (ni necesita saber) qué tipo es cada elemento.
    for (auto& object : objects) {
        object->Update(deltaTime);
    }
}

void Engine::CheckCollisions() {
    // O(n²): comparamos cada par de objetos una sola vez (j empieza en
    // i+1, no en 0). Para cada par que se superpone, además
    // necesitamos saber si el contacto es NUEVO (Enter) o si ya estaba
    // pasando el frame anterior (Stay) -- y eso lo resolvemos
    // consultando 'a->enContactoCon.count(b)': una búsqueda hash, O(1)
    // en promedio, NUNCA un for que recorra el unordered_set entero.
    //
    // 'contactosEsteFrame' es un mapa auxiliar y temporal (solo vive
    // durante esta función) donde vamos anotando, para cada objeto,
    // con quiénes está en contacto ESTE frame.
    std::unordered_map<GameObject*, std::unordered_set<GameObject*>> contactosEsteFrame;

    for (size_t i = 0; i < objects.size(); i++) {
        for (size_t j = i + 1; j < objects.size(); j++) {
            GameObject* a = objects[i].get();
            GameObject* b = objects[j].get();

            // Dos tests posibles según el TipoColisionador de cada
            // objeto: si AMBOS son planos (Cuadrado/Circulo, viven
            // apoyados en el piso), la comparación es en 2D (ignora
            // Y). Cualquier otro caso -- los dos con volumen, o una
            // mezcla -- usa el AABB de siempre en 3D.
            bool ambosPlanos = a->colisionador == TipoColisionador::Plano2D &&
                                b->colisionador == TipoColisionador::Plano2D;
            bool colisionan = ambosPlanos
                ? ColisionAABB2D(a->position, a->scale, b->position, b->scale)
                : ColisionAABB(a->position, a->scale, b->position, b->scale);

            if (colisionan) {
                contactosEsteFrame[a].insert(b);
                contactosEsteFrame[b].insert(a);

                // En este punto, a->enContactoCon todavía tiene los
                // contactos del frame ANTERIOR (recién se actualiza al
                // final de la función, más abajo) -- por eso alcanza
                // este único .count() para distinguir Enter de Stay.
                if (a->enContactoCon.count(b) == 0) {
                    a->OnTriggerEnter3D(b);
                    b->OnTriggerEnter3D(a);
                } else {
                    a->OnTriggerStay3D(b);
                    b->OnTriggerStay3D(a);
                }
            }
        }
    }

    // Segunda pasada (Exit): cualquier objeto que estaba en
    // el 'enContactoCon' del frame anterior y NO aparece en
    // 'contactosEsteFrame[o]' (el de este frame) dejó de tocarse.
    // Volvemos a apoyarnos en .count() sobre un unordered_set -- O(1)
    // en promedio -- en vez de comparar los dos conjuntos elemento a
    // elemento con dos for anidados.
    for (auto& obj : objects) {
        GameObject* o = obj.get();
        const std::unordered_set<GameObject*>& nuevos = contactosEsteFrame[o];

        for (GameObject* viejo : o->enContactoCon) {
            if (nuevos.count(viejo) == 0) {
                o->OnTriggerExit3D(viejo);
            }
        }

        // 'o' queda listo para el próximo frame.
        o->enContactoCon = nuevos;
    }
}

void Engine::Render() const {
    // Mismo patrón que Update(): un loop, una llamada virtual, cero
    // "if (tipo == Player)".
    for (const auto& object : objects) {
        object->Draw();
    }
}

void Engine::Destroy(GameObject* obj) {
    // Solo anotamos la intención -- ver la explicación completa en
    // Engine::Update(), donde se procesa de verdad.
    pendingDestroy.push_back(obj);
}
