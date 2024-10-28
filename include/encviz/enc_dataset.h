#pragma once
#include <string>
#include <vector>
#include <filesystem>
#include <ogrsf_frmts.h>

namespace encviz
{

class enc_dataset
{
public:

    /// Per chart metadata
    struct metadata
    {
        /// Path to data file
        std::filesystem::path path;

        /// Compilation of scale (DSPM CSCL)
        int scale;

        /// Bounding box (deg)
        OGREnvelope bbox;
    };

    /**
     * Constructor
     */
    enc_dataset();

    /**
     * Clear Chart Index
     */
    void clear();

    /**
     * Recursively Load ENC Charts
     *
     * \param[in] enc_root ENC_ROOT base directory
     */
    void load_charts(const std::string &enc_root);

    /**
     * Load Single ENC Chart
     *
     * \param[in] enc_roots ENC_ROOT base directories
     */
    void load_chart(const std::filesystem::path &path);

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
    void export_data(GDALDataset *ods, std::vector<std::string> layers,
                     OGREnvelope bbox, int scale_min);

private:

    /**
     * Get OGR Integer Field
     *
     * \param[in] feat OGR feature
     * \param[in] name Field name
     * \return Requested value
     */
    int get_feat_field_int(OGRFeature *feat, const char *name);

    /// Loaded chart metadata
    std::vector<metadata> charts_;
};

}; // ~namespace encviz
