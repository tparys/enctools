#include <cstdio>
#include <ogrsf_frmts.h>

int main(int argc, char **argv)
{
    // Sanity check
    if (argc < 2)
    {
	fprintf(stderr, "Usage: gdal_test <path/to/vector.ext>\n");
	return 1;
    }

    // Global GDAL init (not re-entrant)
    GDALAllRegister();

    // Load the dataset
    GDALDataset *ds = (GDALDataset*)GDALOpenEx(argv[1], GDAL_OF_READONLY,
					       nullptr, nullptr, nullptr);
    if (ds == nullptr)
    {
	fprintf(stderr, "ERROR: Cannot open %s for reading\n", argv[1]);
	return 1;
    }

    // What layers exist?
    for (OGRLayer *layer : ds->GetLayers())
    {
	printf("Layer: %s (%lld features)\n", layer->GetName(),
	       layer->GetFeatureCount());
    }

    // Cleanup
    delete ds;

    // Global GDAL cleanup (not re-entrant)
    GDALDestroy();

    return 0;
}
