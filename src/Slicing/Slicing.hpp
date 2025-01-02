#ifndef slicing_H
#define slicing_H
#include <clipper2/clipper.h>
#include <vector>
#include "../Mesh/Mesh.hpp"
#include "TriangleIntersections/CalculateIntersections.hpp"
#include "Infill/CreateInfill.hpp"
#include "../SlicerSettings/SlicerSettings.hpp"
#include "Surface/Surface.hpp"
#include "omp.h"

struct Slice
{
    double height;
    Clipper2Lib::PathsD paths;
    vector<Clipper2Lib::PathsD> skirt;
    Clipper2Lib::PathsD outerWall;
    Clipper2Lib::PathsD innerWall; //inner wall is a part of shell, but is not considered in the printing process, it is just the last shell, but it is easier to reference like this when clipping the infill
    std::vector<Clipper2Lib::PathsD> shells;
    Clipper2Lib::PathsD infill;
    Clipper2Lib::PathsD surfaceWall;
    Clipper2Lib::PathsD surface;
    std::vector<Clipper2Lib::PathsD> roofAdjacences;
    std::vector<Clipper2Lib::PathsD> floorAdjacences;
};

class Slicing 
{
private:
public:
    static vector<Slice> SliceModel(vector<Vertex> model, SlicerSettings settings);
};

vector<Slice> Slicing::SliceModel(vector<Vertex> model, SlicerSettings settings) {
    float layerHeight = settings.GetLayerHeight();
    settings.SetSlicingPlaneHeight(layerHeight / 2);

    CreateInfill infillCreator;

    // make infill once, then retrieve it for each slice
    infillCreator.CreateDiagonalInfill(settings.GetInfill(), settings);
    infillCreator.CreateSurfaceInfill(0, settings);
    infillCreator.CreateSurfaceInfill(1, settings);


    vector<Slice> slices;
    bool nonEmpty = true;
#pragma omp parallel
    while (nonEmpty)
    {
        double intersectionHeight = settings.GetSlicingPlaneHeight();
        settings.SetSlicingPlaneHeight(intersectionHeight + layerHeight);

        Clipper2Lib::PathsD paths = CalculateIntersections::CalculateClipperPaths(model, settings, intersectionHeight);
        if (paths.size() == 0)
        {
            nonEmpty = false;
        }
        else
        {
            Slice slice;
            slice.height = intersectionHeight;
            slice.paths = paths;
#pragma omp critical
            slices.push_back(slice);
        }
    }

    //sort slices by height
    std::sort(slices.begin(), slices.end(), [](Slice a, Slice b) {
        return a.height < b.height;
        });

    //remove duplicates -> remove slices with the same height
    slices.erase(std::unique(slices.begin(), slices.end(), [](Slice a, Slice b) {
        return a.height == b.height;
        }), slices.end());

#pragma omp parallel for
    for (int i = 0; i < slices.size(); i++) {
        Slice slice = slices[i];
        Clipper2Lib::PathsD paths = slice.paths;
        //erode outerWall by half the nozzle diameter
        paths = Clipper2Lib::InflatePaths(paths, -settings.GetNozzleDiameter() / 2, Clipper2Lib::JoinType::Miter, Clipper2Lib::EndType::Polygon, 3);
        paths = Clipper2Lib::SimplifyPaths(paths, 0.00125);
        slice.outerWall = paths;
        slice.innerWall = paths;


        //add inner shells
        std::vector<Clipper2Lib::PathsD> shells;
        Clipper2Lib::PathsD lastPaths = paths;
        for (int i = 0; i < settings.GetShells() - 1; i++)
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

        slices[i] = slice;
    }


    if (settings.GetSkirt().enabled) {
        for (int i = 0; i < settings.GetSkirt().height; i++)
        {
            Slice skirtSlice = slices[0];
            for (int j = 0; j < settings.GetSkirt().lines; j++) {
                Clipper2Lib::PathsD skirtLine = Clipper2Lib::InflatePaths(skirtSlice.outerWall, settings.GetNozzleDiameter() * j + settings.GetSkirt().distance, Clipper2Lib::JoinType::Round, Clipper2Lib::EndType::Polygon);
                slices[i].skirt.push_back(skirtLine);
            }
        }
    }

    // calculate surfaces
    #pragma omp parallel for
    for (int i = 0; i < slices.size(); i++)
    {
        Slice curSlice = slices[i];
        if (i < slices.size() - settings.GetRoofs())
        {
            for (int j = i + 1; j <= i + settings.GetRoofs(); j++)
            {
                curSlice.roofAdjacences.push_back(slices[j].innerWall);
            }
        } else {
            curSlice.roofAdjacences.push_back(Clipper2Lib::PathsD());
        }

        if (i >= settings.GetFloors())
        {
            for (int j = i - settings.GetFloors(); j < i; j++)
            {
                curSlice.floorAdjacences.push_back(slices[j].innerWall);
            }
        } else {
            curSlice.floorAdjacences.push_back(Clipper2Lib::PathsD());
        }

        slices[i] = curSlice;
    }

    #pragma omp parallel for
    for (int i = 0; i < slices.size(); i++)
    {
        Slice curSlice = slices[i];

        Clipper2Lib::PathsD offsettedInnerWall = Clipper2Lib::InflatePaths(curSlice.innerWall, -settings.GetNozzleDiameter(), Clipper2Lib::JoinType::Miter, Clipper2Lib::EndType::Polygon);
        
        curSlice.surfaceWall = Surface::CalculateSurface(offsettedInnerWall, curSlice.floorAdjacences, curSlice.roofAdjacences);
        Clipper2Lib::PathsD sparseInfillClipArea = Surface::CalculateSurface(curSlice.innerWall, curSlice.floorAdjacences, curSlice.roofAdjacences);


        //generate infill
        //Take difference of innerwall sparseInfillClipArea
        curSlice.infill = infillCreator.GetInfill();

        //calculate clipping area
        Clipper2Lib::PathsD sparseInfillClip = Clipper2Lib::Difference(offsettedInnerWall, sparseInfillClipArea, Clipper2Lib::FillRule::EvenOdd);
        curSlice.infill = infillCreator.ClipInfill(curSlice.infill, sparseInfillClip);


        //calculate surfaceInfill
        Clipper2Lib::PathsD surfaceInfill = infillCreator.GetSurface(i);
        Clipper2Lib::PathsD inflatedWall = Clipper2Lib::InflatePaths(curSlice.surfaceWall, -settings.GetNozzleDiameter() / 2, Clipper2Lib::JoinType::Miter, Clipper2Lib::EndType::Polygon);
        curSlice.surface = infillCreator.ClipInfill(surfaceInfill, inflatedWall);
        slices[i] = curSlice;
    }

    return slices;
}

#endif