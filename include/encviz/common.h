#pragma once

/// Generalized 2D coordinate
struct coord
{
    /// Horizontal coordinate (Longitude, column, etc..)
    double x;

    /// Vertical coordinate (Latitude, row, etc..)
    double y;
};

/// Simple bounding box
struct bound_box
{
    /// Min coordinates
    coord min;

    /// Max coordinates
    coord max;
};
