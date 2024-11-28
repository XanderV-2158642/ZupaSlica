#ifndef INTERSECTION_H
#define INTERSECTION_H

#include <vector>
#include "../Slicing/TriangleIntersections/CalculateIntersections.hpp"
#include "../Mesh/Mesh.hpp"
#include "../Shader/Shader.hpp"
#include "../Slicing/Slicing.hpp"
#include <clipper2/clipper.h>
#include "../SlicerSettings/SlicerSettings.hpp"


class Intersection
{
private:
    void SetupBuffers();
    void UpdateBuffers(vector<float> &vertices);
    void Draw(Shader &shader, int amountOfLines, float aspectRatio = 1.0f, glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f));
    vector<float> GetVertices(Clipper2Lib::PathsD &paths, float buildplateSize);

    unsigned int VBO, VAO;
    Clipper2Lib::PathsD lines;
    Shader intersectionShader;


    unsigned int shaderProgram;

    float epsilon = 0.05f;
    int plane = 0;

    vector<Slice> sliceMap;

public:
    Intersection();
    void DrawIntersection(float aspectRatio, SlicerSettings settings);
    void SetLines(Clipper2Lib::PathsD &paths) { lines = paths; }
    void PrintLines() { for (int i = 0; i < lines.size(); i++) { printf("Line %d\n", i); } }
    Clipper2Lib::PathsD GetLines() {return lines;}
    void SetHeight(int index);
    float GetHeight() {return plane;}
    int GetMaxHeight() {return sliceMap.size();}
    float GetSlicingPlaneHeight(float layerheight) {return (float) plane * layerheight;}

    void SetSliceMap(vector<Slice> &slices) {sliceMap = slices;}
    vector<Slice> GetSliceMap() {return sliceMap;}

    void Erode(float width);

};

Intersection::Intersection() : intersectionShader("/home/xandervaes/Code/ZupaSlica/src/ShaderFiles/IntersectionShader.vs", "/home/xandervaes/Code/ZupaSlica/src/ShaderFiles/IntersectionShader.fs")
{
    sliceMap = vector<Slice>();
    SetupBuffers();
}

void Intersection::DrawIntersection(float aspectRatio, SlicerSettings settings)
{
    if (sliceMap.size() == 0)
    {
        return;
    }
    vector<float> outerWall = GetVertices(sliceMap[plane].outerWall, settings.GetBuildVolume().x);
    UpdateBuffers(outerWall);
    //Draw(intersectionShader, outerWall.size()/2, aspectRatio, glm::vec3(1.0f, 0.0f, 0.0f));

    for (int i = 0; i < sliceMap[plane].shells.size(); i++)
    {
        vector<float> shell = GetVertices(sliceMap[plane].shells[i], settings.GetBuildVolume().x);
        UpdateBuffers(shell);
        //Draw(intersectionShader, shell.size()/2, aspectRatio, glm::vec3(0.0f, 1.0f, 0.0f));
    }

    vector<float> infill = GetVertices(sliceMap[plane].infill, settings.GetBuildVolume().x);
    UpdateBuffers(infill);
    //Draw(intersectionShader, infill.size()/2, aspectRatio, glm::vec3(1.0f, 1.0f, 0.0f));

    vector<float> surface = GetVertices(sliceMap[plane].surface, settings.GetBuildVolume().x);
    UpdateBuffers(surface);
    Draw(intersectionShader, surface.size()/2, aspectRatio, glm::vec3(0.0f, 0.0f, 1.0f));
}



void Intersection::SetupBuffers(){
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Intersection::UpdateBuffers(vector<float> &vertices){
    
    // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);


    // note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}


void Intersection::Draw(Shader &shader, int amountOfLines, float aspectRatio, glm::vec3 color)
{
    // draw the intersection
    shader.use();
    shader.setFloat("aspectRatio", aspectRatio);
    shader.setVec3("color", color);
    glBindVertexArray(VAO);
    glDrawArrays(GL_LINES, 0, amountOfLines*2);
    glBindVertexArray(0);
}

void Intersection::Erode(float width)
{
    lines = Clipper2Lib::InflatePaths(lines, -width, Clipper2Lib::JoinType::Miter, Clipper2Lib::EndType::Polygon);
}


void Intersection::SetHeight(int index)
{
    plane = index;
}

vector<float> Intersection::GetVertices(Clipper2Lib::PathsD &paths, float buildplateSize)
{
    vector<float> vertices;
    for (int i = 0; i < paths.size(); i++)
    {
        //refactor to PathsD
        for (int j = 0; j < paths[i].size()-1; j++)
        {
            vertices.push_back(paths[i][j].x);
            vertices.push_back(paths[i][j].y);
            vertices.push_back(paths[i][j+1].x);
            vertices.push_back(paths[i][j+1].y);
        }

        vertices.push_back(paths[i][paths[i].size()-1].x);
        vertices.push_back(paths[i][paths[i].size()-1].y);
        vertices.push_back(paths[i][0].x);
        vertices.push_back(paths[i][0].y);
    }

    //scale to buildplate as -1 to 1
    //buildplate is 220x220
    for (int i = 0; i < vertices.size(); i++)
    {
        vertices[i] = vertices[i] / (buildplateSize/2);
    }

    return vertices;
}


#endif