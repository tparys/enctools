#include <gdal.h>
#include <encviz/enc_dataset.h>
#include <encviz/enc_renderer.h>
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

    // Pick tile to load
    int x = 8, y = 18, z = 5;
    if (argc >=  5)
    {
        x = std::atoi(argv[2]);
        y = std::atoi(argv[3]);
        z = std::atoi(argv[4]);
    }

    if (false)
    {
        // Load up ENC set
        encviz::enc_dataset eds;
        std::vector<std::string> enc_roots;
        eds.load_charts(argv[1]);

        // Somewhere for the data to go ...
        GDALDataset *ds_out = GetGDALDriverManager()->GetDriverByName("KML")->
            Create("out.kml", 0, 0, 0, GDT_Unknown, nullptr);

        // Web Mercator testing
        std::vector<std::string> layers = { "LNDARE", "DEPCNT" };
        encviz::web_mercator wm(x, y, z);
        OGREnvelope bbox = wm.get_bbox_deg();
        int scale_min = (int)round(1e8 / pow(2, z));
        eds.export_data(ds_out, layers, bbox, scale_min);

        // Global GDAL Shutdown
        GDALClose(ds_out);
    }
    else
    {
        encviz::render_style style = {
            // Background color
            {{ 255, 255, 255 }},

            // Layer styles
            {
                { "LNDARE",
                  { 0, 192, 0 },
                  { 0, 255, 0 },
                  1
                },
                { "SLCONS",
                  { 0, 0, 0 },
                  { 0, 0, 0 },
                  1
                },
                { "DEPCNT",
                  {},
                  { 128, 128, 128 },
                  1
                }
            }
        };

        std::vector<uint8_t> png_bytes;
        encviz::enc_renderer enc_rend(256, 1e8);
        enc_rend.load_charts(argv[1]);
        enc_rend.render(png_bytes, x, y, z, style);

        // Dump to file
        printf("Writing %lu bytes\n", png_bytes.size());
        FILE *ohandle = fopen("out.png", "wb");
        fwrite(png_bytes.data(), 1, png_bytes.size(), ohandle);
        fclose(ohandle);
    }

    GDALDestroy();
    return 0;
}
