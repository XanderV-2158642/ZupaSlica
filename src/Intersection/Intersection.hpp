#ifndef INTERSECTION_H
#define INTERSECTION_H

#include <vector>
#include "../Slicing/TriangleIntersections/CalculateIntersections.hpp"
#include "../Mesh/Mesh.hpp"
#include "../Shader/Shader.hpp"

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
    vector<VertexLine> lines;
    Shader intersectionShader;


    unsigned int shaderProgram;

public:
    Intersection();
    void DrawIntersection();
    void SetLines(vector<VertexLine> &lines) { this->lines = lines; }
    void PrintLines() { for (int i = 0; i < lines.size(); i++) { printf("Line %d\n", i); } }

};

Intersection::Intersection() : intersectionShader("/home/xandervaes/Code/ZupaSlica/src/ShaderFiles/IntersectionShader.vs", "/home/xandervaes/Code/ZupaSlica/src/ShaderFiles/IntersectionShader.fs")
{
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
        for (int j = 0; j < lines[i].lineSegments.size(); j++)
        {
            if (lines[i].lineSegments[j].v1.Position.x < xmin)
            {
                xmin = lines[i].lineSegments[j].v1.Position.x;
            }
            if (lines[i].lineSegments[j].v1.Position.x > xmax)
            {
                xmax = lines[i].lineSegments[j].v1.Position.x;
            }
            if (lines[i].lineSegments[j].v1.Position.y < ymin)
            {
                ymin = lines[i].lineSegments[j].v1.Position.y;
            }
            if (lines[i].lineSegments[j].v1.Position.y > ymax)
            {
                ymax = lines[i].lineSegments[j].v1.Position.y;
            }

            
            vertices.push_back(lines[i].lineSegments[j].v1.Position.x);
            vertices.push_back(lines[i].lineSegments[j].v1.Position.y);
            vertices.push_back(lines[i].lineSegments[j].v2.Position.x);
            vertices.push_back(lines[i].lineSegments[j].v2.Position.y);
        }
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