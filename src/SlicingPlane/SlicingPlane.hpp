#ifndef SlICINGPLANE_H
#define SLICINGPLANE_H

#include <glm/glm.hpp>
#include <glad/glad.h>
#include <vector>
#include "../Mesh/Mesh.hpp"
#include "../Camera/Camera.hpp"
#include "../Shader/Shader.hpp"


class SlicingPlane {
private:
    /* data */
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    glm::vec3 position;
    glm::vec3 color;
    glm::vec3 scale;

    Shader slicingShader;
    Mesh slicingPlane;

    std::vector<Vertex> GetVertices();
    std::vector<unsigned int> GetIndices();

public:
    SlicingPlane(Shader shader);

    void SetPosition(glm::vec3 pos) { position = pos; }
    void Draw(glm::mat4 view, glm::mat4 projection, Camera camera);
};


SlicingPlane::SlicingPlane(Shader shader) : slicingShader(shader), 
                                            slicingPlane(Mesh(GetVertices(), 
                                            GetIndices(), std::vector<Texture>())), 
                                            vertices(GetVertices()), indices(GetIndices()) 
    {
        color = glm::vec3(0.5f, 0.7f, 0.5f);
        scale = glm::vec3(22.0f, 1.0f, 22.0f);                                    
    }


std::vector<Vertex> SlicingPlane::GetVertices() {
    Vertex vertices[] = {
        Vertex{glm::vec3(-0.5f, 0.0f,  0.5f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.0f, 0.0f)},
        Vertex{glm::vec3(-0.5f, 0.0f, -0.5f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.0f, 1.0f)},
        Vertex{glm::vec3(0.5f, 0.0f, -0.5f), glm::vec3(0.0f, 1.0f, 0.0f),  glm::vec2(1.0f, 1.0f)},
        Vertex{glm::vec3(0.5f, 0.0f,  0.5f), glm::vec3(0.0f, 1.0f, 0.0f),  glm::vec2(1.0f, 0.0f)}
    };

    std::vector<Vertex> verts(vertices, vertices + sizeof(vertices) / sizeof(Vertex));
    return verts;
}


std::vector<unsigned int> SlicingPlane::GetIndices() {
    vector<unsigned int> planeIndices = {
        0, 1, 2,
        0, 2, 3
    };

    return planeIndices;
}

void SlicingPlane::Draw(glm::mat4 view, glm::mat4 projection, Camera camera) {
    slicingShader.use();
    //light properties
    slicingShader.setVec3("objectColor", color);
    slicingShader.setVec3("lightColor", glm::vec3(1.0f));
    slicingShader.setVec3("lightPos", camera.Position);
    slicingShader.setVec3("viewPos", camera.Position);

    slicingShader.setMat4("view", view);
    slicingShader.setMat4("projection", projection);

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);
    //scale to buildplate dimensions
    //dimension for ender3 buildplate: 22x22
    model = glm::scale(model, glm::vec3(scale));
    slicingShader.setMat4("model", model);

    slicingPlane.Draw(slicingShader);
}

#endif