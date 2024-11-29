#ifndef CalculateIntersections_H
#define CalculateIntersections_H

#include <vector>
#include "../../Mesh/Mesh.hpp"
#include <algorithm>
#include <clipper2/clipper.h>

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
    static bool IsIntersecting(vector<Vertex> &triangleVertices, float intersectionHeight);
    static void SortByHeight(vector<Vertex> &triangleVertices);

    static vector<VertexPair> CalculatePairs(vector<Vertex> &vertices, double intersectionHeight);
    static vector<VertexLine> CalculateLines(vector<VertexPair> &vertexPairs);
    static void GroupLine(vector<VertexPair> &vertexPairs, VertexLine &line);


    

public:
    static vector<VertexLine> CalculateLines(vector<Vertex> &vertices, float intersectionHeight);

    static Clipper2Lib::PathsD CalculateClipperPaths(vector<Vertex> &lines, SlicerSettings settings);
};

vector<VertexLine> CalculateIntersections::CalculateLines(vector<Vertex> &vertices, float intersectionHeight)
{
    if (vertices.size() % 3 != 0)
    {
        printf("Amount of vertices: %d\n", vertices.size());
        throw "Amount of vertices is not a multiple of 3";
    }

    vector<VertexPair> vertexPairs = CalculatePairs(vertices, intersectionHeight);

    vector<VertexLine> lines = CalculateLines(vertexPairs);   

    return lines;
}

bool CalculateIntersections::IsIntersecting(vector<Vertex> &triangleVertices, float intersectionHeight)
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

vector<VertexPair> CalculateIntersections::CalculatePairs(vector<Vertex> &vertices, double intersectionHeight){
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
    return vertexPairs;
}

vector<VertexLine> CalculateIntersections::CalculateLines(vector<VertexPair> &vertexPairs)
{
    // create a line for all connecting pairs
    vector<VertexLine> lines;

    double epsilon = 0.0000001;
    
    while (vertexPairs.size() > 0)
    {
        // take the first element out of the vector, remove it from the pairs so it can not pick itself and start a line with it
        VertexPair curPair = vertexPairs[0];
        vertexPairs.erase(vertexPairs.begin());

        VertexLine line;
        line.lineSegments.push_back(curPair);

        glm::vec3 linepos = curPair.v2.Position;

        // find the next pair that connects to the last pair
        while (true)
        {
            bool found = false;
            for (int i = 0; i < vertexPairs.size(); i++)
            {
                if (glm::distance(linepos, vertexPairs[i].v1.Position) < epsilon)
                {
                    curPair = vertexPairs[i];
                    vertexPairs.erase(vertexPairs.begin() + i);
                    line.lineSegments.push_back(curPair);
                    linepos = curPair.v2.Position;
                    found = true;
                    break;
                }
                else if (glm::distance(linepos, vertexPairs[i].v2.Position) < epsilon)
                {
                    curPair = vertexPairs[i];
                    vertexPairs.erase(vertexPairs.begin() + i);
                    line.lineSegments.push_back(curPair);
                    linepos = curPair.v1.Position;
                    found = true;
                    break;
                }
            }
            if (!found)
            {
                break;
            }
        }

        //check if the line is closed
        if (glm::distance(line.lineSegments[0].v1.Position, line.lineSegments[line.lineSegments.size() - 1].v2.Position) < epsilon)
        {
            lines.push_back(line);
        } else if (glm::distance(line.lineSegments[0].v1.Position, line.lineSegments[line.lineSegments.size() - 1].v1.Position) < epsilon)
        {
            lines.push_back(line);
        }
        else {
            // throw error line is not closed
            printf("Line is not closed\n");
            //throw "Line is not closed";
        }
    }

    return lines;
}

Clipper2Lib::PathsD CalculateIntersections::CalculateClipperPaths(vector<Vertex> &lines, SlicerSettings settings)
{
    double intersectionHeight = settings.GetSlicingPlaneHeight();
    // first find all triangle intersecting lines using the calculatePairs function
    vector<VertexPair> vertexPairs = CalculatePairs(lines, intersectionHeight);

    // then group the pairs into lines
    vector<VertexLine> vertexLines = CalculateLines(vertexPairs);


    //then convert each of the lines into a PathD for the clipper library
    Clipper2Lib::PathsD clipperPaths;
    for (int i = 0; i < vertexLines.size(); i++)
    {
        Clipper2Lib::PathD clipperPath;
        clipperPath.push_back(Clipper2Lib::PointD(vertexLines[i].lineSegments[0].v1.Position.x, vertexLines[i].lineSegments[0].v1.Position.y));
        clipperPath.push_back(Clipper2Lib::PointD(vertexLines[i].lineSegments[0].v2.Position.x, vertexLines[i].lineSegments[0].v2.Position.y));
        glm::vec3 lastPos = vertexLines[i].lineSegments[0].v2.Position;
        for (int j = 1; j < vertexLines[i].lineSegments.size(); j++)
        {
            if (glm::distance(lastPos, vertexLines[i].lineSegments[j].v1.Position) < 0.05)
            {
                clipperPath.push_back(Clipper2Lib::PointD(vertexLines[i].lineSegments[j].v2.Position.x, vertexLines[i].lineSegments[j].v2.Position.y));
                lastPos = vertexLines[i].lineSegments[j].v2.Position;
            } else {
                clipperPath.push_back(Clipper2Lib::PointD(vertexLines[i].lineSegments[j].v1.Position.x, vertexLines[i].lineSegments[j].v1.Position.y));
                lastPos = vertexLines[i].lineSegments[j].v1.Position;
            }
        }
        clipperPaths.push_back(clipperPath);
    }

    Clipper2Lib::PathsD returnPaths;
    returnPaths = Clipper2Lib::Union(clipperPaths, Clipper2Lib::FillRule::EvenOdd);

    return returnPaths;
}


void CalculateIntersections::GroupLine(vector<VertexPair> &vertexPairs, VertexLine &line)
{
    
}


#endif