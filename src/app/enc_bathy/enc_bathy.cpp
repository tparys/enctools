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
    //tri.draw();
    delete ds;

    auto bbox = tri.get_coverage();
    enctri::raster grid;
    double res = 1e-3;
    float nodata = -9999;
    tri.rasterize(grid, bbox, res, nodata);

    // Output grid
    FILE *handle = fopen("output.asc", "w");
    if (handle == nullptr)
    {
        perror("Cannot open output file");
        return 1;
    }
    fprintf(handle, "ncols %lu\n", grid.size_x);
    fprintf(handle, "nrows %lu\n", grid.size_y);
    fprintf(handle, "xllcorner %f\n", bbox.min.x);
    fprintf(handle, "yllcorner %f\n", bbox.min.y);
    fprintf(handle, "cellsize %g\n", res);
    fprintf(handle, "NODATA_value %g\n", nodata);
    for (size_t y = 0; y < grid.size_y; y++)
    {
        size_t yinv = grid.size_y - y - 1;
        for (size_t x = 0; x < grid.size_x; x++)
        {
            fprintf(handle, "%g ", grid.data[(yinv * grid.size_x) + x]);
        }
        fprintf(handle, "\n");
    }
    fclose(handle);

    /*
    // Get triangulation mesh
    auto mesh = tri.get_mesh();

    // Show points
    printf("Points:\n");
    for (size_t i = 0; i < mesh.points.size(); i++)
    {
        printf(" - %lu : %g, %g, %g\n",
               i, mesh.points[i].x, mesh.points[i].y, mesh.points[i].z);
    }

    // Show faces
    printf("Faces:\n");
    for (size_t i = 0; i < mesh.faces.size(); i++)
    {
        printf(" - %lu : %lu, %lu, %lu\n",
               i, mesh.faces[i][0], mesh.faces[i][1], mesh.faces[i][2]);
    }
    */

    // Cleanup and exit
    return 0;
}
