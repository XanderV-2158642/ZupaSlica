#ifndef DrawSTL_H
#define DrawSTL_H

#include <glm/glm.hpp>
#include <glad/glad.h>
#include "../Model/Model.hpp"
#include "../Camera/Camera.hpp"

class DrawSTL
{
private:

public:
    static float GetLowestPoint(Model &model){
        float lowest = 0.0f;
        for (int i = 0; i < model.meshes[0].vertices.size(); i+=3){
            if (model.meshes[0].vertices[i+1].Position.z < lowest){
                lowest = model.meshes[0].vertices[i+1].Position.z;
            }
        }
        return lowest;
    }

    static glm::vec3 GetXYCenterPoint(Model &model){
        glm::vec3 center = glm::vec3(0.0f, 0.0f, 0.0f);
        for (int i = 0; i < model.meshes[0].vertices.size(); i+=3){
            center.x += model.meshes[0].vertices[i+1].Position.x;
            center.y += model.meshes[0].vertices[i+1].Position.y;
        }
        center.x /= model.meshes[0].vertices.size()/3;
        center.y /= model.meshes[0].vertices.size()/3;
        return center;
    }

    static void Draw(Shader objectShader, Model ourModel,  glm::mat4 view, glm::mat4 projection, Camera camera, glm::vec3 lowest);
};

void DrawSTL::Draw(Shader objectShader, Model ourModel,  glm::mat4 view, glm::mat4 projection, Camera camera, glm::vec3 lowest){
    objectShader.use();

    // light properties
    objectShader.setVec3("viewPos", camera.Position);
    objectShader.setVec3("lightPos", camera.Position);
    objectShader.setVec3("lightColor", 1.0f, 1.0f, 1.0f);
    objectShader.setVec3("objectColor", 0.3f, 0.7f, 0.7f);

    // view/projection transformations
    objectShader.setMat4("projection", projection);
    objectShader.setMat4("view", view);

    // render the loaded model
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f)); // translate it down so it's at the center of the scene
    model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::translate(model, lowest*-0.1f); // translate it down so it's at the center of the scene
    model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));	// it's a bit too big for our scene, so scale it down
    objectShader.setMat4("model", model);

    ourModel.Draw(objectShader);
}

#endif