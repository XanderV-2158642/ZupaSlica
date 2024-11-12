#ifndef SLICERSETTINGS_H
#define SLICERSETTINGS_H

class SlicerSettings
{
private:
    double slicingPlaneHeight; //mm
    float layerHeight; //mm
    float nozzleDiameter; //mm
    int shells;

public:
    double GetSlicingPlaneHeight() { return slicingPlaneHeight; }
    void SetSlicingPlaneHeight(float height) { slicingPlaneHeight = height + 0.000000001; }

    void SetLayerHeight(float height) { layerHeight = height; }
    float GetLayerHeight() { return layerHeight; }

    void SetNozzleDiameter(float diameter) { nozzleDiameter = diameter; }
    float GetNozzleDiameter() { return nozzleDiameter; }

    void SetShells(int shell) { shells = shell; }
    int GetShells() { return shells; }

    SlicerSettings();
    ~SlicerSettings();
};

SlicerSettings::SlicerSettings() : slicingPlaneHeight(0.000000001) , layerHeight(0.2f), nozzleDiameter(0.4f), shells(2)
{
}




SlicerSettings::~SlicerSettings()
{
}
#endif