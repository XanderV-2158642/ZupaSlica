#ifndef slicing_H
#define slicing_H
#include <clipper2/clipper.h>
#include <vector>
#include "../Mesh/Mesh.hpp"
#include "TriangleIntersections/CalculateIntersections.hpp"

struct Slice
{
    double height;
    Clipper2Lib::PathsD paths;
};

class Slicing 
{
private:
public:
    static vector<Slice> SliceModel(vector<Vertex> model, SlicerSettings settings);
};

vector<Slice> Slicing::SliceModel(vector<Vertex> model, SlicerSettings settings) {
    float layerHeight = settings.GetLayerHeight();
    settings.SetSlicingPlaneHeight(layerHeight/2 + 0.000000001);

    vector<Slice> slices;
    bool nonEmpty = true;
    while (nonEmpty)
    {
        Clipper2Lib::PathsD paths = CalculateIntersections::CalculateClipperPaths(model, settings);
        //printf("Amount of paths: %d\n", paths.size());
        if (paths.size() == 0)
        {
            nonEmpty = false;
        }
        else
        {
            Slice slice;
            slice.height = settings.GetSlicingPlaneHeight();
            //erode paths by half the nozzle diameter
            paths = Clipper2Lib::InflatePaths(paths, -settings.GetNozzleDiameter() / 2, Clipper2Lib::JoinType::Miter, Clipper2Lib::EndType::Polygon);
            //add inner shells
            Clipper2Lib::PathsD lastPaths = paths;
            for (int i = 0; i < settings.GetShells()-1; i++)
            {
                Clipper2Lib::PathsD shellPaths = Clipper2Lib::InflatePaths(lastPaths, -settings.GetNozzleDiameter(), Clipper2Lib::JoinType::Miter, Clipper2Lib::EndType::Polygon);
                paths.insert(paths.end(), shellPaths.begin(), shellPaths.end());
                lastPaths = shellPaths;
            }
            slice.paths = paths;
            slices.push_back(slice);
            settings.SetSlicingPlaneHeight(settings.GetSlicingPlaneHeight() + layerHeight);
        }
    }

    return slices;
}

#endif