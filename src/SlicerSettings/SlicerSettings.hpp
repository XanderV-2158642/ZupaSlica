#ifndef SLICERSETTINGS_H
#define SLICERSETTINGS_H

struct BuildVolume
{
    float x;
    float y;
    float z;
};

class SlicerSettings
{
private:
    BuildVolume buildVolume;
    double slicingPlaneHeight; //mm
    float layerHeight; //mm
    float nozzleDiameter; //mm
    int shells;
    float infill; //percentage
    int roofs;
    int floors;

public:
    double GetSlicingPlaneHeight() { return slicingPlaneHeight; }
    void SetSlicingPlaneHeight(float height) { slicingPlaneHeight = height + 0.000000001; }

    void SetLayerHeight(float height) { this->layerHeight = height; printf("Layer height: %f\n", layerHeight); }
    float GetLayerHeight() { return layerHeight; }

    void SetNozzleDiameter(float diameter) { nozzleDiameter = diameter; }
    float GetNozzleDiameter() { return nozzleDiameter; }

    void SetShells(int shell) { shells = shell; }
    int GetShells() { return shells; }

    void SetInfill(float inf) {infill = inf;}
    float GetInfill(){return infill;}

    void SetRoofs(int roof) { roofs = roof; }
    int GetRoofs() { return roofs; }

    void SetFloors(int floor) { floors = floor; }
    int GetFloors() { return floors; }

    void SetBuildVolume(BuildVolume volume) { buildVolume = volume; }
    BuildVolume GetBuildVolume() { return buildVolume; }

    SlicerSettings();
    ~SlicerSettings();
};

SlicerSettings::SlicerSettings() : slicingPlaneHeight(0.000000001) , layerHeight(0.2f), nozzleDiameter(0.4f), shells(2), buildVolume({220,220,250}), infill(20), roofs(3), floors(3)
{
}


SlicerSettings::~SlicerSettings()
{
}
#endif