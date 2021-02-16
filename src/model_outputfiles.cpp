#include "model.h"
#include "atom.h"
#include "controller.h"
#include "misc.h"
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <filesystem>

///////////////////////////////
// AUX FUNCTION DECLARATIONS //
///////////////////////////////

std::string timeNow();

///////////////////
// RESULT REPORT //
///////////////////

//
bool Model::createOutputFolder(){
  // folder name based on current time to avoid overwriting output files with successive calculations
  output_folder = "./output/" + timeNow();
  if(std::filesystem::create_directories(output_folder)){
    return true;
  }
  else{
    output_folder = "./";
    return false;
  }
}

void Model::createReport(std::string input_filepath, std::vector<std::string> parameters){
  std::ofstream output_report(output_folder+"/MoloVol result report.txt");
  output_report << "MoloVol program: calculation results report\n\n";
  output_report << "Structure file analyzed: " << input_filepath << "\n";
  for(int i = 0; i < parameters.size(); i++){
    output_report << parameters[i] << "\n";
  }






  output_report.close();
}

//////////////////////
// STRUCTURE OUTPUT //
//////////////////////

void Model::writeXYZfile(std::vector<std::tuple<std::string, double, double, double>> &atom_coordinates, std::string output_type){
  std::ofstream output_structure(output_folder+"/structure_"+output_type+".xyz");
  output_structure << output_type << "\nStructure generated with MoloVol\n\n";
  for(int i = 0; i < atom_coordinates.size(); i++){
    output_structure << std::get<0>(atom_coordinates[i]) << " ";
    output_structure << std::to_string(std::get<1>(atom_coordinates[i])) << " ";
    output_structure << std::to_string(std::get<2>(atom_coordinates[i])) << " ";
    output_structure << std::to_string(std::get<3>(atom_coordinates[i])) << "\n";
  }
  output_structure.close();
}


////////////////////////
// SURFACE MAP OUTPUT //
////////////////////////

void Model::writeSurfaceMap(std::string output_dir){
  std::cout << "function called" << std::endl;
  /*
  std::ofstream output_map(output_folder+"/surface_map.dx");
  output_map << "# OpenDX density file generated by MoloVol\n";
  output_map << "# Contains 3D surface map data to read in PyMol or Chimera\n";
  output_map << "# Data is written in C array order: In grid[x,y,z] the axis x is fastest\n";
  output_map << "# varying, then y, then finally x.\n";
  output_map << "object 1 class gridpositions counts " << cell.lv_dim_n_gridsteps[0][0] << " " << cell.lv_dim_n_gridsteps[0][1] << " " << cell.lv_dim_n_gridsteps[0][2] << "\n";
  output_map << "origin " << cell.getMin()[0] + cell.lv_vox_size[0]/2 << " " << cell.getMin()[1] + cell.lv_vox_size[0]/2 << " " << cell.getMin()[2] + cell.lv_vox_size[0]/2 << "\n";
  output_map << "delta " << cell.lv_vox_size[0] << " 0 0\n";
  output_map << "delta 0 " << cell.lv_vox_size[0] << " 0\n";
  output_map << "delta 0 0 " << cell.lv_vox_size[0] << "\n";
  output_map << "object 2 class gridconnections counts " << cell.lv_dim_n_gridsteps[0][0] << " " << cell.lv_dim_n_gridsteps[0][1] << " " << cell.lv_dim_n_gridsteps[0][2] << "\n";
  output_map << "object 3 class array type float rank 0 items " << cell.lv_dim_n_gridsteps[0][3] << "\n";

  int i = 0;
  for(size_t x = 0; x < cell.lv_dim_n_gridsteps[0][0]; x++){
    for(size_t y = 0; y < cell.lv_dim_n_gridsteps[0][1]; y++){
      for(size_t z = 0; z < cell.lv_dim_n_gridsteps[0][2]; z++){
        size_t n = z*cell.lv_dim_n_gridsteps[0][0]*cell.lv_dim_n_gridsteps[0][1]+y*cell.lv_dim_n_gridsteps[0][0]+x;
        if(cell.lv_vox_type[0][n] == 3){
          output_map << 0;
        }
        else if(cell.lv_vox_type[0][n] == 5){
          output_map << 2;
        }
        else if(cell.lv_vox_type[0][n] == 9){
          output_map << 6;
        }
        else if(cell.lv_vox_type[0][n] == 17){
          output_map << 4;
        }
        else if(cell.lv_vox_type[0][n] == 33){
          output_map << 8;
        }
        else if(cell.lv_vox_type[0][n] == 65){
          output_map << 3.3;
        }
        else {
          output_map << -2;
        }
        i++;
        if (i < 3) {
          output_map << " ";
        }
        else if (i == 3) {
          output_map << "\n";
          i = 0;
        }
      }
    }
  }
  if (i < 3) {
    output_map << "\n";
  }
  output_map << "attribute \"dep\" string \"positions\"\n";
  output_map << "object \"density\" class field\n";
  output_map << "component \"positions\" value 1\n";
  output_map << "component \"connections\" value 2\n";
  output_map << "component \"data\" value 3";
    // Close the file
  output_map.close();
*/
}

///////////////////
// AUX FUNCTIONS //
///////////////////

// find the current time and convert to a string in format year-month-day_hour-min-sec
std::string timeNow(){
    time_t rawtime;
    struct tm * timeinfo;
    char buffer[80];

    time (&rawtime);
    timeinfo = localtime (&rawtime);

    strftime (buffer,80,"%Y-%m-%d_%Hh%Mm%Ss",timeinfo);

    return buffer;
}




