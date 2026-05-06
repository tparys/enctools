#pragma once

#include <array>
#include <vector>
#include <encdata/geometry.h>
#include <enctri/types.h>

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
     * Get triangulation mesh
     *
     * \return Internal triangulation
     */
    mesh get_mesh();

    /**
     * Get triangulation coverage
     *
     * \return Coverage as OGR Envelope
     */
    encdata::bbox_2d get_coverage() const;

    /**
     * Rasterize to 2D grid
     *
     * \param[out] grid Output raster
     * \param[in] bbox Grid coverage
     * \param[in] res Grid resolution
     * \param[in] nodata Null value for marking no valid data
     */
    void rasterize(raster &grid, encdata::bbox_2d const &bbox,
                   double res, float nodata);

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
