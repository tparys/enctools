#pragma once

/**
 * \file
 * \brief Common Definitions
 *
 * Common structure and type definitions used throughout the code.
 */

#include <cstdint>

namespace encviz
{

/// Tile coordinate system
enum tile_coords
{
    WTMS, ///< 0,0 at northwest
    XYZ,  ///< 0,0 at southwest
};

/// Generalized 2D coordinate
struct coord
{
    /// Horizontal coordinate (Longitude, column, etc..)
    double x;

    /// Vertical coordinate (Latitude, row, etc..)
    double y;
};

}; // ~namespace encviz
