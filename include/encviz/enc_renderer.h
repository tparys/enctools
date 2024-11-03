#pragma once
#include <cstdint>
#include <string>
#include <cairo.h>
#include <encviz/enc_dataset.h>
#include <encviz/web_mercator.h>

namespace encviz
{

class enc_renderer
{
public:

    /**
     * Constructor
     *
     * \param[in] tile_size Dimension of output image
     * \param[in] min_scale0 Min display scale at zoom=0
     */
    enc_renderer(int tile_size, double min_scale0);

    /**
     * Recursively Load ENC Charts
     *
     * \param[in] enc_root ENC_ROOT base directory
     */
    void load_charts(const std::string &enc_root);

    /**
     * Render Chart Data
     *
     * \param[in] x Tile X coordinate (from left)
     * \param[in] y Tile Y coordinate (from bottom)
     * \param[in] z Tile zoom (power of 2)
     * \param[in] style Tile styling data
     * \return False if no data to render
     */
    bool render(int x, int y, int z, const render_style &style);

private:

    /**
     * Render Feature Geometry
     *
     * \param[out] cr Image context
     * \param[in] geo Feature geometry
     * \param[in] wm Web Mercator point mapper
     * \param[in] style Feature style
     */
    void render_geo(cairo_t *cr, const OGRGeometry *geo,
                    const web_mercator &wm, const layer_style &style);

    /**
     * Render LineString Geometry
     *
     * \param[out] cr Image context
     * \param[in] geo Feature geometry
     * \param[in] wm Web Mercator point mapper
     * \param[in] style Feature style
     */
    void render_line(cairo_t *cr, const OGRLineString *geo,
                     const web_mercator &wm, const layer_style &style);

    /**
     * Render Polygon Geometry
     *
     * \param[out] cr Image context
     * \param[in] geo Feature geometry
     * \param[in] wm Web Mercator point mapper
     * \param[in] style Feature style
     */
    void render_poly(cairo_t *cr, const OGRPolygon *geo,
                     const web_mercator &wm, const layer_style &style);

    /**
     * Set Render Color
     *
     * \param[out] cr Image context
     * \param[in] c RGB color
     */
    void set_color(cairo_t *cr, const color &c);

    /// Dimension of output image
    int tile_size_;

    /// Min display scale at zoom=0
    double min_scale0_;

    /// Chart collection
    enc_dataset enc_;
};

}; // ~namespace encviz
