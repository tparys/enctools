#include <cstdio>
#include <gdal_priv.h>
#include <ogrsf_frmts.h>
#include <enctri/enc_triangulator.h>

int main(int argc, char **argv)
{
    // Check args
    if (argc < 2)
    {
        printf("Usage: enc_bathy <path/to/enc.000>\n");
        return -1;
    }

    // Global GDAL Initialization
    GDALAllRegister();

    // Open dataset
    GDALDataset *ds = GDALDataset::Open(argv[1],
                                        GDAL_OF_VECTOR | GDAL_OF_READONLY,
                                        nullptr, nullptr, nullptr);
    if (!ds)
    {
        throw std::runtime_error("Cannot open ENC dataset");
    }

    // Create a triangulator object
    enctri::enc_triangulator tri(ds);
    tri.draw();

    // Get triangulation mesh
    auto mesh = tri.get();

    // Show points
    printf("Points:\n");
    for (size_t i = 0; i < mesh.points.size(); i++)
    {
        printf(" - %lu : %g, %g, %g\n",
               i, mesh.points[i].x, mesh.points[i].y, mesh.points[i].z);
    }

    // Show faces
    printf("Faces:\n");
    for (size_t i = 0; i < mesh.points.size(); i++)
    {
        printf(" - %lu : %lu, %lu, %lu\n",
               i, mesh.faces[i][0], mesh.faces[i][1], mesh.faces[i][2]);
    }

    // Cleanup and exit
    delete ds;
    return 0;
}
