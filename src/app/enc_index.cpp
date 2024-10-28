#include <gdal.h>
#include <encviz/enc_dataset.h>
#include <encviz/web_mercator.h>

int main(int argc, char **argv)
{
    // Check args
    if (argc < 2)
    {
        printf("Usage: enc_index path/to/ENC_ROOT [path/to/ENC_ROOT] ...\n");
        return 1;
    }

    // Global GDAL Initialization
    GDALAllRegister();

    // Load up
    encviz::enc_dataset eds;
    std::vector<std::string> enc_roots;
    for (int i = 1; i < argc; i++)
    {
        eds.load_charts(argv[i]);
    }

    // Web Mercator testing
    std::vector<std::string> layers = { "LNDARE" };
    encviz::web_mercator wm(8, 18, 5);
    OGREnvelope bbox = wm.get_bbox_deg();
    int scale_min = (int)round(2e7 / pow(2, 8));
    eds.export_data(nullptr, layers, bbox, scale_min);

    // Global GDAL Shutdown
    GDALDestroy();

    return 0;
}
