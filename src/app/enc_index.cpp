#include <gdal.h>
#include <encviz/enc_dataset.h>
#include <encviz/web_mercator.h>

int main(int argc, char **argv)
{
    // Check args
    if (argc < 2)
    {
        printf("Usage: enc_index path/to/ENC_ROOT [x y z]\n");
        return 1;
    }

    // Global GDAL Initialization
    GDALAllRegister();

    // Load up ENC set
    encviz::enc_dataset eds;
    std::vector<std::string> enc_roots;
    eds.load_charts(argv[1]);

    // Pick tile to load
    int x = 8, y = 18, z = 5;
    if (argc >=  5)
    {
        x = std::atoi(argv[2]);
        y = std::atoi(argv[3]);
        z = std::atoi(argv[4]);
    }

    // Web Mercator testing
    std::vector<std::string> layers = { "LNDARE" };
    encviz::web_mercator wm(x, y, z);
    OGREnvelope bbox = wm.get_bbox_deg();
    int scale_min = (int)round(2e7 / pow(2, 8));
    eds.export_data(nullptr, layers, bbox, scale_min);

    // Global GDAL Shutdown
    GDALDestroy();

    return 0;
}
