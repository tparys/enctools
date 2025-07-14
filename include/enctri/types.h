#pragma once

#include <array>
#include <vector>

namespace enctri
{

// Triangle face by vertex index
typedef std::array<size_t, 3> face;

// 3D point mesh
struct mesh
{
    std::vector<encdata::point_3d> points;
    std::vector<face> faces;
};

}; // ~namespace enctri
