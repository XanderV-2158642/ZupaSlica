#ifndef SURFACE_H
#define SURFACE_H

#include <vector>
#include "../Slicing.hpp"
#include "../../SlicerSettings/SlicerSettings.hpp"
#include <clipper2/clipper.h> 

class Surface
{
private:
    static Clipper2Lib::PathsD CalculateRoofs(Clipper2Lib::PathsD &curSlice, vector<Clipper2Lib::PathsD> adjacentSlices);
    static Clipper2Lib::PathsD CalculateFloors();

    static void FilterArtifacts(Clipper2Lib::PathsD &paths, double epsilon);

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
    return CalculateRoofs(curSlice, roofAdjacences);
};

Clipper2Lib::PathsD Surface::CalculateRoofs(Clipper2Lib::PathsD &curSlice, vector<Clipper2Lib::PathsD> adjacentSlices)
{
    //compare outerwalls of current slice with outerwalls of adjacent slices
    Clipper2Lib::PathsD surface;
    
    if (adjacentSlices.size() == 0)
    {
        return surface;
    }
    Clipper2Lib::PathsD result;
    result = Clipper2Lib::Difference(curSlice, adjacentSlices[0], Clipper2Lib::FillRule::EvenOdd);
    FilterArtifacts(result, 0.25);
    //Union result with surface
    surface = Clipper2Lib::Union(surface, result, Clipper2Lib::FillRule::EvenOdd);
    
    return surface;
};

Clipper2Lib::PathsD Surface::CalculateFloors()
{
    Clipper2Lib::PathsD result;
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