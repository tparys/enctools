#pragma once
#include <string>
#include <encviz/common.h>

struct enc_metadata
{
    /// Short name for chart (basename)
    std::string base_name;

    /// Full path to chart
    std::string full_path;

    /// Compilation of scale
    double scale;

    /// Bounding box (deg)
    bound_box bbox_deg_;
};
