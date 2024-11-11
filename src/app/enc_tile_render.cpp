#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <gdal.h>
#include <encviz/enc_dataset.h>
#include <encviz/enc_renderer.h>
#include <encviz/web_mercator.h>

void usage(int exit_code)
{
    printf("Usage:\n"
           "  enc_tile_render [opts] <ENC_ROOT> <STYLE.xml> <X> <Y> <Z>\n"
           "\n"
           "Options:\n"
           "  -h         - Show help\n"
           "  -c <type>  - Set tile system (wmts or xyz, default=xyz)\n"
           "  -o <file>  - Set output file (default=out.png)\n"
           "  -s <value> - Set min display scale at zoom = 0, default=1e9\n"
           "  -t <size>  - Set tile size (default=256)\n"
           "\n"
           "Where:\n"
           "  ENC ROOT   - Path to folder containing ENC charts\n"
           "  X          - Horizontal tile coordinate\n"
           "  Y          - Vertical tile coordinate\n"
           "  Z          - Zoom tile coordinate\n");
    exit(exit_code);
}

int main(int argc, char **argv)
{
    int opt;
    int tile_size = 256;
    double scale0 = 1e9;
    encviz::tile_coords tc = encviz::tile_coords::XYZ;
    std::string out_file = "out.png";

    // Parse args
    while ((opt = getopt(argc, argv, "hc:o:s:t:")) != -1)
    {
        switch (opt)
        {
            case 'h':
                // Help text
                usage(0);
                break;

            case 'c':
                // Set tile coordinate system
                if (strcmp(optarg, "xyz") == 0)
                {
                    tc = encviz::tile_coords::XYZ;
                }
                else if (strcmp(optarg, "wtms") == 0)
                {
                    tc = encviz::tile_coords::WTMS;
                }
                else
                {
                    printf("Invalid tile coordinate system: %s\n\n", optarg);
                    usage(1);
                }
                break;

            case 'o':
                // Set output file
                out_file = optarg;
                break;

            case 's':
                // Set min display scale
                scale0 = atof(optarg);
                break;

            case 't':
                // Set tile size
                tile_size = atoi(optarg);
                break;

            default:
                // Invalid arg / missing argument
                usage(1);
                break;
        }
    }
    if ((argc - optind) < 5)
    {
        usage(1);
    }
    int x = std::atoi(argv[optind + 2]);
    int y = std::atoi(argv[optind + 3]);
    int z = std::atoi(argv[optind + 4]);

    // Global GDAL Initialization
    GDALAllRegister();

    encviz::render_style style = encviz::load_style(argv[optind + 1]);

    std::vector<uint8_t> png_bytes;
    encviz::enc_renderer enc_rend(tile_size, scale0);
    enc_rend.load_charts(argv[optind]);
    enc_rend.render(png_bytes, tc, x, y, z, style);

    // Dump to file
    printf("Writing %lu bytes\n", png_bytes.size());
    FILE *ohandle = fopen(out_file.c_str(), "wb");
    fwrite(png_bytes.data(), 1, png_bytes.size(), ohandle);
    fclose(ohandle);

    GDALDestroy();
    return 0;
}
