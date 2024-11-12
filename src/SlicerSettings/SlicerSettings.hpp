#ifndef SLICERSETTINGS_H
#define SLICERSETTINGS_H

class SlicerSettings
{
private:
    double slicingPlaneHeight; //mm
    float layerHeight; //mm

public:
    double GetSlicingPlaneHeight() { return slicingPlaneHeight; }
    void SetSlicingPlaneHeight(float height) { slicingPlaneHeight = height + 0.000000001; }

    void SetLayerHeight(float height) { layerHeight = height; }
    float GetLayerHeight() { return layerHeight; }

    SlicerSettings();
    ~SlicerSettings();
};

SlicerSettings::SlicerSettings() : slicingPlaneHeight(0.000000001) , layerHeight(0.2f)
{
}



SlicerSettings::~SlicerSettings()
{
}
#endif