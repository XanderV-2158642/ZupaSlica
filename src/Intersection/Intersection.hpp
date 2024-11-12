#ifndef INTERSECTION_H
#define INTERSECTION_H

#include <vector>
#include "../Slicing/TriangleIntersections/CalculateIntersections.hpp"
#include "../Mesh/Mesh.hpp"
#include "../Shader/Shader.hpp"
#include "../Slicing/Slicing.hpp"
#include <clipper2/clipper.h>

// settings
const unsigned int INTERSECTION_WIDTH = 1280;
const unsigned int INTERSECTION_HEIGHT = 720;



class Intersection
{
private:
    void SetupBuffers();
    void UpdateBuffers(vector<float> &vertices);
    void Draw(Shader &shader, int amountOfLines);

    unsigned int VBO, VAO;
    Clipper2Lib::PathsD lines;
    Shader intersectionShader;


    unsigned int shaderProgram;

    float epsilon = 0.05f;
    int plane = 0;

    vector<Slice> sliceMap;

public:
    Intersection();
    void DrawIntersection();
    void SetLines(Clipper2Lib::PathsD &paths) { lines = paths; }
    void PrintLines() { for (int i = 0; i < lines.size(); i++) { printf("Line %d\n", i); } }
    Clipper2Lib::PathsD GetLines() {return lines;}
    void SetHeight(int index) {plane = index; if (index < sliceMap.size()) SetLines(sliceMap[index].paths);}
    float GetHeight() {return plane;}
    int GetMaxHeight() {return sliceMap.size();}
    float GetSlicingPlaneHeight(float layerheight) {return (float) plane * layerheight;}

    void SetSliceMap(vector<Slice> &slices) {sliceMap = slices;}

};

Intersection::Intersection() : intersectionShader("/home/xandervaes/Code/ZupaSlica/src/ShaderFiles/IntersectionShader.vs", "/home/xandervaes/Code/ZupaSlica/src/ShaderFiles/IntersectionShader.fs")
{
    sliceMap = vector<Slice>();
    SetupBuffers();
}

void Intersection::DrawIntersection()
{
    //draw lines
    float xmin = 1000000;
    float xmax = -1000000;
    float ymin = 1000000;
    float ymax = -1000000;

    vector<float> vertices;
    for (int i = 0; i < lines.size(); i++)
    {
        //refactor to PathsD
        for (int j = 0; j < lines[i].size()-1; j++)
        {
            if (lines[i][j].x < xmin)
            {
                xmin = lines[i][j].x;
            }
            if (lines[i][j].x > xmax)
            {
                xmax = lines[i][j].x;
            }
            if (lines[i][j].y < ymin)
            {
                ymin = lines[i][j].y;
            }
            if (lines[i][j].y > ymax)
            {
                ymax = lines[i][j].y;
            }

            vertices.push_back(lines[i][j].x);
            vertices.push_back(lines[i][j].y);
            vertices.push_back(lines[i][j+1].x);
            vertices.push_back(lines[i][j+1].y);
        }

        // close the line and check if the last point is min or max
        if (lines[i][lines[i].size()-1].x < xmin)
        {
            xmin = lines[i][lines[i].size()-1].x;
        }
        if (lines[i][lines[i].size()-1].x > xmax)
        {
            xmax = lines[i][lines[i].size()-1].x;
        }
        if (lines[i][lines[i].size()-1].y < ymin)
        {
            ymin = lines[i][lines[i].size()-1].y;
        }
        if (lines[i][lines[i].size()-1].y > ymax)
        {
            ymax = lines[i][lines[i].size()-1].y;
        }

        vertices.push_back(lines[i][lines[i].size()-1].x);
        vertices.push_back(lines[i][lines[i].size()-1].y);
        vertices.push_back(lines[i][0].x);
        vertices.push_back(lines[i][0].y);
    }

    //scale down to fit -0.8 to 0.8 and center
    float xdiff = xmax - xmin;
    float ydiff = ymax - ymin;

    float highestDiff = xdiff > ydiff ? xdiff : ydiff;

    for (int i = 0; i < vertices.size(); i++)
    {
        if (i % 2 == 0)
        {
            vertices[i] = (vertices[i] - xmin) / highestDiff * 1.6 - 0.8;
        }
        else
        {
            vertices[i] = (vertices[i] - ymin) / highestDiff * 1.6 - 0.8;
        }
    }

    UpdateBuffers(vertices);
    Draw(intersectionShader, vertices.size()/2);
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


void Intersection::Draw(Shader &shader, int amountOfLines)
{
    // draw the intersection
    shader.use();
    glBindVertexArray(VAO);
    glDrawArrays(GL_LINES, 0, amountOfLines*2);
    glBindVertexArray(0);
}

#endif