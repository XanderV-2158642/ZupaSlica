#ifndef slicing_H
#define slicing_H
#include <clipper2/clipper.h>
#include <vector>
#include "../Mesh/Mesh.hpp"
#include "TriangleIntersections/CalculateIntersections.hpp"
#include "Infill/CreateInfill.hpp"
#include "../SlicerSettings/SlicerSettings.hpp"
#include "Surface/Surface.hpp"

struct Slice
{
    double height;
    Clipper2Lib::PathsD outerWall;
    Clipper2Lib::PathsD innerWall; //inner wall is a part of shell, but is not considered in the printing process, it is just the last shell, but it is easier to reference like this when clipping the infill
    std::vector<Clipper2Lib::PathsD> shells;
    Clipper2Lib::PathsD infill;
    Clipper2Lib::PathsD surface;
};

class Slicing 
{
private:
public:
    static vector<Slice> SliceModel(vector<Vertex> model, SlicerSettings settings);
};

vector<Slice> Slicing::SliceModel(vector<Vertex> model, SlicerSettings settings) {
    float layerHeight = settings.GetLayerHeight();
    settings.SetSlicingPlaneHeight(layerHeight/2);

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
            //erode outerWall by half the nozzle diameter
            paths = Clipper2Lib::InflatePaths(paths, -settings.GetNozzleDiameter() / 2, Clipper2Lib::JoinType::Miter, Clipper2Lib::EndType::Polygon);
            paths = Clipper2Lib::SimplifyPaths(paths, 0.00125);
            slice.outerWall = paths;
            slice.innerWall = paths;
            

            //add inner shells
            std::vector<Clipper2Lib::PathsD> shells;
            Clipper2Lib::PathsD lastPaths = paths;
            for (int i = 0; i < settings.GetShells()-1; i++)
            {
                Clipper2Lib::PathsD shellPaths = Clipper2Lib::InflatePaths(lastPaths, -settings.GetNozzleDiameter(), Clipper2Lib::JoinType::Miter, Clipper2Lib::EndType::Polygon);
                shellPaths = Clipper2Lib::SimplifyPaths(shellPaths, 0.00125); 
                lastPaths = shellPaths;
                shells.push_back(shellPaths);
            }
            slice.shells = shells;

            //set inner wall
            slice.innerWall = lastPaths;

            //add infill
            Clipper2Lib::PathsD infill = CreateInfill::CreateDiagonalInfill(settings.GetInfill(), settings);
            infill = CreateInfill::ClipInfill(infill, slice.innerWall);
            slice.infill = infill;

            slices.push_back(slice);
            //move the slicing plane up
            settings.SetSlicingPlaneHeight(settings.GetSlicingPlaneHeight() + layerHeight);
        }
    }

    // calculate surfaces
    for (int i = 0; i < slices.size(); i++)
    {
        Slice curSlice = slices[i];
        vector<Clipper2Lib::PathsD> floorAdjacences;
        vector<Clipper2Lib::PathsD> roofAdjacences;
        if (i < slices.size() - 1)
        {
            roofAdjacences.push_back(slices[i + 1].innerWall);
        }
        
        curSlice.surface = Surface::CalculateSurface(curSlice.innerWall, floorAdjacences, roofAdjacences );
        slices[i] = curSlice;
    } 

    return slices;
}

#endif