#pragma once

#include <array>
#include <vector>
#include <encdata/geometry.h>

namespace enctri
{

class triangulator
{
public:

    // Triangle face by vertex index
    typedef std::array<size_t, 3> face;

    /// Output data structure
    struct mesh
    {
        std::vector<encdata::point_3d> points;
        std::vector<face> faces;
    };

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
     * Get triangulation
     *
     * \return Internal triangulation
     */
    mesh get();

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
