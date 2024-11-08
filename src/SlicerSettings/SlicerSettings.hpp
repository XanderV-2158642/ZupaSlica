#ifndef SLICERSETTINGS_H
#define SLICERSETTINGS_H

class SlicerSettings
{
private:
    float slicingPlaneHeight;
public:
    float GetSlicingPlaneHeight() { return slicingPlaneHeight; }
    void SetSlicingPlaneHeight(float height) { slicingPlaneHeight = height; }

    SlicerSettings();
    ~SlicerSettings();
};

SlicerSettings::SlicerSettings() : slicingPlaneHeight(0.0f)
{
}


SlicerSettings::~SlicerSettings()
{
}
#endif