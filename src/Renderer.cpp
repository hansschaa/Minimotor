#include "Renderer.h"
#include <GL/glew.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>
#include <vector>
#include <cmath>
#include <iostream>

namespace {
    // ---------------- Pipeline 3D (cubo + esfera) ----------------
    unsigned int VAO = 0, VBO = 0, shaderProgram = 0;
    unsigned int sphereVAO = 0, sphereVBO = 0;
    int sphereVertexCount = 0;

    // ---------------- Formas planas EN EL MUNDO 3D ----------------
    // Mismo shader/pipeline que el cubo y la esfera (declarados arriba):
    // solo agregan geometría nueva, nada de shaders nuevos.
    unsigned int flatSquareVAO = 0, flatSquareVBO = 0;
    unsigned int flatCircleVAO = 0, flatCircleVBO = 0;
    int flatCircleVertexCount = 0;

    struct Vertice { float x, y, z; };

    void CheckShaderCompile(unsigned int shader, const char* nombre) {
        int ok;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
        if (!ok) {
            char log[1024];
            glGetShaderInfoLog(shader, 1024, nullptr, log);
            std::cerr << "Error compilando " << nombre << ":\n" << log << std::endl;
        }
    }
    void CheckProgramLink(unsigned int program) {
        int ok;
        glGetProgramiv(program, GL_LINK_STATUS, &ok);
        if (!ok) {
            char log[1024];
            glGetProgramInfoLog(program, 1024, nullptr, log);
            std::cerr << "Error linkeando el shader program:\n" << log << std::endl;
        }
    }

    // Genera una esfera UV (paralelos x meridianos) como una lista
    // plana de triángulos -- mismo estilo "sin índices, sin EBO" que ya
    // usa el cubo, para que la geometría se lea de la misma forma.
    // 'radius' = 0.5 le da a la esfera diámetro 1, el mismo tamaño
    // "unitario" que el cubo (que va de -0.5 a 0.5 en cada eje).
    std::vector<Vertice> GenerarEsfera(int stacks, int slices, float radius) {
        std::vector<Vertice> verts;
        verts.reserve(stacks * slices * 6);

        for (int i = 0; i < stacks; i++) {
            float lat0 = glm::pi<float>() * (-0.5f + static_cast<float>(i) / stacks);
            float lat1 = glm::pi<float>() * (-0.5f + static_cast<float>(i + 1) / stacks);
            float y0 = std::sin(lat0), r0 = std::cos(lat0);
            float y1 = std::sin(lat1), r1 = std::cos(lat1);

            for (int j = 0; j < slices; j++) {
                float lon0 = 2.0f * glm::pi<float>() * static_cast<float>(j) / slices;
                float lon1 = 2.0f * glm::pi<float>() * static_cast<float>(j + 1) / slices;
                float x0 = std::cos(lon0), z0 = std::sin(lon0);
                float x1 = std::cos(lon1), z1 = std::sin(lon1);

                glm::vec3 p00 = radius * glm::vec3(r0 * x0, y0, r0 * z0);
                glm::vec3 p01 = radius * glm::vec3(r0 * x1, y0, r0 * z1);
                glm::vec3 p10 = radius * glm::vec3(r1 * x0, y1, r1 * z0);
                glm::vec3 p11 = radius * glm::vec3(r1 * x1, y1, r1 * z1);

                // Dos triangulos por cada "cuadradito" de la grilla
                // latitud/longitud -- el mismo patron que ya usa el
                // cubo para cada una de sus 6 caras.
                verts.push_back({p00.x, p00.y, p00.z});
                verts.push_back({p10.x, p10.y, p10.z});
                verts.push_back({p11.x, p11.y, p11.z});

                verts.push_back({p00.x, p00.y, p00.z});
                verts.push_back({p11.x, p11.y, p11.z});
                verts.push_back({p01.x, p01.y, p01.z});
            }
        }
        return verts;
    }

    // Cuadrado chato de lado 1, apoyado en el plano XZ (y = 0, normal
    // hacia +Y) -- las mismas dos coordenadas que un piso, con la
    // "altura" siempre en cero. Con scale (1,1,1) queda del mismo
    // tamaño "unitario" que el cubo, para que sea facil combinarlos.
    std::vector<Vertice> GenerarCuadradoPlano() {
        return {
            {-0.5f, 0.0f, -0.5f}, { 0.5f, 0.0f, -0.5f}, { 0.5f, 0.0f, 0.5f},
            {-0.5f, 0.0f, -0.5f}, { 0.5f, 0.0f,  0.5f}, {-0.5f, 0.0f, 0.5f},
        };
    }

    // Circulo chato (triangle fan) de radio 'radius', tambien apoyado
    // en el plano XZ -- mismo patron que GenerarCirculoUnitario, pero
    // con Y fijo en 0 en vez de Z, y en 3D en vez de 2D.
    std::vector<Vertice> GenerarCirculoPlano(int segments, float radius) {
        std::vector<Vertice> verts;
        verts.reserve(segments + 2);
        verts.push_back({0.0f, 0.0f, 0.0f}); // centro
        for (int i = 0; i <= segments; i++) {
            float angle = 2.0f * glm::pi<float>() * static_cast<float>(i) / segments;
            verts.push_back({radius * std::cos(angle), 0.0f, radius * std::sin(angle)});
        }
        return verts;
    }
}

void Renderer::Init() {
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    // Cubo de lado 1, centrado en el origen -- la misma geometría de
    // siempre, sin colores por vértice: el color de cada GameObject se
    // manda como uniform (un color por objeto, no por cara).
    std::vector<Vertice> vertices = {
        {-0.5f,-0.5f, 0.5f}, { 0.5f,-0.5f, 0.5f}, { 0.5f, 0.5f, 0.5f},
        {-0.5f,-0.5f, 0.5f}, { 0.5f, 0.5f, 0.5f}, {-0.5f, 0.5f, 0.5f},

        { 0.5f,-0.5f,-0.5f}, {-0.5f,-0.5f,-0.5f}, {-0.5f, 0.5f,-0.5f},
        { 0.5f,-0.5f,-0.5f}, {-0.5f, 0.5f,-0.5f}, { 0.5f, 0.5f,-0.5f},

        {-0.5f,-0.5f,-0.5f}, {-0.5f,-0.5f, 0.5f}, {-0.5f, 0.5f, 0.5f},
        {-0.5f,-0.5f,-0.5f}, {-0.5f, 0.5f, 0.5f}, {-0.5f, 0.5f,-0.5f},

        { 0.5f,-0.5f, 0.5f}, { 0.5f,-0.5f,-0.5f}, { 0.5f, 0.5f,-0.5f},
        { 0.5f,-0.5f, 0.5f}, { 0.5f, 0.5f,-0.5f}, { 0.5f, 0.5f, 0.5f},

        {-0.5f, 0.5f, 0.5f}, { 0.5f, 0.5f, 0.5f}, { 0.5f, 0.5f,-0.5f},
        {-0.5f, 0.5f, 0.5f}, { 0.5f, 0.5f,-0.5f}, {-0.5f, 0.5f,-0.5f},

        {-0.5f,-0.5f,-0.5f}, { 0.5f,-0.5f,-0.5f}, { 0.5f,-0.5f, 0.5f},
        {-0.5f,-0.5f,-0.5f}, { 0.5f,-0.5f, 0.5f}, {-0.5f,-0.5f, 0.5f},
    };

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertice), vertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertice), (void*)0);
    glEnableVertexAttribArray(0);

    // ---------------- Esfera: mismo shader 3D, otra malla ----------------
    // 16 paralelos x 24 meridianos: suficientemente redonda sin
    // desperdiciar vertices para el tamano en el que se ve en pantalla.
    std::vector<Vertice> sphereVerts = GenerarEsfera(16, 24, 0.5f);
    sphereVertexCount = static_cast<int>(sphereVerts.size());

    glGenVertexArrays(1, &sphereVAO);
    glBindVertexArray(sphereVAO);
    glGenBuffers(1, &sphereVBO);
    glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
    glBufferData(GL_ARRAY_BUFFER, sphereVerts.size() * sizeof(Vertice), sphereVerts.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertice), (void*)0);
    glEnableVertexAttribArray(0);

    // ---------------- Cuadrado y circulo planos: mismo shader 3D ----------------
    // Van en el mismo Init() que el cubo/la esfera (no en Init2D):
    // usan 'shaderProgram', el mismo de siempre, porque son objetos del
    // mundo, no del overlay de pantalla.
    std::vector<Vertice> flatSquareVerts = GenerarCuadradoPlano();
    glGenVertexArrays(1, &flatSquareVAO);
    glBindVertexArray(flatSquareVAO);
    glGenBuffers(1, &flatSquareVBO);
    glBindBuffer(GL_ARRAY_BUFFER, flatSquareVBO);
    glBufferData(GL_ARRAY_BUFFER, flatSquareVerts.size() * sizeof(Vertice), flatSquareVerts.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertice), (void*)0);
    glEnableVertexAttribArray(0);

    std::vector<Vertice> flatCircleVerts = GenerarCirculoPlano(32, 0.5f);
    flatCircleVertexCount = static_cast<int>(flatCircleVerts.size());
    glGenVertexArrays(1, &flatCircleVAO);
    glBindVertexArray(flatCircleVAO);
    glGenBuffers(1, &flatCircleVBO);
    glBindBuffer(GL_ARRAY_BUFFER, flatCircleVBO);
    glBufferData(GL_ARRAY_BUFFER, flatCircleVerts.size() * sizeof(Vertice), flatCircleVerts.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertice), (void*)0);
    glEnableVertexAttribArray(0);

    const char* vertexShaderSrc = R"(
        #version 330 core
        layout (location = 0) in vec3 aPos;
        uniform mat4 model;
        uniform mat4 view;
        uniform mat4 projection;
        void main() {
            gl_Position = projection * view * model * vec4(aPos, 1.0);
        }
    )";
    const char* fragmentShaderSrc = R"(
        #version 330 core
        out vec4 FragColor;
        uniform vec3 color;
        void main() {
            FragColor = vec4(color, 1.0);
        }
    )";

    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSrc, nullptr);
    glCompileShader(vertexShader);
    CheckShaderCompile(vertexShader, "vertex shader (Renderer)");

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSrc, nullptr);
    glCompileShader(fragmentShader);
    CheckShaderCompile(fragmentShader, "fragment shader (Renderer)");

    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    CheckProgramLink(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

void Renderer::BeginFrame(const glm::mat4& view, const glm::mat4& projection) {
    glUseProgram(shaderProgram);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
}

void Renderer::DrawCube(const glm::vec3& position, const glm::vec3& rotationDeg,
                         const glm::vec3& scale, const glm::vec3& color) {
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);
    model = glm::rotate(model, glm::radians(rotationDeg.x), glm::vec3(1, 0, 0));
    model = glm::rotate(model, glm::radians(rotationDeg.y), glm::vec3(0, 1, 0));
    model = glm::rotate(model, glm::radians(rotationDeg.z), glm::vec3(0, 0, 1));
    model = glm::scale(model, scale);

    glUseProgram(shaderProgram);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    glUniform3fv(glGetUniformLocation(shaderProgram, "color"), 1, glm::value_ptr(color));

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
}

void Renderer::DrawSphere(const glm::vec3& position, const glm::vec3& rotationDeg,
                           const glm::vec3& scale, const glm::vec3& color) {
    // Exactamente el mismo armado de matriz Modelo que DrawCube -- lo
    // unico que cambia es que binding y que malla se dibuja. La
    // rotacion no tiene efecto visual en una esfera perfecta, pero se
    // deja por consistencia con el resto de las funciones Draw*.
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);
    model = glm::rotate(model, glm::radians(rotationDeg.x), glm::vec3(1, 0, 0));
    model = glm::rotate(model, glm::radians(rotationDeg.y), glm::vec3(0, 1, 0));
    model = glm::rotate(model, glm::radians(rotationDeg.z), glm::vec3(0, 0, 1));
    model = glm::scale(model, scale);

    glUseProgram(shaderProgram);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    glUniform3fv(glGetUniformLocation(shaderProgram, "color"), 1, glm::value_ptr(color));

    glBindVertexArray(sphereVAO);
    glDrawArrays(GL_TRIANGLES, 0, sphereVertexCount);
}

void Renderer::DrawSquare(const glm::vec3& position, const glm::vec3& rotationDeg,
                           const glm::vec3& scale, const glm::vec3& color) {
    // Exactamente el mismo armado de matriz Modelo, el mismo shader y
    // el mismo BeginFrame que DrawCube/DrawSphere -- lo unico que
    // cambia es la malla: un cuadrado chato en vez de un volumen. Por
    // eso la camara en perspectiva SI lo afecta (se achica de lejos,
    // se puede tapar), a diferencia de DrawSquare2D.
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);
    model = glm::rotate(model, glm::radians(rotationDeg.x), glm::vec3(1, 0, 0));
    model = glm::rotate(model, glm::radians(rotationDeg.y), glm::vec3(0, 1, 0));
    model = glm::rotate(model, glm::radians(rotationDeg.z), glm::vec3(0, 0, 1));
    model = glm::scale(model, scale);

    glUseProgram(shaderProgram);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    glUniform3fv(glGetUniformLocation(shaderProgram, "color"), 1, glm::value_ptr(color));

    glBindVertexArray(flatSquareVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void Renderer::DrawCircle(const glm::vec3& position, const glm::vec3& rotationDeg,
                           const glm::vec3& scale, const glm::vec3& color) {
    // Ver el comentario de DrawSquare -- mismo shader, misma matriz
    // Modelo, unico cambio es la malla (triangle fan, apoyado en XZ).
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);
    model = glm::rotate(model, glm::radians(rotationDeg.x), glm::vec3(1, 0, 0));
    model = glm::rotate(model, glm::radians(rotationDeg.y), glm::vec3(0, 1, 0));
    model = glm::rotate(model, glm::radians(rotationDeg.z), glm::vec3(0, 0, 1));
    model = glm::scale(model, scale);

    glUseProgram(shaderProgram);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    glUniform3fv(glGetUniformLocation(shaderProgram, "color"), 1, glm::value_ptr(color));

    glBindVertexArray(flatCircleVAO);
    glDrawArrays(GL_TRIANGLE_FAN, 0, flatCircleVertexCount);
}

void Renderer::Shutdown() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &sphereVAO);
    glDeleteBuffers(1, &sphereVBO);
    glDeleteVertexArrays(1, &flatSquareVAO);
    glDeleteBuffers(1, &flatSquareVBO);
    glDeleteVertexArrays(1, &flatCircleVAO);
    glDeleteBuffers(1, &flatCircleVBO);
    glDeleteProgram(shaderProgram);
}
