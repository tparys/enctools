#include <cstdio>
#include <gdal_priv.h>
#include <ogrsf_frmts.h>
#include <enctri/enc_triangulator.h>

void usage(int exit_code)
{
    printf("Usage:\n"
           "  enc_bathy [opts] <in_file> <out_driver> <out_file> <res>\n"
           "\n"
           "Options:\n"
           "  -h           - Show help\n"
           "  -e <epsg_id> - Set output EPSG CRS\n");
    exit(exit_code);
}

int main(int argc, char **argv)
{
    int opt, epsg_id = -1;

    // Parse args
    while ((opt = getopt(argc, argv, "he:")) != -1)
    {
        switch (opt)
        {
            case 'h':
                // Help text
                usage(0);
                break;

            case 'e':
                // Set output EPSG
                epsg_id = atoi(optarg);
                break;

            default:
                // Invalid arg / missing argument
                usage(1);
                break;
        }
    }

    // Check args
    if ((argc - optind) < 4)
    {
        usage(1);
    }

    // Global GDAL Initialization
    GDALAllRegister();

    // Set up coordinate warp if requested
    OGRSpatialReference src_srs, dst_srs;
    OGRCoordinateTransformation *ds_ct = nullptr;
    if (epsg_id != -1)
    {
        // S57 data in WGS 84 (EPSG:4326)
        src_srs.importFromEPSG(4326);
        src_srs.SetAxisMappingStrategy(OAMS_TRADITIONAL_GIS_ORDER);

        // Requested output data
        dst_srs.importFromEPSG(epsg_id);
        dst_srs.SetAxisMappingStrategy(OAMS_TRADITIONAL_GIS_ORDER);

        // Create coordinate warp
        ds_ct = OGRCreateCoordinateTransformation(&src_srs, &dst_srs);
    }

    // Open dataset
    GDALDataset *ds = GDALDataset::Open(argv[optind + 0],
                                        GDAL_OF_VECTOR | GDAL_OF_READONLY,
                                        nullptr, nullptr, nullptr);
    if (!ds)
    {
        throw std::runtime_error("Cannot open ENC dataset");
    }

    // Create a triangulator object
    enctri::enc_triangulator tri(ds, ds_ct);
    //tri.draw();
    delete ds;

    tri.gdal_rasterize(argv[optind + 1], argv[optind + 2],
                       atof(argv[optind + 3]), -9999);

    // Cleanup and exit
    if (ds_ct)
    {
        delete ds_ct;
    }
    return 0;
}
