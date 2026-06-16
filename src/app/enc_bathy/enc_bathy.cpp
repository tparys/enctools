#include <cstdio>
#include <filesystem>
#include <gdal_priv.h>
#include <ogrsf_frmts.h>
#include <encdata/enc_dataset.h>
#include <enctri/enc_triangulator.h>
#include <encviz/xml_config.h>
namespace fs = std::filesystem;
using namespace encviz;

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
    const char *config_path = nullptr;

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
                config_path = optarg;
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
    fs::path config_resolved;
    if (config_path != nullptr)
    {
        config_resolved = config_path;
    }
    else
    {
        // Default to ~/.enctools
        config_resolved = getenv("HOME");
        config_resolved.append(".enctools");
    }
    fs::path config_file = config_resolved / "config.xml";
    printf(" - Reading %s ...\n", config_file.string().c_str());
    tinyxml2::XMLDocument doc;
    if (doc.LoadFile(config_file.string().c_str()))
    {
        // Parse error?
        throw std::runtime_error("Cannot parse " + config_file.string());
    }

    // Read in config
    tinyxml2::XMLElement *root = doc.RootElement();
    fs::path chart_path = xml_text(xml_query(root, "chart_path"));
    fs::path meta_path = xml_text(xml_query(root, "meta_path"));
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
