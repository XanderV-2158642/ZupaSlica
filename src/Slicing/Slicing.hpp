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
    Clipper2Lib::PathsD surfaceWall;
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

            
            slice.infill = Clipper2Lib::PathsD();

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
        if (i < slices.size() - settings.GetRoofs())
        {
            for (int j = i + 1; j <= i + settings.GetRoofs(); j++)
            {
                roofAdjacences.push_back(slices[j].innerWall);
            }
        } else {
            roofAdjacences.push_back(Clipper2Lib::PathsD());
        }

        if (i >= settings.GetFloors())
        {
            for (int j = i - settings.GetFloors(); j < i; j++)
            {
                floorAdjacences.push_back(slices[j].innerWall);
            }
        } else {
            floorAdjacences.push_back(Clipper2Lib::PathsD());
        }


        Clipper2Lib::PathsD offsettedInnerWall = Clipper2Lib::InflatePaths(curSlice.innerWall, -settings.GetNozzleDiameter(), Clipper2Lib::JoinType::Miter, Clipper2Lib::EndType::Polygon);
        
        curSlice.surfaceWall = Surface::CalculateSurface(offsettedInnerWall, floorAdjacences, roofAdjacences);
        Clipper2Lib::PathsD sparseInfillClipArea = Surface::CalculateSurface(curSlice.innerWall, floorAdjacences, roofAdjacences);


        //calculate surfaceInfill
        Clipper2Lib::PathsD surfaceInfill = CreateInfill::CreateSurfaceInfill(i, settings); 
        Clipper2Lib::PathsD inflatedWall = Clipper2Lib::InflatePaths(curSlice.surfaceWall, -settings.GetNozzleDiameter() / 2, Clipper2Lib::JoinType::Miter, Clipper2Lib::EndType::Polygon);
        curSlice.surface = CreateInfill::ClipInfill(surfaceInfill, inflatedWall);

        //generate infill
        //Take difference of innerwall sparseInfillClipArea
        curSlice.infill = CreateInfill::CreateDiagonalInfill(settings.GetInfill(), settings);

        //calculate clipping area
        Clipper2Lib::PathsD sparseInfillClip = Clipper2Lib::Difference(curSlice.innerWall, sparseInfillClipArea, Clipper2Lib::FillRule::EvenOdd);
        curSlice.infill = CreateInfill::ClipInfill(curSlice.infill, sparseInfillClip);

        slices[i] = curSlice;
    }

    return slices;
}

#endif