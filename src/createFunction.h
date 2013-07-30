#pragma once

#include <openvdb/openvdb.h>
#include <functional>

template <typename GridType>
typename GridType::Ptr
createFunction(openvdb::Vec3i size,
          std::function<float(int, int, int)> isoFunction,
          float background = 6.0f)
{
  typename GridType::Ptr grid = openvdb::createGrid<GridType>(background);
  auto accessor = grid->getAccessor();
  for (int x=-size.x()-background; x < size.x()+1+background; x++) {
    for (int y=-size.y()-background; y < size.y()+1+background; y++) {
      for (int z=-size.z()-background; z < size.z()+1+background; z++) {
        float value = isoFunction(x, y, z);

        openvdb::Coord coord(x, y, z);
        accessor.setValue(coord, value);
      }
    }
  }
  grid->signedFloodFill();
  return grid;
};
