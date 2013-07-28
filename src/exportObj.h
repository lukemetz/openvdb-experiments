#pragma once

#include <string>
#include <openvdb/openvdb.h>
#include <openvdb/tools/VolumeToMesh.h>
#include <sstream>
#include <iostream>



template <typename GridType>
std::string exportObj(const GridType &grid)
{
  std::vector<openvdb::Vec3s> verts;
  std::vector<openvdb::Vec4I> quads;
  std::vector<openvdb::Vec3I> tris;
  
  float isovalue = 0;
  float adaptivity = 0;
  openvdb::tools::volumeToMesh(grid, verts, tris, quads, isovalue, adaptivity);

  std::stringstream output;
  output << "g object" << std::endl << std::endl;

  using openvdb::Index64;
  
  for (openvdb::Vec3s vert : verts) {
      output << "v " << vert[0]
             << " " << vert[1] 
             << " " << vert[2] 
             << std::endl;
  }
  output << std::endl;

  for (openvdb::Vec4I quad : quads) {
    output << "f " << quad[0]+1
            << " " << quad[1]+1 
            << " " << quad[2]+1 
            <<" " << quad[3]+1
            << std::endl;
  }
  
  for (openvdb::Vec3I tri : tris) {
    output << "f " << tri[0]+1
            << " " << tri[1]+1 
            << " " << tri[2]+1 
            << std::endl;
  }
  output << std::endl;
  
  return output.str();
};
