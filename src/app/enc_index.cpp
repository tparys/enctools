#include <gdal.h>
#include <encviz/enc_dataset.h>

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
    std::vector<std::string> enc_roots;
    for (int i = 1; i < argc; i++)
    {
        enc_roots.push_back(argv[i]);
    }
    encviz::enc_dataset blarg(enc_roots);

    // Global GDAL Shutdown
    GDALDestroy();

    return 0;
}
