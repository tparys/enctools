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

    // Somewhere for the data to go ...
    GDALDataset *ds_out = GetGDALDriverManager()->GetDriverByName("KML")->
        Create("out.kml", 0, 0, 0, GDT_Unknown, nullptr);

    // Web Mercator testing
    std::vector<std::string> layers = { "LNDARE", "DEPCNT" };
    encviz::web_mercator wm(x, y, z);
    OGREnvelope bbox = wm.get_bbox_deg();
    int scale_min = (int)round(1e8 / pow(2, 8));
    eds.export_data(ds_out, layers, bbox, scale_min);

    // Global GDAL Shutdown
    GDALClose(ds_out);
    GDALDestroy();

    return 0;
}
