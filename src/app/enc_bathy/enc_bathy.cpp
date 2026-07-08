#include <cstdio>
#include <filesystem>
#include <gdal_priv.h>
#include <ogrsf_frmts.h>
#include <encdata/os_paths.h>
#include <encdata/config_reader.h>
#include <encdata/enc_dataset.h>
#include <enctri/enc_triangulator.h>
namespace fs = std::filesystem;

void usage(int exit_code)
{
    printf("Usage:\n"
           "  enc_bathy [opts] <in_file> <out_driver> <out_file> <res>\n"
           "\n"
           "Options:\n"
           "  -h           - Show help\n"
           "  -c <path>  - Set config directory (default=~/.enctools/config.xml)\n"
           "  -e <epsg_id> - Set output EPSG CRS\n");
    exit(exit_code);
}

int main(int argc, char **argv)
{
    int opt, epsg_id = -1;
    const char *config = nullptr;

    // Parse args
    while ((opt = getopt(argc, argv, "hc:e:")) != -1)
    {
        switch (opt)
        {
            case 'h':
                // Help text
                usage(0);
                break;

            case 'c':
                // Set config path
                config = optarg;
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

    // ENC Dataset
    encdata::enc_dataset enc_;
    const fs::path share_path = encdata::get_user_share_path(argv[0]);
    fs::path config_path = encdata::get_user_config_path(config);
    fs::path config_file = config_path / "config.xml";
    printf("Using config directory: %s ...\n", config_path.string().c_str());
    printf("Using share directory: %s ...\n", share_path.c_str());

    // Load config file
    encdata::config_reader cfg((config_path / "config.json").string().c_str());

    // Read in config
    fs::path chart_path = cfg.get<std::string>(".data.chart-dir");
    fs::path meta_path = cfg.get<std::string>(".data.meta-dir", "meta");

    // Ensure some paths are absolute
    if (chart_path.is_relative())
        chart_path = config_path / chart_path;
    if (meta_path.is_relative())
        meta_path = config_path / meta_path;

    printf(" - Charts: %s\n", chart_path.string().c_str());
    printf(" - Metadata: %s\n", meta_path.string().c_str());

    // Configure charts
    enc_.set_cache_path(meta_path);
    enc_.load_charts(chart_path);

    // Export named chart
    GDALDataset *chart_data = GetGDALDriverManager()->GetDriverByName(GDAL_MEM_DRIVER)->
        Create("", 0, 0, 0, GDT_Unknown, nullptr);
    OGREnvelope bbox = {};
    std::vector<std::string> layers = { "LNDARE", "SOUNDG" };
    if (!enc_.export_chart(chart_data, bbox, argv[optind + 0], layers))
    {
        return false;
    }

    // Create a triangulator object
    enctri::enc_triangulator tri(chart_data, ds_ct);
    //tri.draw();
    delete chart_data;

    tri.gdal_rasterize(argv[optind + 1], argv[optind + 2],
                       atof(argv[optind + 3]), -9999);

    // Cleanup and exit
    if (ds_ct)
    {
        delete ds_ct;
    }
    return 0;
}
