#pragma once

#include <openvdb/openvdb.h>
#include "simplexnoise1234.h"
#include <vector>

#include <iostream>


template <typename GridType>
typename GridType::Ptr
createSimplexNoise(openvdb::Vec3i size, std::vector<openvdb::Vec3f> scales)
{
  typedef typename GridType::ValueType ValueT;
  typename GridType::Ptr grid = openvdb::createGrid<GridType>(0);

  //Todo fix cache misses
  SimplexNoise1234 noise;
  auto accessor = grid->getAccessor();
  for (int x=-size.x(); x < size.x(); x++) {
    for (int y=-size.y(); y < size.y(); y++) {
      for (int z=-size.z(); z < size.z(); z++) {
        openvdb::Coord coord(x, y, z);
        float value = noise.noise(x * scales[0].x(),
                                  y * scales[0].y(),
                                  z * scales[0].z());
        accessor.setValue(coord, value);
      }
    }
  }
  return grid;
};
