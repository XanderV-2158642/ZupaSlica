#ifndef CalculateIntersections_H
#define CalculateIntersections_H

#include <vector>
#include "../../Mesh/Mesh.hpp"
#include <algorithm>

struct VertexPair
{
    Vertex v1;
    Vertex v2;
};

struct VertexLine
{
    vector<VertexPair> lineSegments;
};

class CalculateIntersections
{

private:
    static bool IsIntersecting(vector<Vertex> triangleVertices, float intersectionHeight);
    static void SortByHeight(vector<Vertex> &triangleVertices);
public:
    static vector<VertexLine> CalculateLines(vector<Vertex> vertices, float intersectionHeight);
};

vector<VertexLine> CalculateIntersections::CalculateLines(vector<Vertex> vertices, float intersectionHeight)
{
    if (vertices.size() % 3 != 0)
    {
        throw "Amount of vertices is not a multiple of 3";
    }

    vector<VertexPair> vertexPairs;
    // vertices are grouped by 3 creating a triangle
    for (int i = 0; i < vertices.size(); i += 3)
    {
        // get the 3 vertices of the triangle and put in a vector
        vector<Vertex> triangleVertices = {vertices[i], vertices[i + 1], vertices[i + 2]};
        SortByHeight(triangleVertices);
        if (triangleVertices[0].Position.z < intersectionHeight && triangleVertices[2].Position.z > intersectionHeight)
        {
            // 2 points below or above
            if (triangleVertices[1].Position.z < intersectionHeight)
            {
                //0-2 
                float x1 = triangleVertices[0].Position.x + (intersectionHeight - triangleVertices[0].Position.z) * (triangleVertices[2].Position.x - triangleVertices[0].Position.x) / (triangleVertices[2].Position.z - triangleVertices[0].Position.z);
                float y1 = triangleVertices[0].Position.y + (intersectionHeight - triangleVertices[0].Position.z) * (triangleVertices[2].Position.y - triangleVertices[0].Position.y) / (triangleVertices[2].Position.z - triangleVertices[0].Position.z);

                //1-2
                float x2 = triangleVertices[1].Position.x + (intersectionHeight - triangleVertices[1].Position.z) * (triangleVertices[2].Position.x - triangleVertices[1].Position.x) / (triangleVertices[2].Position.z - triangleVertices[1].Position.z);
                float y2 = triangleVertices[1].Position.y + (intersectionHeight - triangleVertices[1].Position.z) * (triangleVertices[2].Position.y - triangleVertices[1].Position.y) / (triangleVertices[2].Position.z - triangleVertices[1].Position.z);

                VertexPair pair;
                pair.v1.Position = glm::vec3(x1, y1, intersectionHeight);
                pair.v2.Position = glm::vec3(x2, y2, intersectionHeight);
                vertexPairs.push_back(pair);

            }
            else
            {
                // 0-1 and 0-2
                float x1 = triangleVertices[0].Position.x + (intersectionHeight - triangleVertices[0].Position.z) * (triangleVertices[1].Position.x - triangleVertices[0].Position.x) / (triangleVertices[1].Position.z - triangleVertices[0].Position.z);
                float y1 = triangleVertices[0].Position.y + (intersectionHeight - triangleVertices[0].Position.z) * (triangleVertices[1].Position.y - triangleVertices[0].Position.y) / (triangleVertices[1].Position.z - triangleVertices[0].Position.z);

                float x2 = triangleVertices[0].Position.x + (intersectionHeight - triangleVertices[0].Position.z) * (triangleVertices[2].Position.x - triangleVertices[0].Position.x) / (triangleVertices[2].Position.z - triangleVertices[0].Position.z);
                float y2 = triangleVertices[0].Position.y + (intersectionHeight - triangleVertices[0].Position.z) * (triangleVertices[2].Position.y - triangleVertices[0].Position.y) / (triangleVertices[2].Position.z - triangleVertices[0].Position.z);

                VertexPair pair;
                pair.v1.Position = glm::vec3(x1, y1, intersectionHeight);
                pair.v2.Position = glm::vec3(x2, y2, intersectionHeight);
                vertexPairs.push_back(pair);
            }
        }

    }
    VertexLine line;
    line.lineSegments = vertexPairs;

    return {line};
}

bool CalculateIntersections::IsIntersecting(vector<Vertex> triangleVertices, float intersectionHeight)
{
    // check if the intersectionHeight is between the y values of the triangle
    float minY = min(triangleVertices[0].Position.y, min(triangleVertices[1].Position.y, triangleVertices[2].Position.y));
    float maxY = max(triangleVertices[0].Position.y, max(triangleVertices[1].Position.y, triangleVertices[2].Position.y));

    return intersectionHeight >= minY && intersectionHeight <= maxY;
}

void CalculateIntersections::SortByHeight(vector<Vertex> &triangleVertices)
{
    // sort the vertices by height
    sort(triangleVertices.begin(), triangleVertices.end(), [](Vertex v1, Vertex v2) {
        return v1.Position.z < v2.Position.z;
    });
}

#endif