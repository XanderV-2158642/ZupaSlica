#ifndef CREATEINFILL_HPP
#define CREATEINFILL_HPP

#include "clipper2/clipper.h"
#include "../../SlicerSettings/SlicerSettings.hpp"

class CreateInfill
{
private:
public:
    static Clipper2Lib::PathsD CreateRectInfill(float density, SlicerSettings settings);
    static Clipper2Lib::PathsD ClipInfill(Clipper2Lib::PathsD &infill, Clipper2Lib::PathsD &Clip);
};

Clipper2Lib::PathsD CreateInfill::CreateRectInfill(float density, SlicerSettings settings){
    Clipper2Lib::PathsD paths;

    //convert density to a percentage
    density = density / 100;

    //formula for rect infill density
    // spacing = nozzleDiameter / density
    float spacing = settings.GetNozzleDiameter() / density;

    // number of lines in x direction
    int xLines = settings.GetBuildVolume().x / spacing;

    // number of lines in y direction
    int yLines = settings.GetBuildVolume().y / spacing;

    float minX = -settings.GetBuildVolume().x/2;
    float minY = -settings.GetBuildVolume().y/2;
    float maxX = settings.GetBuildVolume().x/2;
    float maxY = settings.GetBuildVolume().y/2;



    for (int i = 0; i < xLines; i++)
    {
        Clipper2Lib::PathD path;
        // start point is at min y
        path.push_back(Clipper2Lib::PointD(minX + i * spacing, minY));
        // end point is at max y
        path.push_back(Clipper2Lib::PointD(minX + i * spacing, maxY));
        paths.push_back(path);
    }

    for (int i = 0; i < yLines; i++)
    {
        Clipper2Lib::PathD path;
        // start point is at min x
        path.push_back(Clipper2Lib::PointD(minX, minY + i * spacing));
        // end point is at max x
        path.push_back(Clipper2Lib::PointD(maxX, minY + i * spacing));
        paths.push_back(path);
    }

    return paths;
}

Clipper2Lib::PathsD CreateInfill::ClipInfill(Clipper2Lib::PathsD &infill, Clipper2Lib::PathsD &Clip){
    Clipper2Lib::ClipperD clipper;
    Clipper2Lib::PathsD clippedTmp;
    Clipper2Lib::PathsD clippedInfill;
    clipper.AddOpenSubject(infill);
    clipper.AddClip(Clip);
    clipper.Execute(Clipper2Lib::ClipType::Intersection, Clipper2Lib::FillRule::EvenOdd, clippedTmp, clippedInfill);
    clipper.Clear();

    return clippedInfill;
}

#endif