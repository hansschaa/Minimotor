#include "DebugDraw.h"

#include <GL/glew.h>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <cmath>

namespace DebugDraw {

namespace {

    // Un vértice de Debug Draw: posición (x,y,z) + color (r,g,b). Igual
    // que el struct Vertice del cubo en main.cpp, pero acá no hace falta
    // nada más — no hay texturas ni normales, es solo wireframe.
    struct VertexPC {
        float x, y, z;
        float r, g, b;
    };

    // Todas las líneas que se van a dibujar ESTE frame. Se llena con
    // Line/Arrow/Axes/AABB/Sphere y se vacía en Render().
    std::vector<VertexPC> vertices;

    unsigned int vao = 0;
    unsigned int vbo = 0;
    unsigned int shaderProgram = 0;

    // ------------------------------------------------------------
    // Shader propio de Debug Draw. Nótese que NO tiene uniform 'model':
    // a diferencia del cubo, los puntos que le llegan a Line/Arrow/AABB/
    // Sphere ya están en espacio MUNDO (por eso todas piden coordenadas
    // mundiales, no locales) — alcanza con view y projection, las mismas
    // que usa el resto de la escena.
    // ------------------------------------------------------------
    const char* vertexShaderSrc = R"(
        #version 330 core
        layout (location = 0) in vec3 aPos;
        layout (location = 1) in vec3 aColor;

        uniform mat4 view;
        uniform mat4 projection;

        out vec3 vColor;

        void main() {
            gl_Position = projection * view * vec4(aPos, 1.0);
            vColor = aColor;
        }
    )";

    const char* fragmentShaderSrc = R"(
        #version 330 core
        in vec3 vColor;
        out vec4 FragColor;
        void main() {
            FragColor = vec4(vColor, 1.0);
        }
    )";

    unsigned int CompileShader(unsigned int type, const char* src) {
        unsigned int id = glCreateShader(type);
        glShaderSource(id, 1, &src, nullptr);
        glCompileShader(id);
        return id;
    }

    // Agrega un vértice al buffer de este frame. Todas las funciones
    // públicas del módulo (Line, Arrow, Axes, AABB, Sphere) terminan
    // llamando a esto — es el único lugar que toca 'vertices'.
    void PushVertex(const glm::vec3& p, const glm::vec3& c) {
        vertices.push_back({ p.x, p.y, p.z, c.x, c.y, c.z });
    }

} // namespace (anónimo)

void Init() {
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    // Atributo 0: posición. Atributo 1: color. Mismo mecanismo que el
    // VAO/VBO del cubo en main.cpp, con el mismo layout (position, color).
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexPC), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexPC), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShaderSrc);
    unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShaderSrc);

    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vs);
    glAttachShader(shaderProgram, fs);
    glLinkProgram(shaderProgram);

    // Ya unidos al programa, los shaders individuales no hacen falta más.
    glDeleteShader(vs);
    glDeleteShader(fs);
}

void Shutdown() {
    glDeleteProgram(shaderProgram);
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
}

void Line(const glm::vec3& start, const glm::vec3& end, const glm::vec3& color) {
    PushVertex(start, color);
    PushVertex(end, color);
}

void Arrow(const glm::vec3& origin, const glm::vec3& dir, float length, const glm::vec3& color) {
    // Dirección normalizada: si viene un vector "crudo" (como
    // cameraTarget - cameraPosition, que no mide 1), lo normalizamos acá
    // adentro para no obligar a quien llama a acordarse de hacerlo.
    glm::vec3 d = dir;
    float dirLen = glm::length(d);
    if (dirLen > 0.0001f) d = d / dirLen;

    glm::vec3 tip = origin + d * length;
    Line(origin, tip, color);

    // La punta: dos líneas cortas que salen de 'tip', inclinadas hacia
    // atrás usando un vector PERPENDICULAR a la dirección principal. En
    // 3D conseguimos ese perpendicular con un producto cruz contra un
    // vector de referencia que no sea paralelo a 'd' (si 'd' casi
    // coincide con Y, usamos X como referencia en su lugar).
    glm::vec3 reference = (std::fabs(d.y) < 0.99f) ? glm::vec3(0.0f, 1.0f, 0.0f)
                                                    : glm::vec3(1.0f, 0.0f, 0.0f);
    glm::vec3 perp = glm::normalize(glm::cross(d, reference));

    float back = length * 0.25f;
    float side = length * 0.12f;
    glm::vec3 base = tip - d * back;

    Line(tip, base + perp * side, color);
    Line(tip, base - perp * side, color);
}

void Axes(const glm::vec3& position, float size) {
    // Convención de colores: X rojo, Y verde, Z azul.
    Line(position, position + glm::vec3(size, 0.0f, 0.0f), glm::vec3(1.0f, 0.15f, 0.15f)); // X
    Line(position, position + glm::vec3(0.0f, size, 0.0f), glm::vec3(0.2f, 0.85f, 0.2f));  // Y
    Line(position, position + glm::vec3(0.0f, 0.0f, size), glm::vec3(0.25f, 0.45f, 1.0f)); // Z
}

void AABB(const glm::vec3& min, const glm::vec3& max, const glm::vec3& color) {
    // Las 8 esquinas, combinando min/max en cada uno de los 3 ejes: con
    // dos puntos alcanza para describir toda la caja.
    glm::vec3 c[8] = {
        { min.x, min.y, min.z }, { max.x, min.y, min.z },
        { max.x, max.y, min.z }, { min.x, max.y, min.z },
        { min.x, min.y, max.z }, { max.x, min.y, max.z },
        { max.x, max.y, max.z }, { min.x, max.y, max.z },
    };

    // Cara de atrás (z = min.z)
    Line(c[0], c[1], color); Line(c[1], c[2], color);
    Line(c[2], c[3], color); Line(c[3], c[0], color);
    // Cara de adelante (z = max.z)
    Line(c[4], c[5], color); Line(c[5], c[6], color);
    Line(c[6], c[7], color); Line(c[7], c[4], color);
    // Las 4 aristas que conectan ambas caras (profundidad)
    Line(c[0], c[4], color); Line(c[1], c[5], color);
    Line(c[2], c[6], color); Line(c[3], c[7], color);
}

void Sphere(const glm::vec3& center, float radius, const glm::vec3& color) {
    // No dibujamos una malla de esfera real: 3 círculos perpendiculares
    // entre sí, uno por cada plano, son suficientes para que el ojo
    // "complete" la forma esférica.
    const int segments = 32;
    const float TWO_PI = 6.28318530718f;

    for (int i = 0; i < segments; i++) {
        float a0 = TWO_PI * (float)i / segments;
        float a1 = TWO_PI * (float)(i + 1) / segments;

        // Plano XY
        Line(center + glm::vec3(std::cos(a0) * radius, std::sin(a0) * radius, 0.0f),
             center + glm::vec3(std::cos(a1) * radius, std::sin(a1) * radius, 0.0f), color);

        // Plano XZ
        Line(center + glm::vec3(std::cos(a0) * radius, 0.0f, std::sin(a0) * radius),
             center + glm::vec3(std::cos(a1) * radius, 0.0f, std::sin(a1) * radius), color);

        // Plano YZ
        Line(center + glm::vec3(0.0f, std::cos(a0) * radius, std::sin(a0) * radius),
             center + glm::vec3(0.0f, std::cos(a1) * radius, std::sin(a1) * radius), color);
    }
}

void Render(const glm::mat4& view, const glm::mat4& projection) {
    // Si nadie llamó a Line/Arrow/Axes/AABB/Sphere este frame (por
    // ejemplo, con todos los checkboxes de ImGui apagados), no hay nada
    // que dibujar.
    if (vertices.empty()) return;

    glUseProgram(shaderProgram);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"),
                        1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"),
                        1, GL_FALSE, glm::value_ptr(projection));

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    // GL_DYNAMIC_DRAW (a diferencia del GL_STATIC_DRAW del cubo): este
    // buffer cambia de contenido en cada frame, porque la geometría de
    // debug se recalcula todo el tiempo.
    glBufferData(GL_ARRAY_BUFFER,
                 vertices.size() * sizeof(VertexPC),
                 vertices.data(),
                 GL_DYNAMIC_DRAW);

    glLineWidth(2.0f);
    glDrawArrays(GL_LINES, 0, (GLsizei)vertices.size());

    // Vaciamos el buffer: Debug Draw es temporal por diseño -- lo que
    // no se vuelva a pedir el próximo frame, no se vuelve a dibujar.
    vertices.clear();
}

} // namespace DebugDraw
