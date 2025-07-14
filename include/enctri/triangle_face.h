#pragma once

/**
 * \brief Surface Mesh Triangle Face
 *
 * Operations relating to locating points on a 2D triangle mesh with height data.
 */

#include <array>
#include <encdata/geometry.h>

namespace enctri
{

class triangle_face
{
public:

    /**
     * Constructor
     *
     * \param[in] v Triangle vertices
     */
    triangle_face(const std::array<encdata::point_3d, 3> &v);

    /**
     * Query 2D Bounding Box
     *
     * \return Computed centroid
     */
    const encdata::bbox_2d &get_bbox() const;

    /**
     * Query 2D Centroid
     *
     * \return Computed centroid
     */
    const encdata::point_2d &get_centroid() const;

    /**
     * Query if Point Inside Triangle
     *
     * \param[in] p Query point
     * \return True if within specified triangle
     */
    bool contains(const encdata::point_2d &p) const;

    /**
     * Query Height on 3D Plane
     *
     * \param[in] p Query point
     * \return Plane Z value at point
     */
    double height(const encdata::point_2d &p) const;

private:

    /**
     * Determine Halfplane Side
     *
     * \param[in] p1 First point
     * \param[in] p2 Second point 
     * \param[in] p2 third point
     * \return Postive if on one side, negative otherwise
     */
    double sign(const encdata::point_2d &p1,
                const encdata::point_2d &p2,
                const encdata::point_2d &p3) const;

    /// 2D vertices
    std::array<encdata::point_2d, 3> v_;

    /// 2D bounding box
    encdata::bbox_2d bbox_;

    /// 2D centroid
    encdata::point_2d centroid_;

    /// 3D plane coordinates in form of Ax + By + Cz + D = 0
    std::array<double, 4> plane_;
};

}; // ~namespace enctri
