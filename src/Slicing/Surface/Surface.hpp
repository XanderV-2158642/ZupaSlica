#ifndef SURFACE_H
#define SURFACE_H

#include <vector>
#include "../Slicing.hpp"
#include "../../SlicerSettings/SlicerSettings.hpp"
#include <clipper2/clipper.h> 

class Surface
{
private:
    static Clipper2Lib::PathsD CalculateSliceSurface(Clipper2Lib::PathsD &curSlice, vector<Clipper2Lib::PathsD> adjacentSlices);
    static Clipper2Lib::PathsD CalculateFloors(Clipper2Lib::PathsD &curSlice, vector<Clipper2Lib::PathsD> adjacentSlices);

    static void FilterArtifacts(Clipper2Lib::PathsD &paths, double epsilon);
    static Clipper2Lib::PathsD IntersectAdjacentSlices(vector<Clipper2Lib::PathsD> adjacentSlices);

    static void printPaths(Clipper2Lib::PathsD paths)
    {
        for (int i = 0; i < paths.size(); i++)
        {
            for (int j = 0; j < paths[i].size(); j++)
            {
                printf("X: %f, Y: %f\n", paths[i][j].x, paths[i][j].y);
            }
        }
    };

public:
    static Clipper2Lib::PathsD CalculateSurface(Clipper2Lib::PathsD curSlice, vector<Clipper2Lib::PathsD> floorAdjacences, vector<Clipper2Lib::PathsD> roofAdjacences);

};

Clipper2Lib::PathsD Surface::CalculateSurface(Clipper2Lib::PathsD curSlice, vector<Clipper2Lib::PathsD> floorAdjacences, vector<Clipper2Lib::PathsD> roofAdjacences)
{
    //Clipper2Lib::PathsD result;
    Clipper2Lib::PathsD surface;
    //Calculate floors
    Clipper2Lib::PathsD floors = CalculateSliceSurface(curSlice, floorAdjacences);
    //Calculate roofs
    Clipper2Lib::PathsD roofs = CalculateSliceSurface(curSlice, roofAdjacences);
    //Union result with surface
    surface = Clipper2Lib::Union(floors, roofs, Clipper2Lib::FillRule::EvenOdd);

    return surface;
};

Clipper2Lib::PathsD Surface::CalculateSliceSurface(Clipper2Lib::PathsD &curSlice, vector<Clipper2Lib::PathsD> adjacentSlices)
{
    //curslice is innerwall of the layer offsetted by nozzle diameter -> always prints extra inner wall on surfaces
    //adjacent is inner wall of the next layers

    //compare innerwalls of current slice with outerwalls of adjacent slices
    Clipper2Lib::PathsD surface;
    
    if (adjacentSlices.size() == 0)
    {
        return surface;
    }

    // intersect adjacent slices
    Clipper2Lib::PathsD adjacentIntersected = IntersectAdjacentSlices(adjacentSlices);
    Clipper2Lib::PathsD result =  Clipper2Lib::Difference(curSlice, adjacentIntersected, Clipper2Lib::FillRule::EvenOdd);

    FilterArtifacts(result, 0.8);
    //Union result with surface
    surface = Clipper2Lib::Union(surface, result, Clipper2Lib::FillRule::EvenOdd);
    
    return result;
};

Clipper2Lib::PathsD Surface::CalculateFloors(Clipper2Lib::PathsD &curSlice, vector<Clipper2Lib::PathsD> adjacentSlices)
{
    Clipper2Lib::PathsD surface;

    if (adjacentSlices.size() == 0)
    {
        return surface;
    }

    surface = Clipper2Lib::Difference(curSlice, adjacentSlices[0], Clipper2Lib::FillRule::EvenOdd);

    FilterArtifacts(surface, 0.8);
    surface = Clipper2Lib::Union(surface, curSlice, Clipper2Lib::FillRule::EvenOdd);

    return surface;
};

Clipper2Lib::PathsD Surface::IntersectAdjacentSlices(vector<Clipper2Lib::PathsD> adjacentSlices)
{
    Clipper2Lib::PathsD result = adjacentSlices[0];
    for (int i = 1; i < adjacentSlices.size(); i++)
    {
        result = Clipper2Lib::Intersect(result, adjacentSlices[i], Clipper2Lib::FillRule::EvenOdd);
    }

    return result;
};

void Surface::FilterArtifacts(Clipper2Lib::PathsD &paths, double epsilon)
{
    for (int i = 0; i < paths.size(); i++)
    {
        double area = Clipper2Lib::Area(paths[i]);
        if (area > 0 && area < epsilon)
        {
            paths.erase(paths.begin() + i);
            i--;

        } else if (area < 0 && area > -epsilon)
        {
            paths.erase(paths.begin() + i);
            i--;
        }
    }
};  


#endif 