#include <cstdio>
#include <gdal_priv.h>
#include <ogrsf_frmts.h>
#include <enctri/enc_triangulator.h>

int main(int argc, char **argv)
{
    // Check args
    if (argc < 5)
    {
        printf("Usage: enc_bathy <path/to/enc.000> <driver> <filename> <res>\n");
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

    tri.gdal_rasterize(argv[2], argv[3], atof(argv[4]), -9999);
    //tri.gdal_rasterize("AAIGrid", "output.asc", 1e-3, -9999);

    // Cleanup and exit
    return 0;
}
