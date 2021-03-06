#include <openvdb/openvdb.h>
#include <iostream>
#include <openvdb/tools/VolumeToMesh.h>
#include <openvdb/tools/Composite.h>
#include <fstream>

#include "exportObj.h"
#include "createSimplexNoise.h"
#include "createBox.h"
#include "createFunction.h"

#include <openvdb/tools/GridTransformer.h>

#include "simplexnoise1234.h"

// Populate the given grid with a narrow-band level set representation of a sphere.
// The width of the narrow band is determined by the grid's background value.
// (Example code only; use tools::createSphereSDF() in production.)
template<class GridType>
void
makeSphere(GridType& grid, float radius, const openvdb::Vec3f& c)
{
    typedef typename GridType::ValueType ValueT;

    // Distance value for the constant region exterior to the narrow band
    const ValueT outside = grid.background();

    // Distance value for the constant region interior to the narrow band
    // (by convention, the signed distance is negative in the interior of
    // a level set)
    const ValueT inside = -outside;

    // Use the background value as the width in voxels of the narrow band.
    // (The narrow band is centered on the surface of the sphere, which
    // has distance 0.)
    int padding = int(openvdb::math::RoundUp(openvdb::math::Abs(outside)));
    // The bounding box of the narrow band is 2*dim voxels on a side.
    int dim = int(radius + padding);

    // Get a voxel accessor.
    typename GridType::Accessor accessor = grid.getAccessor();

    // Compute the signed distance from the surface of the sphere of each
    // voxel within the bounding box and insert the value into the grid
    // if it is smaller in magnitude than the background value.
    openvdb::Coord ijk;
    int &i = ijk[0], &j = ijk[1], &k = ijk[2];
    for (i = c[0] - dim; i < c[0] + dim; ++i) {
        const float x2 = openvdb::math::Pow2(i - c[0]);
        for (j = c[1] - dim; j < c[1] + dim; ++j) {
            const float x2y2 = openvdb::math::Pow2(j - c[1]) + x2;
            for (k = c[2] - dim; k < c[2] + dim; ++k) {

                // The distance from the sphere surface in voxels
                const float dist = openvdb::math::Sqrt(x2y2
                    + openvdb::math::Pow2(k - c[2])) - radius;

                // Convert the floating-point distance to the grid's value type.
                ValueT val = ValueT(dist);

                // Only insert distances that are smaller in magnitude than
                // the background value.
                if (val < inside || outside < val) continue;

                // Set the distance for voxel (i,j,k).
                accessor.setValue(ijk,val);
            }
        }
    }

    // Propagate the outside/inside sign information from the narrow band
    // throughout the grid.
    grid.signedFloodFill();
}


int main()
{
  std::cout << "starting " << std::endl;
  // Initialize the OpenVDB library.  This must be called at least
    // once per program and may safely be called multiple times.
    openvdb::initialize();

    // Create an empty floating-point grid with background value 0. 
    openvdb::FloatGrid::Ptr grid = openvdb::FloatGrid::create(/*background value=*/100.0);

    // Get an accessor for coordinate-based access to voxels.
    openvdb::FloatGrid::Accessor accessor = grid->getAccessor();
    std::cout << "making sphere" << std::endl;

    //makeSphere(*grid, /*radius=*/50.0, /*center=*/openvdb::Vec3f(1.5, 2, 3));
    std::cout << "done sphere" << std::endl;
    std::vector<openvdb::Vec3f> scales{openvdb::Vec3f(.001, .001, .002)};
    
    //grid = createBox<openvdb::FloatGrid>(openvdb::Vec3i(40, 40, 20));
    SimplexNoise1234 s;
  
    grid = createFunction<openvdb::FloatGrid>(openvdb::Vec3i(60, 60, 60), [&](int x, int y, int z) {
      return y;
    });
    
    //auto grid2 = createBox<openvdb::FloatGrid>(openvdb::Vec3i(6, 2, 2));
    auto noiseGrid = createSimplexNoise <openvdb::FloatGrid>(openvdb::Vec3i(60,60,60), {openvdb::Vec3f(.1, .1, .1)});
    
    openvdb::FloatGrid::Ptr scalarGrid = openvdb::FloatGrid::create(/*background value=*/10.0);

    openvdb::tools::compMul(*noiseGrid, *scalarGrid);
    openvdb::tools::compSum(*grid, *noiseGrid);
    
    std::ofstream file;
    file.open("out.obj");
    file << exportObj(*grid);
    file.close();
}
