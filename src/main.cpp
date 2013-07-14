#include <openvdb/openvdb.h>
#include <iostream>
#include <openvdb/tools/VolumeToMesh.h>
#include <fstream>

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
    openvdb::FloatGrid::Ptr grid = openvdb::FloatGrid::create(/*background value=*/2.0);

    // Get an accessor for coordinate-based access to voxels.
    openvdb::FloatGrid::Accessor accessor = grid->getAccessor();
    std::cout << "making sphere" << std::endl;

    makeSphere(*grid, /*radius=*/50.0, /*center=*/openvdb::Vec3f(1.5, 2, 3));
    std::cout << "done sphere" << std::endl;

    openvdb::tools::VolumeToMesh mesher(0);
    mesher(*grid);
    
    std::ofstream file;
    file.precision(5);
    file.open("out.obj");
 
    file << "g object" << std::endl << std::endl;

    using openvdb::Index64;

    // Copy points and generate point normals.

    for (Index64 n = 0, i = 0,  N = mesher.pointListSize(); n < N; ++n) {
        const openvdb::Vec3s& p = mesher.pointList()[n];
        file << "v " << p[0] << " " << p[1] << " " << p[2] << std::endl;
    }
    std::cout << mesher.pointListSize()<< " points found" << std::endl;
    
    file << std::endl;
    // Copy primitives
    
    std::cout << mesher.polygonPoolListSize() << "pool size" << std::endl;
    openvdb::tools::PolygonPoolList& polygonPoolList = mesher.polygonPoolList();
    Index64 numQuads = 0;
    for (Index64 n = 0, N = mesher.polygonPoolListSize(); n < N; ++n) {
        std::cout << polygonPoolList[n].numQuads() << ", " << polygonPoolList[n].numQuads() << std::endl;
        numQuads += polygonPoolList[n].numQuads();
    }
    std::cout << numQuads << " Quads found" << std::endl;

    std::vector<openvdb::Vec4I> indices;
    indices.reserve(numQuads);
    openvdb::Vec3d normal, e1, e2;

    for (Index64 n = 0, N = mesher.polygonPoolListSize(); n < N; ++n) {
        const openvdb::tools::PolygonPool& polygons = polygonPoolList[n];
        for (Index64 i = 0, I = polygons.numQuads(); i < I; ++i) {
            const openvdb::Vec4I& quad = polygons.quad(i);
            indices.push_back(quad);

            e1 = mesher.pointList()[quad[1]];
            e1 -= mesher.pointList()[quad[0]];
            e2 = mesher.pointList()[quad[2]];
            e2 -= mesher.pointList()[quad[1]];
            normal = e1.cross(e2);

            file << "f " << quad[0]+1 << " " << quad[1]+1 << " " << quad[2]+1 <<" " << quad[3]+1 << std::endl;
            const double length = normal.length();
            if (length > 1.0e-7) normal *= (1.0 / length);

            //file << "vn " << normal[0] << " " << normal[1] << " " << normal[2] << std::endl;
        }
    }
    
    file << std::endl;
    
    int onNormal = 0;
    for (openvdb::Vec4I indice : indices) {
      //file << "f " << indice[0] << " " << indice[1] << " " << indice[2] << " " << indice[3] << std::endl;
    }

}
