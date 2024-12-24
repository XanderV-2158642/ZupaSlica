#pragma once
#include <vector>
#include "../Slicing/Slicing.hpp"
#include "omp.h"

class PathOptimization
{
private:
    vector<Slice> slices;

    void OptimizeInfill();
    void OptimizeSurface();

    Clipper2Lib::PathsD SortPaths(Clipper2Lib::PathsD paths);

public:
    PathOptimization(vector<Slice> slices)
    {
        this->slices = slices;
    }
    
    ~PathOptimization()
    {
    }

    void OptimizePaths();

    vector<Slice> GetSlices()
    {
        return slices;
    }
};

void PathOptimization::OptimizePaths() {
    OptimizeInfill();
    OptimizeSurface();
}

void PathOptimization::OptimizeInfill() {
    #pragma omp parallel for
    for (int i = 0; i < slices.size(); i++)
    {
        Slice slice = slices[i];
        Clipper2Lib::PathsD optimizedInfill = SortPaths(slice.infill);
        slice.infill = optimizedInfill;
        slices[i] = slice;
    }
}

void PathOptimization::OptimizeSurface() {
    #pragma omp parallel for
    for (int i = 0; i < slices.size(); i++)
    {
        Slice slice = slices[i];
        Clipper2Lib::PathsD optimizedSurface = SortPaths(slice.surface);
        slice.surface = optimizedSurface;
        slices[i] = slice;
    }
}

Clipper2Lib::PathsD PathOptimization::SortPaths(Clipper2Lib::PathsD paths) {
    if (paths.size() <= 1 ){
        return paths;
    }

    Clipper2Lib::PathsD sortedPaths;
    sortedPaths.push_back(paths[0]); // start with the first path
    paths.erase(paths.begin());

    while (paths.size() > 0){
        // path is a vector of 2 points, start and end.
        Clipper2Lib::PointD EndPoint = sortedPaths[sortedPaths.size()-1][1]; // endpoint is the end of the last path
        double closestDistance = sqrt(pow(EndPoint.x - paths[0][0].x, 2) + pow(EndPoint.y - paths[0][0].y, 2));
        int closestIndex = 0;
        bool reverse = false;
        for (int i = 0; i < paths.size(); i++){
            Clipper2Lib::PointD nextStart = paths[i][0];
            double distance = sqrt(pow(EndPoint.x - nextStart.x, 2) + pow(EndPoint.y - nextStart.y, 2));
            if (distance < closestDistance){
                closestDistance = distance;
                closestIndex = i;
                reverse = false;
            }

            Clipper2Lib::PointD nextEnd = paths[i][paths[i].size()-1];
            distance = sqrt(pow(EndPoint.x - nextEnd.x, 2) + pow(EndPoint.y - nextEnd.y, 2));
            if (distance < closestDistance){
                closestDistance = distance;
                closestIndex = i;
                reverse = true;
            }
        }

        if (reverse){
            Clipper2Lib::PathD reversedPath;
            for (int i = paths[closestIndex].size()-1; i >= 0; i--){
                reversedPath.push_back(paths[closestIndex][i]);
            }
            paths.erase(paths.begin() + closestIndex);
            sortedPaths.push_back(reversedPath);
        } else {
            sortedPaths.push_back(paths[closestIndex]);
            paths.erase(paths.begin() + closestIndex);
        }
    }

    return sortedPaths;
}