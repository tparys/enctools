#include <vector>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Constrained_Delaunay_triangulation_2.h>
#include <CGAL/Triangulation_vertex_base_with_info_2.h>
#include <CGAL/draw_constrained_triangulation_2.h>
#include <enctri/triangulator.h>

namespace enctri
{

// CGAL uses intense levels of C++ templating
typedef double                                              Z;
typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
typedef CGAL::Triangulation_vertex_base_with_info_2<Z, K>   Vb;
typedef CGAL::Constrained_triangulation_face_base_2<K>      Fb;
typedef CGAL::Triangulation_data_structure_2<Vb, Fb>        Tds;
typedef CGAL::Constrained_Delaunay_triangulation_2<K, Tds>  Cdt;

#define CDT ((Cdt*)ptr_)

/// Constructor
triangulator::triangulator()
{
    ptr_ = new Cdt();
}

/// Destructor
triangulator::~triangulator()
{
    delete CDT;
}

/// Draw triangulation via Qt
void triangulator::draw()
{
    CGAL::draw(*CDT);
}

/// Insert single point
void triangulator::add_point(const encdata::point_3d &p)
{
    // Add 2D point
    Cdt::Point dp(p.x, p.y);

    // Vertex handle contains Z data
    Cdt::Vertex_handle handle = CDT->insert(dp);
    handle->info() = p.z;
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
    for (const encdata::point_2d &p : points)
    {
        Cdt::Point dp(p.x, p.y);
        Cdt::Vertex_handle handle = CDT->insert(dp);
        handle->info() = z;
        vh.push_back(handle);
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
