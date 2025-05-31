#include <vector>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Constrained_Delaunay_triangulation_2.h>
#include <CGAL/Triangulation_vertex_base_with_info_2.h>
#ifndef CGAL_NO_DRAW_CDT
  #include <CGAL/draw_constrained_triangulation_2.h>
#endif
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
 * Get triangulation
 *
 * \param[out] points 3D coordinates
 * \param[out] faces Vertex indeces
 */
triangulator::mesh triangulator::get()
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
