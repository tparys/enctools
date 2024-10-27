#include <cstdio>

#include <stdexcept>
#include <ogrsf_frmts.h>
#include <encviz/enc_metadata.h>

#define CHECKNULL(ptr, msg) if ((ptr) == nullptr) throw std::runtime_error((msg))

namespace encviz
{

/**
 * Constructor
 *
 * param[in] path Path to ENC chart data
 */
enc_metadata::enc_metadata(const std::filesystem::path &path)
    : path_(path)
{
    // Open dataset
    std::string foo = path_.string();
    printf("GDAL Open: %s\n", foo.c_str());
    const char *const drivers[] = { "S57", nullptr };
    GDALDataset *ds = GDALDataset::Open(path_.string().c_str(),
                                        GDAL_OF_VECTOR | GDAL_OF_READONLY,
                                        drivers, nullptr, nullptr);
    CHECKNULL(ds, "Cannot open OGR dataset");

    //
    // Get Compilation Scale of Chart
    //

    // Get "Dataset ID" (DSID) chart metadata
    OGRLayer *layer = ds->GetLayerByName("DSID");
    CHECKNULL(layer, "Cannot open DSID layer");

    // .. which contains one feature
    layer->ResetReading();
    OGRFeature *feat = layer->GetNextFeature();
    CHECKNULL(feat, "Cannot read DSID feature");

    // .. that has "Dataset Parameter" (DSPM) "Compilation of Scale" (CSCL)
    int idx = feat->GetFieldIndex("DSPM_CSCL");
    if (idx == -1) { throw std::runtime_error("Cannot find CSCL field index"); }
    scale_ = feat->GetFieldAsInteger(idx);

    //
    // Get Chart Coverage Bounds
    //

    // Get "Coverage" (M_COVR) data
    layer = ds->GetLayerByName("M_COVR");
    CHECKNULL(layer, "Cannot open DSID layer");

    for (auto &feat : layer)
    {
        // "Category of Coverage" (CATCOV) may describe "Included" (1) or
        // "Excluded (2) data. Filter on included only ...
        int idx = feat->GetFieldIndex("CATCOV");
        if (idx == -1) { throw std::runtime_error("Cannot find CATCOV field index"); }
        if (feat->GetFieldAsInteger(idx) != 1)
            continue;

        // Get coverage for this feature
        OGRGeometry *geo = feat->GetGeometryRef();
        CHECKNULL(layer, "Cannot get feature geometry");

        // There's probably only one coverage feature,
        // but in case there's not merge each one
        OGREnvelope covr;
        geo->getEnvelope(&covr);
        bbox_.Merge(covr);
    }

    printf(" - MinX: %f\n", bbox_.MinX);
    printf(" - MaxX: %f\n", bbox_.MaxX);
    printf(" - MinY: %f\n", bbox_.MinY);
    printf(" - MaxY: %f\n", bbox_.MaxY);

    // Cleanup
    delete ds;
}
    
}; // ~namespace encviz
