#include <iostream>
#include <fstream>
#include <algorithm>
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
    // Cache location
    char *phome = getenv("HOME");
    if (phome != nullptr)
    {
        cache_ = phome;
        cache_ += "/.encviz";
    }
    printf("Cache location is %s\n", cache_.c_str());
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

    printf("%lu charts loaded\n", charts_.size());
}

/**
 * Load Single ENC Chart
 *
 * \param[in] path Path to ENC chart
 */
bool enc_dataset::load_chart(const std::filesystem::path &path)
{
    return load_chart_cache(path) || load_chart_disk(path);
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
    std::vector<const metadata*> selected;
    for (const auto &[name, chart] : charts_)
    {
        if ((scale_min <= chart.scale) && bbox.Intersects(chart.bbox))
        {
            selected.push_back(&chart);
        }
    }

    // Sort in ascending scale order (most detailed first)
    std::sort(selected.begin(), selected.end(),
              [](const metadata* a, const metadata* b) {
                  return a->scale < b->scale;});

    // Dump what we have to screen
    printf("Selected %lu/%lu charts:\n", selected.size(), charts_.size());
    for (const auto &chart : selected)
    {
        printf(" - (%d) %s\n", chart->scale, chart->path.c_str());
    }
}

/**
 * Save Single ENC Chart To Cache
 *
 * \param[in] path Path to ENC chart
 * \return False on failure
 */
bool enc_dataset::save_chart_cache(const metadata &meta)
{
    // Ensure cache directory exists
    if (!std::filesystem::exists(cache_) &&
        !std::filesystem::create_directories(cache_))
    {
        return false;
    }

    // Save to /path/to/cache/NAME ...
    std::filesystem::path cached_path = cache_ / meta.path.stem();
    std::ofstream handle(cached_path.string().c_str());
    if (handle.good())
    {
        handle << meta.path << "\n"
               << meta.scale << "\n"
               << meta.bbox.MinX << "\n"
               << meta.bbox.MaxX << "\n"
               << meta.bbox.MinY << "\n"
               << meta.bbox.MaxY << "\n";
    }

    return handle.good();
}

/**
 * Load Single ENC Chart from Cache
 *
 * \param[in] path Path to ENC chart
 * \return False on failure
 */
bool enc_dataset::load_chart_cache(const std::filesystem::path &path)
{
    // Look for /path/to/cache/NAME ...
    std::filesystem::path cached_path = cache_ / path.stem();
    if (std::filesystem::exists(cached_path))
    {
        // Try and read it
        std::ifstream handle(cached_path.string().c_str());
        if (handle.good())
        {
            metadata next;
            handle >> next.path
                   >> next.scale
                   >> next.bbox.MinX
                   >> next.bbox.MaxX
                   >> next.bbox.MinY
                   >> next.bbox.MaxY;

            // Ensure no EOF, and path matches before saving metadata
            if (handle.good() && (path == next.path))
            {
                charts_[path.stem().string()] = next;
                return true;
            }
        }
    }

    // Failed to load
    return false;
}

/**
 * Load Single ENC Chart from Disk
 *
 * \param[in] path Path to ENC chart
 * \return False on failure
 */
bool enc_dataset::load_chart_disk(const std::filesystem::path &path)
{
    metadata next = {path};

    // Open dataset
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
    }

    // Cleanup
    delete ds;

    // Save
    charts_[path.stem().string()] = next;
    save_chart_cache(next);

    return true;
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
