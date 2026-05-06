#include <vector>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Constrained_Delaunay_triangulation_2.h>
#include <CGAL/Triangulation_vertex_base_with_info_2.h>
#ifndef CGAL_NO_DRAW_CDT
  #include <CGAL/draw_constrained_triangulation_2.h>
#endif
#include <enctri/triangle_face.h>
#include <enctri/triangulator.h>

namespace enctri
{

// Extra data we're tracking per vertex
struct VertexInfo
{
    /// Height data
    double z;

    /// Vertex ID
    size_t id;
};

// 2D Constrained Delaunay Triangulation w/ Extra Vertex Info
typedef CGAL::Exact_predicates_inexact_constructions_kernel        K;
typedef CGAL::Triangulation_vertex_base_with_info_2<VertexInfo, K> Vb;
typedef CGAL::Constrained_triangulation_face_base_2<K>             Fb;
typedef CGAL::Triangulation_data_structure_2<Vb, Fb>               Tds;
typedef CGAL::Constrained_Delaunay_triangulation_2<K, Tds>         Cdt;

#define CDT ((Cdt*)ptr_)

/// Not a class member so header doesn't include CGAL bits ...
static Cdt::Vertex_handle insert_point3d(Cdt *cdt, const encdata::point_3d &p)
{
    // Create 2D point in CGAL kernel
    Cdt::Point kp(p.x, p.y);

    // Add point to triangulation
    Cdt::Vertex_handle handle = cdt->insert(kp);

    // Store Z value
    handle->info().z = p.z;

    // NOTE: not assigning vertex ID here as this point may be an existing
    // vertex, and there doesn't seem to be a good way to handle that ...

    return handle;
}

/**
 * Constructor
 */
triangulator::triangulator()
{
    ptr_ = new Cdt();
}

/**
 * Destructor
 */
triangulator::~triangulator()
{
    delete CDT;
}

/**
 * Draw triangulation via Qt
 */
void triangulator::draw()
{
#ifndef CGAL_NO_DRAW_CDT
    CGAL::draw(*CDT);
#else
    throw std::runtime_error("Function not supported");
#endif
}

/**
 * Get triangulation mesh
 *
 * \param[out] points 3D coordinates
 * \param[out] faces Vertex indeces
 */
mesh triangulator::get_mesh()
{
    mesh out;

    // Copy out vertices, renumbering as we go
    size_t count = 0;
    for (const auto &vertex_handle : CDT->finite_vertex_handles())
    {
        encdata::point_3d p;
        p.x = vertex_handle->point().x();
        p.y = vertex_handle->point().y();
        p.z = vertex_handle->info().z;
        vertex_handle->info().id = count++;
        out.points.push_back(p);
    }

    // Copy out faces
    for (const auto &face_handle : CDT->finite_face_handles())
    {
        face next_face = {};

        // No random accessors for points, so need to use iterators
        for (int i = 0; i < 3; i++)
        {
            next_face.at(i) = face_handle->vertex(i)->info().id;
        }

        out.faces.push_back(next_face);
    }

    return out;
}

/**
 * Get triangulation coverage
 *
 * \return Coverage as OGR Envelope
 */
encdata::bbox_2d triangulator::get_coverage() const
{
    encdata::bbox_2d bbox = {{INFINITY, INFINITY}, {-INFINITY, -INFINITY}};
    for (const auto &vertex_handle : CDT->finite_vertex_handles())
    {
        // Next point
        double x = vertex_handle->point().x();
        double y = vertex_handle->point().y();

        // Update bounds
        bbox.min.x = std::min(bbox.min.x, x);
        bbox.min.y = std::min(bbox.min.y, y);
        bbox.max.x = std::max(bbox.max.x, x);
        bbox.max.y = std::max(bbox.max.y, y);
    }

    return bbox;
}

/**
 * Rasterize to 2D grid
 *
 * \param[out] data Grid data, row major
 * \param[in] bbox Grid coverage
 * \param[in] res Grid resolution
 * \param[in] nodata Null value for marking no valid data
 */
void triangulator::rasterize(raster &grid, encdata::bbox_2d const &bbox,
                             double res, float nodata)
{
    // Figure out how big dataset needs to be and initialize
    grid.size_x = std::round((bbox.max.x - bbox.min.x) / res);
    grid.size_y = std::round((bbox.max.y - bbox.min.y) / res);
    grid.data.resize(grid.size_x * grid.size_y);
    std::fill(grid.data.begin(), grid.data.end(), nodata);

    // Collect triangulation as 2.5D mesh
    mesh out = get_mesh();

    // Rasterize triangles to 2D dataset
    for (enctri::face const &face : out.faces)
    {
        // Convert triangle points from geo to local coordinates
        std::array<encdata::point_3d, 3> points;
        for (size_t i = 0; i < 3; i++)
        {
            encdata::point_3d const &point_geo = out.points[face[i]];
            points[i].x = (point_geo.x - bbox.min.x) / res;
            points[i].y = (point_geo.y - bbox.min.y) / res;
            points[i].z = point_geo.z;
        }

        // Solve triangle face
        triangle_face tri(points);
        encdata::bbox_2d tri_bbox = tri.get_bbox();
        int min_x = std::max((int)std::floor(tri_bbox.min.x), 0);
        int min_y = std::max((int)std::floor(tri_bbox.min.y), 0);
        int max_x = std::min((int)std::ceil(tri_bbox.max.x), (int)grid.size_x - 1);
        int max_y = std::min((int)std::ceil(tri_bbox.max.y), (int)grid.size_y - 1);

        // Raster triangle
        for (int y = min_y; y <= max_y; y++)
        {
            for (int x = min_x; x <= max_x; x++)
            {
                encdata::point_2d p;
                p.x = x;
                p.y = y;
                if (tri.contains(p))
                {
                    grid.data[(y * grid.size_x) + x] = (float)tri.height(p);
                }
            }
        }
    }
}

/// Insert single point
void triangulator::add_point(const encdata::point_3d &p)
{
    insert_point3d(CDT, p);
}

/**
 * Insert 2D path at shared height
 *
 * \param[in] path Specified points
 * \param[in] z Specified height
 */
void triangulator::add_path(const encdata::path_2d &path, double z)
{
    add_constrained(path, z, false);
}

/**
 * Insert 2D polygon at shared height
 *
 * \param[in] poly Specified polygon
 * \param[in] z Specified height
 */
void triangulator::add_poly(const encdata::path_2d &poly, double z)
{
    add_constrained(poly, z, true);
}

/**
 * Insert 2D points along constrained path
 *
 * \param[in] points Specified points
 * \param[in] z Specified height
 * \param[in] close Insert constraint from last to first point
 */
void triangulator::add_constrained(const encdata::path_2d &points, double z, bool close)
{
    // Add 2D points, store vertex handles
    std::vector<Cdt::Vertex_handle> vh;
    vh.reserve(points.size());
    for (const encdata::point_2d &p : points)
    {
        encdata::point_3d p3 = { p.x, p.y, z };
        vh.push_back(insert_point3d(CDT, p3));
    }

    // Add constraints along path
    for (size_t i = 0; i + 1 < points.size(); i++)
    {
        CDT->insert_constraint(vh[i], vh[i + 1]);
    }

    // Close path to make a polygon?
    if (close && (points.size() > 1))
    {
        CDT->insert_constraint(vh[0], vh[points.size() - 1]);
    }
}

}; // ~namespace enctri
