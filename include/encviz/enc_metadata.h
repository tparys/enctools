#pragma once
#include <string>
#include <filesystem>
#include <ogr_core.h>
#include <encviz/common.h>

namespace encviz
{

class enc_metadata
{

public:

    /**
     * Constructor
     *
     * param[in] path Path to ENC chart data
     */
    enc_metadata(const std::filesystem::path &path);

    /// Path to data file
    std::filesystem::path path_;

    /// Compilation of scale (DSPM CSCL)
    int scale_;

    /// Bounding box (deg)
    OGREnvelope bbox_;
};

}; // ~namespace encviz
