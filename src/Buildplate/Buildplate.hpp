#ifndef BUILDPLATE_H
#define BUILDPLATE_H

#include <glm/glm.hpp>
#include <glad/glad.h>
#include <vector>
#include "../Mesh/Mesh.hpp"

class Buildplate {
public:
    static std::vector<Vertex> GetVertices();
    static std::vector<unsigned int> GetIndices();
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

#endif