#ifndef CREATEINFILL_HPP
#define CREATEINFILL_HPP

#include "clipper2/clipper.h"
#include "../../SlicerSettings/SlicerSettings.hpp"

class CreateInfill
{
private:
public:
    static Clipper2Lib::PathsD CreateRectInfill(float density, SlicerSettings settings);
    static Clipper2Lib::PathsD CreateDiagonalInfill(float density, SlicerSettings settings);
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

Clipper2Lib::PathsD CreateInfill::CreateDiagonalInfill(float density, SlicerSettings settings){
    Clipper2Lib::PathsD paths;

    //convert density to a percentage
    density = density / 100;

    //formula for rect infill density
    // spacing = nozzleDiameter / density
    float spacing = settings.GetNozzleDiameter()*2 / density;

    //create a diagonal line from the bottom left to the top right
    //angle between the this line and horizontal is 45 degrees

    //get the size of the build plate
    float minX = -settings.GetBuildVolume().x/2;
    float minY = -settings.GetBuildVolume().y/2;
    float maxX = settings.GetBuildVolume().x/2;
    float maxY = settings.GetBuildVolume().y/2;

    //check the largest distance to determine used param
    float maxDist = max((maxY=minY), (maxX-minX));

    //calculate the amount of lines needed
    int lines = maxDist / spacing;

    //calculate manhattan values according to pythagorean theorem
    //a^2 + b^2 = c^2
    float x_component = spacing * spacing / 2;



    //draw lines
    for (int i = 0; i < lines; i++)
    {
        Clipper2Lib::PathD path;
        // start point is at min x
        path.push_back(Clipper2Lib::PointD(-maxDist/2 - x_component*i, -maxDist/2 + x_component*i));
        // end point is at max x
        path.push_back(Clipper2Lib::PointD(maxDist/2 - x_component*i, maxDist/2 + x_component*i));
        paths.push_back(path);

        if (i != 0) {
            Clipper2Lib::PathD path2;
            // start point is at min x
            path2.push_back(Clipper2Lib::PointD(-maxDist/2 + x_component*i, -maxDist/2 - x_component*i));
            // end point is at max x
            path2.push_back(Clipper2Lib::PointD(maxDist/2 + x_component*i, maxDist/2 - x_component*i));
            paths.push_back(path2);
        }

        Clipper2Lib::PathD path3;
        // start point is at min x
        path3.push_back(Clipper2Lib::PointD(-maxDist/2 + x_component*i, maxDist/2 + x_component*i));
        // end point is at max x
        path3.push_back(Clipper2Lib::PointD(maxDist/2 + x_component*i, -maxDist/2 + x_component*i));
        paths.push_back(path3);

        if (i != 0) {
            Clipper2Lib::PathD path4;
            // start point is at min x
            path4.push_back(Clipper2Lib::PointD(-maxDist/2 - x_component*i, maxDist/2 - x_component*i));
            // end point is at max x
            path4.push_back(Clipper2Lib::PointD(maxDist/2 - x_component*i, -maxDist/2 - x_component*i));
            paths.push_back(path4);
        }
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