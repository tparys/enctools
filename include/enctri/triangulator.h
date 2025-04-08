#pragma once

#include <encdata/geometry.h>

namespace enctri
{

class triangulator
{
public:

    /**
     * Constructor
     */
    triangulator();

    /**
     * Destructor
     */
    ~triangulator();

    /**
     * Draw triangulation (Qt)
     */
    void draw();

    /**
     * Insert single 3D point
     *
     * \param[in] p Specified point
     */
    void add_point(const encdata::point_3d &p);

    /**
     * Insert 2D path at shared height
     *
     * \param[in] path Specified points
     * \param[in] z Specified height
     */
    void add_path(const encdata::path_2d &path, double z);

    /**
     * Insert 2D polygon at shared height
     *
     * \param[in] poly Specified polygon
     * \param[in] z Specified height
     */
    void add_poly(const encdata::path_2d &poly, double z);

private:

    /**
     * Insert 2D points along constrained path
     *
     * \param[in] points Specified points
     * \param[in] z Specified height
     * \param[in] close Insert constraint from last to first point
     */
    void add_constrained(const encdata::path_2d &points, double z, bool close);

    /// Internal pointer to Constrained Delaunay triangulation
    void *ptr_;

};

}; // ~namespace enctri
