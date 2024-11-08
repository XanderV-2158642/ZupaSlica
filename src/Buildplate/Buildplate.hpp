#ifndef BUILDPLATE_H
#define BUILDPLATE_H

#include <glm/glm.hpp>
#include <glad/glad.h>
#include <vector>
#include "../Mesh/Mesh.hpp"
#include "../Camera/Camera.hpp"
#include "../Shader/Shader.hpp"

class Buildplate {
public:
    static std::vector<Vertex> GetVertices();
    static std::vector<unsigned int> GetIndices();
    static void Draw(Shader shader, Mesh buildPlate, glm::mat4 view, glm::mat4 projection, Camera camera);
};


std::vector<Vertex> Buildplate::GetVertices() {
    Vertex vertices[] = {
        Vertex{glm::vec3(-0.5f, 0.0f,  0.5f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.0f, 0.0f)},
        Vertex{glm::vec3(-0.5f, 0.0f, -0.5f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.0f, 1.0f)},
        Vertex{glm::vec3(0.5f, 0.0f, -0.5f), glm::vec3(0.0f, 1.0f, 0.0f),  glm::vec2(1.0f, 1.0f)},
        Vertex{glm::vec3(0.5f, 0.0f,  0.5f), glm::vec3(0.0f, 1.0f, 0.0f),  glm::vec2(1.0f, 0.0f)}
    };

    std::vector<Vertex> verts(vertices, vertices + sizeof(vertices) / sizeof(Vertex));
    return verts;
}


std::vector<unsigned int> Buildplate::GetIndices() {
    vector<unsigned int> planeIndices = {
        0, 1, 2,
        0, 2, 3
    };

    return planeIndices;
}

void Buildplate::Draw(Shader shader, Mesh buildPlate, glm::mat4 view, glm::mat4 projection, Camera camera) {
    shader.use();
    //light properties
    shader.setVec3("objectColor", 0.5f, 0.7f, 0.5f);
    shader.setVec3("lightColor", 1.0f, 1.0f, 1.0f);
    shader.setVec3("lightPos", camera.Position);
    shader.setVec3("viewPos", camera.Position);

    shader.setMat4("view", view);
    shader.setMat4("projection", projection);

    glm::mat4 model = glm::mat4(1.0f);
    //scale to buildplate dimensions
    //dimension for ender3 buildplate: 22x22
    model = glm::scale(model, glm::vec3(22.0f, 1.0f, 22.0f));
    shader.setMat4("model", model);

    buildPlate.Draw(shader);
}

#endif