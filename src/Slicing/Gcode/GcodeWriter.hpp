#ifndef GCODEWRITER_H
#define GCODEWRITER_H
#include <vector>
#include "../TriangleIntersections/CalculateIntersections.hpp"


const float DEFAULT_LAYER_HEIGHT = 0.2f;
const float DEFAULT_WIDTH = 0.4f;

const char* GCODE_HEADER = "M140 s60 ;set bed temperature \n"
"M190 S60 ;wait for bed temperature to be reached \n"
"M104 S200 ;set temperature \n"
"M109 S200 ;wait for temperature to be reached \n"
"M82 ;set extruder to absolute mode \n"
"G28 ;home all axes \n"
"G92 E0 ;zero the extruder \n"
"G1 Z2.0 F3000 ;move the nozzle up \n"
"G1 X0.1 Y20 Z0.3 F5000.0 ;move to start-line position \n"
"G1 X0.1 Y200.0 Z0.3 F1500.0 E15 ;draw 1st line \n"
"G1 X0.4 Y200.0 Z0.3 F5000.0 ;move to side a little \n"
"G1 X0.4 Y20 Z0.3 F1500.0 E30 ;draw 2nd line \n"
"G92 E0 ;zero the extruder \n"
"G1 Z2.0 F3000 ;move the nozzle up \n"
"G92 E0 ;zero the extruder \n"
"G1 F24 \n"
"M107 ; fan off for first layer \n";

const char* GCODE_FOOTER = "M140 s0 ;set bed temperature \n"
"M107 ;fan off \n"
"M220 S100 ;reset speed factor override percentage to default (100%) \n"
"M221 S100 ;reset extrude factor override percentage to default (100%) \n"
"G91 ;relative positioning \n"
"G1 F1800 E-3 ;retract the filament a bit before lifting the nozzle, to release some of the pressure \n"
"G1 F3000 Z20 ;move Z up a bit and retract filament even more \n"
"G90 ;absolute positioning \n"
"G1 X0 Y235 F9000 ;move to park position \n"
"M84 ;steppers off \n"
"M82 ;absolute extrusion mode \n"
"M104 S0 ;turn off extruder \n";

// 0.962
// 0.307

// E target = 0.04702
// dist = 1.019693
// E = dist * width * height
// E = 1.019693 * 0.4 * 0.28 = 0.11420562
// E = 0.04728822

// E = 0.11420562 / 0.04702 = 2.43

//1.62309

// volume = 48.8 * 0.2 *0.4 = 3.904
// 3.904 / 1.62309 = extrusion param
//

//extrusion param = 2.40528

class GCodeWriter
{
private:
    float extrusionVal = 0;
    float layerHeight = DEFAULT_LAYER_HEIGHT;
    float width = DEFAULT_WIDTH;

    float bedCenterX = 110;
    float bedCenterY = 110;

    float extrudedLength = 0;

public:
    GCodeWriter(float layerHeight = DEFAULT_LAYER_HEIGHT, float width = DEFAULT_WIDTH)
    {
        this->extrusionVal = 2.40528f;
        this->layerHeight = layerHeight;
        this->width = width;


    }

    void WriteGCode(const char* dirname, vector<VertexLine> &lines)
    {

        // create file inside directory
        string filename = string(dirname) + "/output.gcode";
        
        // filename 
        ofstream file;
        file.open(filename);

        file << GCODE_HEADER;

        for (int i = 0; i < lines.size(); i++)
        {

            for (int j = 0; j < lines[i].lineSegments.size(); j++)
            {
                if (j == 0)
                {
                    file << "G0 F2400 X" << lines[i].lineSegments[j].v1.Position.x + bedCenterX << " Y" << lines[i].lineSegments[j].v1.Position.y + bedCenterY << " Z" << 0.2f << "\n";
                    file << "G1 F1200" << " E" << extrudedLength << "\n";
                }
                
                VertexPair pair = lines[i].lineSegments[j];
                float dist = glm::distance(pair.v1.Position, pair.v2.Position);
                float E = dist * width * layerHeight * extrusionVal;
                extrudedLength += E;
                //file << "G1 X" << pair.v1.Position.x + bedCenterX << " Y" << pair.v1.Position.y + bedCenterY <<  " E" << extrudedLength << "\n";
                file << "G1 X" << pair.v2.Position.x + bedCenterX << " Y" << pair.v2.Position.y + bedCenterY<<  " E" << extrudedLength << "\n";

                //printf("pair 1 - 2: %f %f - %f %f\n", pair.v1.Position.x, pair.v1.Position.y, pair.v2.Position.x, pair.v2.Position.y);
            }
        }

        file << GCODE_FOOTER;
        file.close();
    }
    
};



#endif