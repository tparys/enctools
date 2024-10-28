#include <filesystem>
#include <encviz/enc_dataset.h>

// Helper macro for data presence
#define CHECKNULL(ptr, msg) if ((ptr) == nullptr) throw std::runtime_error((msg))

namespace encviz
{

/**
 * Constructor
 */
enc_dataset::enc_dataset()
{
}

/**
 * Clear Chart Index
 */
void enc_dataset::clear()
{
    charts_.clear();
}

/**
 * Recursively Load ENC Charts
 *
 * \param[in] enc_root ENC_ROOT base directory
 */
void enc_dataset::load_charts(const std::string &enc_root)
{
    auto rdi = std::filesystem::recursive_directory_iterator(enc_root);
    for (const std::filesystem::directory_entry &entry : rdi)
    {
        if (entry.path().extension() == ".000")
        {
           load_chart(entry.path());
        }
    }
}

/**
 * Load Single ENC Chart
 *
 * \param[in] enc_roots ENC_ROOT base directories
 */
void enc_dataset::load_chart(const std::filesystem::path &path)
{
    metadata next = {path};

    // Open dataset
    printf("Index: %s\n", path.string().c_str());
    const char *const drivers[] = { "S57", nullptr };
    GDALDataset *ds = GDALDataset::Open(path.string().c_str(),
                                        GDAL_OF_VECTOR | GDAL_OF_READONLY,
                                        drivers, nullptr, nullptr);
    CHECKNULL(ds, "Cannot open OGR dataset");

    // Get Compilation Scale of Chart
    {
        // Get "Dataset ID" (DSID) chart metadata
        OGRLayer *layer = ds->GetLayerByName("DSID");
        CHECKNULL(layer, "Cannot open DSID layer");

        // .. which contains one feature
        layer->ResetReading();
        std::unique_ptr<OGRFeature> feat(layer->GetNextFeature());
        CHECKNULL(feat, "Cannot read DSID feature");

        // .. that has "Dataset Parameter" (DSPM) "Compilation of Scale" (CSCL)
        next.scale = get_feat_field_int(feat.get(), "DSPM_CSCL");
        printf(" - Scale: 1:%d\n", next.scale);
    }

    // Get Chart Coverage Bounds
    {
        // Get "Coverage" (M_COVR) data
        OGRLayer *layer = ds->GetLayerByName("M_COVR");
        CHECKNULL(layer, "Cannot open M_COVR layer");

        // There's probably only one coverage feature,
        // but just in case, combine any we find
        for (auto &feat : layer)
        {
            // "Category of Coverage" (CATCOV) may be:
            //  - "coverage available" (1)
            //  - "no coverage available" (2)
            if (get_feat_field_int(feat.get(), "CATCOV") != 1)
                continue;

            // Get coverage for this feature
            OGRGeometry *geo = feat->GetGeometryRef();
            CHECKNULL(layer, "Cannot get feature geometry");

            // There's probably only one coverage feature,
            // but in case there's not merge each one
            OGREnvelope covr;
            geo->getEnvelope(&covr);
            next.bbox.Merge(covr);
        }

        printf(" - BBox: (%g to %g), (%g to %g)\n",
               next.bbox.MinX, next.bbox.MaxX,
               next.bbox.MinY, next.bbox.MaxY);
    }

    // Cleanup and save
    delete ds;
    charts_.push_back(next);
}

/**
 * Export ENC Data to Empty Dataset
 *
 * Creates specified layers in output dataset, populating with best data
 * available for given bounding box and minimum presentation scale.
 *
 * \param[out] ds Output dataset
 * \param[in] layers Specified ENC layers (S57)
 * \param[in] bbox Data bounding box (deg)
 * \param[in] scale_min Minimum data compilation scale
 */
void enc_dataset::export_data(GDALDataset *ods, std::vector<std::string> layers,
                              OGREnvelope bbox, int scale_min)
{
    printf("Filter: Scale=%d, BBOX=(%g to %g),(%g to %g)\n",
           scale_min, bbox.MinX, bbox.MaxX, bbox.MinY, bbox.MaxY);

    // Build list of suitable charts
    std::vector<std::string> selected;
    for (const auto &chart : charts_)
    {
        if ((scale_min <= chart.scale) && bbox.Intersects(chart.bbox))
        {
            selected.push_back(chart.path.string());
        }
    }

    printf("Selected %lu/%lu charts:\n", selected.size(), charts_.size());
    for (const auto &path : selected)
    {
        printf(" - %s\n", path.c_str());
    }
}

/**
 * Get OGR Integer Field
 *
 * \param[in] feat OGR feature
 * \param[in] name Field name
 * \return Requested value
 */
int enc_dataset::get_feat_field_int(OGRFeature *feat, const char *name)
{
    // See if field even exists
    int idx = feat->GetFieldIndex(name);
    if (idx == -1)
    {
        std::ostringstream oss;
        oss << "Feature does not have field \"" << name << "\"";
        throw std::runtime_error(oss.str());
    }

    // Check that the field is really an integer
    OGRFieldDefn *defn = feat->GetFieldDefnRef(idx);
    CHECKNULL(defn, "Cannot get feature field definition");
    if (defn->GetType() != OGRFieldType::OFTInteger)
    {
        std::ostringstream oss;
        oss << "Feature field \"" << name << "\" is not an integer";
        throw std::runtime_error(oss.str());
    }

    // Safe to call now
    return feat->GetFieldAsInteger(idx);
}


}; // ~namespace encviz
