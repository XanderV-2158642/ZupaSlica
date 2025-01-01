#ifndef GCODEWRITER_H
#define GCODEWRITER_H
#include <vector>
#include "../TriangleIntersections/CalculateIntersections.hpp"
#include "../../SlicerSettings/SlicerSettings.hpp"
#include "../Slicing.hpp"

#include <clipper2/clipper.h>


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
"G1 Z2.0 F3000 ; Move Z Axis up little to prevent scratching of Heat Bed\n"
"G1 X5 Y20 Z0.3 F5000.0 ; Move over to prevent blob squish\n"
"G92 E0 \n"
"G1 F2700 E-5 \n"
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
    double extrusionVal = 0;
    double layerHeight = DEFAULT_LAYER_HEIGHT;
    double width = DEFAULT_WIDTH;
    float speed = 50.0f;

    float bedCenterX = 110;
    float bedCenterY = 110;

    double extrudedLength = -5;

    bool retract = true;
    bool retracted = true;
    double retractionLength = 5.0;
    double retractionSpeed = 2700;

    SlicerSettings* settings;

    void UpdateParams()
    {
        this->extrusionVal = 2.40528f;
        this->layerHeight = settings->GetLayerHeight();
        this->width = settings->GetNozzleDiameter();
        bedCenterX = settings->GetBuildVolume().x / 2;
        bedCenterY = settings->GetBuildVolume().y / 2;
    }

    void WriteSkirt(ofstream& file, vector<Clipper2Lib::PathsD>& skirts, double height);
    void WriteShells(ofstream &file, vector<Clipper2Lib::PathsD> &shells, double height);
    void WriteWalls(ofstream &file, Clipper2Lib::PathsD &walls, double height);
    void WriteInfill(ofstream &file, Clipper2Lib::PathsD &infill, double height);
    void WriteSurfaceWalls(ofstream &file, Clipper2Lib::PathsD &walls, double height);
    void WriteSurfaceInfill(ofstream &file, Clipper2Lib::PathsD &infill, double height);

public:
    GCodeWriter(SlicerSettings &settings)
    {
        this->extrusionVal = 2.40528;
        this->layerHeight = DEFAULT_LAYER_HEIGHT;
        this->width = DEFAULT_WIDTH;
        this->settings = &settings;
    }

    void SetPrintSpeed(float speed){
		this->speed = speed;
	}

    const float GetPrintSpeed(){
        return this->speed;
    }

    void WriteGCode(const char* dirname, vector<VertexLine> &lines);
    void WriteGCode(const char* dirname, Clipper2Lib::PathsD &paths);
    void WriteGCode(string dirname, vector<Slice> &slices);
};

void GCodeWriter::WriteGCode(const char* dirname, vector<VertexLine> &lines)
{

    // create file inside directory
    string filename = string(dirname) + "/output.gcode";
    
    // filename 
    ofstream file;
    file.open(filename);

    file << GCODE_HEADER;

    extrudedLength = 0;

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

void GCodeWriter::WriteGCode(const char* dirname, Clipper2Lib::PathsD &paths)
{
    string filename = string(dirname) + "/output.gcode";

    ofstream file;
    file.open(filename);

    file << GCODE_HEADER;

    extrudedLength = 0;

    for(int i = 0; i<paths.size(); i++){
        file << "G0 F2400 X" << paths[i][0].x + bedCenterX << " Y" << paths[i][0].y + bedCenterY << " Z" << 0.2f << "\n";
        file << "G1 F1200" << " E" << extrudedLength << "\n";
        for (int j = 1; j < paths[i].size(); j++){
            float distance = glm::distance(glm::vec2(paths[i][j].x, paths[i][j].y), glm::vec2(paths[i][j-1].x, paths[i][j-1].y));
            float E = distance * width * layerHeight * extrusionVal;
            extrudedLength += E;
            file << "G1 X" << paths[i][j].x + bedCenterX << " Y" << paths[i][j].y + bedCenterY << " E" << extrudedLength << "\n";
        }
        //close the path
        float distance = glm::distance(glm::vec2(paths[i][0].x, paths[i][0].y), glm::vec2(paths[i][paths[i].size()-1].x, paths[i][paths[i].size()-1].y));
        float E = distance * width * layerHeight * extrusionVal;
        extrudedLength += E;
        file << "G1 X" << paths[i][0].x + bedCenterX << " Y" <<  paths[i][0].y << " E" << extrudedLength << "\n";
    }


    file << GCODE_FOOTER;
}

void GCodeWriter::WriteGCode(string dirname, vector<Slice> &slices) {
    UpdateParams();
    string filename = dirname + "/output.gcode";

    ofstream file;
    file.open(filename);

    file << GCODE_HEADER;

    extrudedLength = -5;
    for (int i = 0; i < slices.size(); i++){
        Slice slice = slices[i];
        
        // skirt or brim first
        WriteSkirt(file, slice.skirt, layerHeight * (i + 1));
        // shells first
        WriteShells(file, slice.shells, layerHeight*(i+1));
        //then walls
        WriteWalls(file, slice.outerWall, layerHeight*(i+1));
        //then surface walls
        WriteSurfaceWalls(file, slice.surfaceWall, layerHeight*(i+1));
        //then surface infill
        WriteSurfaceInfill(file, slice.surface, layerHeight*(i+1));
        //then infill
        WriteInfill(file, slice.infill, layerHeight*(i+1));

        // turn on fan in the first three layers
        if (i == 0){
            file << "M106 S85;fan on \n";
        }
        if (i == 1){
            file << "M106 S170 ;fan on \n";
        }
        if (i == 2){
            file << "M106 S255 ;fan on \n";
        }

    }
    file << GCODE_FOOTER;

    file.close();
}

void GCodeWriter::WriteSkirt(ofstream& file, vector<Clipper2Lib::PathsD>& skirts, double height){
    if (skirts.size() == 0){
		return;
	}
	string speedString = "F" + to_string(this->speed*60);
	string printSpeed = "F" + to_string(this->speed*30);
	for (int i = 0; i < skirts.size(); i++){
		Clipper2Lib::PathD path = skirts[i][0];
		// go to start of path
		file << "G0 " << speedString << " X" << path[0].x + bedCenterX << " Y" << path[0].y + bedCenterY << " Z" << height << "\n";
		if (retracted){
			extrudedLength += retractionLength;
			retracted = false;
		}
		file << "G1 " << printSpeed << " E" << to_string(extrudedLength) << "\n";

		//print the path
		for (int k = 1; k < path.size(); k++){
			double distance = glm::distance(glm::vec2(path[k].x, path[k].y), glm::vec2(path[k-1].x, path[k-1].y));
			double E = distance * width * layerHeight / extrusionVal;
			extrudedLength += E;
			string extruded = " E" + to_string(extrudedLength);
			file << "G1 " << printSpeed << " X" << path[k].x + bedCenterX << " Y" << path[k].y + bedCenterY << extruded << "\n";
		}
	}
	//retract
	if (retract && !retracted){
		extrudedLength -= retractionLength;
		retracted = true;
		file << "G1 E" << to_string(extrudedLength) << " F" << to_string(retractionSpeed) << "\n";
	}
}


void GCodeWriter::WriteShells(ofstream &file, vector<Clipper2Lib::PathsD> &shells, double height){
    string speedString = "F" + to_string(this->speed*60);
    string printSpeed = "F" + to_string(this->speed*30);
    for (int i = 0; i < shells.size(); i ++){
        for (int j = 0; j < shells[i].size(); j++){
            Clipper2Lib::PathD path = shells[i][j];
            // go to start of path
            file << "G0 " << speedString << " X" << path[0].x + bedCenterX << " Y" << path[0].y + bedCenterY << " Z" << height << "\n";
            if (retracted){
                extrudedLength += retractionLength;
                retracted = false;
            }
            file << "G1 " << printSpeed << " E" << to_string(extrudedLength) << "\n";

            //print the path
            for (int k = 1; k < path.size(); k++){
                double distance = glm::distance(glm::vec2(path[k].x, path[k].y), glm::vec2(path[k-1].x, path[k-1].y));
                double E = distance * width * layerHeight / extrusionVal;
                extrudedLength += E;
                string extruded = " E" + to_string(extrudedLength);
                file << "G1 " << printSpeed << " X" << path[k].x + bedCenterX << " Y" << path[k].y + bedCenterY << extruded << "\n";
            }

            //close the path
            double distance = glm::distance(glm::vec2(path[0].x, path[0].y), glm::vec2(path[path.size()-1].x, path[path.size()-1].y));
            double E = distance * width * layerHeight / extrusionVal;
            extrudedLength += E;
            file << "G1 " << printSpeed << " X" << path[0].x + bedCenterX << " Y" << path[0].y + bedCenterY << " E" << to_string(extrudedLength) << "\n";

            //retract
            if (retract && !retracted){
                extrudedLength -= retractionLength;
                retracted = true;
                file << "G1 E" << to_string(extrudedLength) << " F" << to_string(retractionSpeed) << "\n";
            }
        }
    }
}

void GCodeWriter::WriteWalls(ofstream &file, Clipper2Lib::PathsD &walls, double height){
    string speedString = "F" + to_string(this->speed*60);
    string printSpeed = "F" + to_string(this->speed*30);
    for (int i = 0; i < walls.size(); i++){
        Clipper2Lib::PathD path = walls[i];
        // go to start of path
        file << "G0 " << speedString << " X" << path[0].x + bedCenterX << " Y" << path[0].y + bedCenterY << " Z" << height << "\n";
        if (retracted){
            extrudedLength += retractionLength;
            retracted = false;
        }
        file << "G1 " << printSpeed << " E" << to_string(extrudedLength) << "\n";

        //print the path
        for (int k = 1; k < path.size(); k++){
            double distance = glm::distance(glm::vec2(path[k].x, path[k].y), glm::vec2(path[k-1].x, path[k-1].y));
            double E = distance * width * layerHeight / extrusionVal;
            extrudedLength += E;
            string extruded = " E" + to_string(extrudedLength);
            file << "G1 " << printSpeed << " X" << path[k].x + bedCenterX << " Y" << path[k].y + bedCenterY << extruded << "\n";
        }

        //close the path
        double distance = glm::distance(glm::vec2(path[0].x, path[0].y), glm::vec2(path[path.size()-1].x, path[path.size()-1].y));
        double E = distance * width * layerHeight / extrusionVal;
        extrudedLength += E;
        file << "G1 " << printSpeed << " X" << path[0].x + bedCenterX << " Y" << path[0].y + bedCenterY << " E" << to_string(extrudedLength) << "\n";

        //retract
        if (retract && !retracted){
            extrudedLength -= retractionLength;
            retracted = true;
            file << "G1 E" << to_string(extrudedLength) << " F" << to_string(retractionSpeed) << "\n";
        }
    }
}

void GCodeWriter::WriteInfill(ofstream &file, Clipper2Lib::PathsD &infill, double height){
    string speedString = "F" + to_string(this->speed*60);
    string printSpeed = "F" + to_string(this->speed*60);
    for (int i = 0; i < infill.size(); i++){
        Clipper2Lib::PathD path = infill[i];
        // go to start of path
        file << "G0 " << speedString << " X" << path[0].x + bedCenterX << " Y" << path[0].y + bedCenterY << " Z" << height << "\n";
        if (retracted){
            extrudedLength += retractionLength;
            retracted = false;
        }
        file << "G1 " << printSpeed << " E" << to_string(extrudedLength) << "\n";

        //print the path
        for (int k = 1; k < path.size(); k++){
            double distance = glm::distance(glm::vec2(path[k].x, path[k].y), glm::vec2(path[k-1].x, path[k-1].y));
            double E = distance * width * layerHeight / extrusionVal;
            extrudedLength += E;
            string extruded = " E" + to_string(extrudedLength);
            file << "G1 " << printSpeed << " X" << path[k].x + bedCenterX << " Y" << path[k].y + bedCenterY << extruded << "\n";
        }
    }
    //retract
    if (retract && !retracted){
        extrudedLength -= retractionLength;
        retracted = true;
        file << "G1 E" << to_string(extrudedLength) << " F" << to_string(retractionSpeed) << "\n";
    }
}

void GCodeWriter::WriteSurfaceWalls(ofstream &file, Clipper2Lib::PathsD &walls, double height){
    string speedString = "F" + to_string(this->speed*60);
    string printSpeed = "F" + to_string(this->speed*30);
    for (int i = 0; i < walls.size(); i++){
        Clipper2Lib::PathD path = walls[i];
        // go to start of path
        file << "G0 " << speedString << " X" << path[0].x + bedCenterX << " Y" << path[0].y + bedCenterY << " Z" << height << "\n";
        if (retracted){
            extrudedLength += retractionLength;
            retracted = false;
        }
        file << "G1 " << printSpeed << " E" << to_string(extrudedLength) << "\n";

        //print the path
        for (int k = 1; k < path.size(); k++){
            double distance = glm::distance(glm::vec2(path[k].x, path[k].y), glm::vec2(path[k-1].x, path[k-1].y));
            double E = distance * width * layerHeight / extrusionVal;
            extrudedLength += E;
            string extruded = " E" + to_string(extrudedLength);
            file << "G1 " << printSpeed << " X" << path[k].x + bedCenterX << " Y" << path[k].y + bedCenterY << extruded << "\n";
        }

        //close the path
        double distance = glm::distance(glm::vec2(path[0].x, path[0].y), glm::vec2(path[path.size()-1].x, path[path.size()-1].y));
        double E = distance * width * layerHeight / extrusionVal;
        extrudedLength += E;
        file << "G1 " << printSpeed << " X" << path[0].x + bedCenterX << " Y" << path[0].y + bedCenterY << " E" << to_string(extrudedLength) << "\n";

        //retract
        if (retract && !retracted){
            extrudedLength -= retractionLength;
            retracted = true;
            file << "G1 E" << to_string(extrudedLength) << " F" << to_string(retractionSpeed) << "\n";
        }
    }
}

void GCodeWriter::WriteSurfaceInfill(ofstream &file, Clipper2Lib::PathsD &infill, double height){
    string speedString = "F" + to_string(this->speed*60);
    string printSpeed = "F" + to_string(this->speed*30);
    for (int i = 0; i < infill.size(); i++){
        Clipper2Lib::PathD path = infill[i];
        // go to start of path
        file << "G0 " << speedString << " X" << path[0].x + bedCenterX << " Y" << path[0].y + bedCenterY << " Z" << height << "\n";
        if (retracted){
            extrudedLength += retractionLength;
            retracted = false;
        }
        file << "G1 " << printSpeed << " E" << to_string(extrudedLength) << "\n";

        //print the path
        for (int k = 1; k < path.size(); k++){
            double distance = glm::distance(glm::vec2(path[k].x, path[k].y), glm::vec2(path[k-1].x, path[k-1].y));
            double E = distance * width * layerHeight / extrusionVal;
            extrudedLength += E;
            string extruded = " E" + to_string(extrudedLength);
            file << "G1 " << printSpeed << " X" << path[k].x + bedCenterX << " Y" << path[k].y + bedCenterY << extruded << "\n";
        }
    }
    if (retract && !retracted){
        extrudedLength -= retractionLength;
        retracted = true;
        file << "G1 E" << to_string(extrudedLength) << " F" << to_string(retractionSpeed) << "\n";
    }
}

#endif