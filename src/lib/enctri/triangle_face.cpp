/**
 * \brief Surface Mesh Triangle Face
 *
 * Operations relating to locating points on a 2D triangle mesh with height data.
 */

// FIXME
#include <cstdio>

#include <cmath>
#include <enctri/triangle_face.h>

namespace enctri
{

/**
 * Constructor
 *
 * \param[in] v Triangle vertices
 */
triangle_face::triangle_face(const std::array<encdata::point_3d, 3> &v)
{
    // Copy 2D verticies
    for (size_t i = 0; i < 3; i++)
    {
        v_[i].x = v[i].x;
        v_[i].y = v[i].y;
    }

    // Compute bounding box
    bbox_.min.x = std::min(std::min(v_[0].x, v_[1].x), v_[2].x);
    bbox_.min.y = std::min(std::min(v_[0].y, v_[1].y), v_[2].y);
    bbox_.max.x = std::max(std::max(v_[0].x, v_[1].x), v_[2].x);
    bbox_.max.y = std::max(std::max(v_[0].y, v_[1].y), v_[2].y);

    // Compute centroid
    centroid_.x = (v_[0].x + v_[1].x + v_[2].x) / 3;
    centroid_.y = (v_[0].y + v_[1].y + v_[2].y) / 3;

    // Compute vectors v12, v13
    encdata::point_3d v12 = {
        v[1].x - v[0].x,
        v[1].y - v[0].y,
        v[1].z - v[0].z,
    };
    encdata::point_3d v13 = {
        v[2].x - v[0].x,
        v[2].y - v[0].y,
        v[2].z - v[0].z,
    };

    // Compute cross product of v12 x v13
    encdata::point_3d cross = {
        (v12.y * v13.z) - (v12.z * v13.y),
        (v12.z * v13.x) - (v12.x * v13.z),
        (v12.x * v13.y) - (v12.y * v13.x),
    };

    // Normalize cross produce to get most of plane parameters
    double mag = sqrt((cross.x * cross.x) +
                      (cross.y * cross.y) +
                      (cross.z * cross.z));
    plane_[0] = cross.x / mag;
    plane_[1] = cross.y / mag;
    plane_[2] = cross.z / mag;

    // Use first point to determine offset D
    plane_[3] = - plane_[0] * v[0].x - plane_[1] * v[0].y - plane_[2] * v[0].z;
}

/**
 * Query 2D Bounding Box
 *
 * \return Computed centroid
 */
const encdata::bbox_2d &triangle_face::get_bbox() const
{
    return bbox_;
}

/**
 * Query 2D Centroid
 *
 * \return Computed centroid
 */
const encdata::point_2d &triangle_face::get_centroid() const
{
    return centroid_;
}

/**
 * Query if Point Inside Triangle
 *
 * \param[in] p Query point
 * \return True if within specified triangle
 */
bool triangle_face::contains(const encdata::point_2d &p) const
{
    double d1 = sign(p, v_[0], v_[1]);
    double d2 = sign(p, v_[1], v_[2]);
    double d3 = sign(p, v_[2], v_[0]);

    bool has_neg = (d1 < 0) || (d2 < 0) || (d3 < 0);
    bool has_pos = (d1 > 0) || (d2 > 0) || (d3 > 0);

    return !(has_neg && has_pos);
}

/**
 * Query Height on 3D Plane
 *
 * \param[in] p Query point
 * \return Plane Z value at point
 */
double triangle_face::height(const encdata::point_2d &p) const
{
    // Solve "Ax + By + Cz + D = 0" for "z"
    return -(plane_[3] + (plane_[0] * p.x) + (plane_[1] * p.y)) / plane_[2];
}

/**
 * Determine Halfplane Side
 *
 * \param[in] p1 First point
 * \param[in] p2 Second point 
 * \param[in] p2 third point
 * \return Postive if on one side, negative otherwise
 */
double triangle_face::sign(const encdata::point_2d &p1,
                           const encdata::point_2d &p2,
                           const encdata::point_2d &p3) const
{
    return (p1.x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p3.y);
}

}; // ~namespace enctri
