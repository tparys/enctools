#pragma once

/**
 * Simple Geometry Types
 */

#include <vector>

namespace encdata
{

/// Simple 2D coordinate
struct point_2d
{
    double x;
    double y;
};

/// Simple 2D bounding box
struct bbox_2d
{
    point_2d min;
    point_2d max;
};

/// Simple 3D coordinate (extending 2D)
struct point_3d : public point_2d
{
    double z;
};

/// Simple 2D path
typedef std::vector<point_2d> path_2d;

}; // ~namespace encdata
