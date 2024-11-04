#include <encviz/enc_renderer.h>

namespace encviz
{

/**
 * Cairo Stream Callback
 *
 * \param[out] closure Pointer to std::vector<uint8_t> output stream
 * \param[in] data Data buffer
 * \param[in] length Length of data buffer
 */
static cairo_status_t cairo_write_to_vector(void *closure,
                                            const unsigned char *data,
                                            unsigned int length)
{
    // TODO - Can probably make this better
    std::vector<uint8_t> *output = (std::vector<uint8_t>*)closure;
    output->reserve(output->size() + length);
    for (unsigned int i = 0; i < length; i++)
    {
        output->push_back(data[i]);
    }
    return CAIRO_STATUS_SUCCESS;
}

/**
 * Constructor
 *
 * \param[in] tile_size Dimension of output image
 * \param[in] min_scale0 Min display scale at zoom=0
 */
enc_renderer::enc_renderer(int tile_size, double min_scale0)
    : tile_size_(tile_size)
    , min_scale0_(min_scale0)
{
}

/**
 * Recursively Load ENC Charts
 *
 * \param[in] enc_root ENC_ROOT base directory
 */
void enc_renderer::load_charts(const std::string &enc_root)
{
    enc_.load_charts(enc_root);
}

/**
 * Render Chart Data
 *
 * \param[out] data PNG bytestream
 * \param[in] x Tile X coordinate (from left)
 * \param[in] y Tile Y coordinate (from bottom)
 * \param[in] z Tile zoom (power of 2)
 * \return False if no data to render
 */
bool enc_renderer::render(std::vector<uint8_t> &data, int x, int y, int z,
                          const render_style &style)
{
    // Collect the layers we need
    std::vector<std::string> layers;
    for (const layer_style &lstyle : style.layers)
    {
        layers.push_back(lstyle.layer_name);
    }

    // Get base tile boundaries
    encviz::web_mercator wm(x, y, z, tile_size_);
    OGREnvelope bbox = wm.get_bbox_deg();

    // Oversample a bit so not clip text between tiles
    {
        double oversample = 0.1;
        double width = bbox.MaxX - bbox.MinX;
        double height = bbox.MaxY - bbox.MinY;
        bbox.MinX -= oversample * (width/2);
        bbox.MaxX += oversample * (width/2);
        bbox.MinY -= oversample * (height/2);
        bbox.MaxY += oversample * (height/2);
    }

    // Export all data in this tile
    GDALDataset *tile_data = GetGDALDriverManager()->GetDriverByName("Memory")->
        Create("", 0, 0, 0, GDT_Unknown, nullptr);
    int scale_min = (int)round(min_scale0_ / pow(2, z));
    enc_.export_data(tile_data, layers, bbox, scale_min);

    // Create a cairo surface
    cairo_surface_t *surface =
        cairo_image_surface_create(CAIRO_FORMAT_ARGB32, tile_size_, tile_size_);
    cairo_t *cr = cairo_create(surface);
                                                          
    // Flood background w/ white 0xffffff
    if (style.background.has_value())
    {
        set_color(cr, style.background.value());
        cairo_paint(cr);
    }

    // Render style layers
    for (const auto &lstyle : style.layers)
    {
        // Render feature geometry in this layer
        OGRLayer *tile_layer = tile_data->GetLayerByName(lstyle.layer_name.c_str());
        for (const auto &feat : tile_layer)
        {
            OGRGeometry *geo = feat->GetGeometryRef();
            render_geo(cr, geo, wm, lstyle);
        }
    }

    // Write out image
    data.clear();
    cairo_status_t rc =
        cairo_surface_write_to_png_stream(surface,
                                          cairo_write_to_vector,
                                          &data);
    if (rc)
    {
        printf("Cairo write error %d : %s\n", rc,
               cairo_status_to_string(rc));
    }

    // Cleanup
    cairo_destroy(cr);
    cairo_surface_destroy(surface);
    GDALClose(tile_data);

    return true;
}

/**
 * Render Feature Geometry
 *
 * \param[out] cr Image context
 * \param[in] geo Feature geometry
 * \param[in] wm Web Mercator point mapper
 * \param[in] style Feature style
 */
void enc_renderer::render_geo(cairo_t *cr, const OGRGeometry *geo,
                              const web_mercator &wm, const layer_style &style)
{
    // What sort of geometry were we passed?
    OGRwkbGeometryType gtype = geo->getGeometryType();
    switch (gtype)
    {
        case wkbPoint: // 1
            render_point(cr, geo->toPoint(), wm, style);
            break;

        case wkbMultiPoint: // 4
            for (const OGRPoint *child : geo->toMultiPoint())
            {
                render_point(cr, child, wm, style);
            }
            break;

        case wkbPoint25D: // 0x80000001
            // TODO - SOUNDG only?
            render_depth(cr, geo->toPoint(), wm, style);
            break;

        case wkbMultiPoint25D: // 0x80000004
            // TODO - SOUNDG only?
            for (const OGRPoint *child : geo->toMultiPoint())
            {
                render_depth(cr, child, wm, style);
            }
            break;

        case wkbLineString: // 2
            render_line(cr, geo->toLineString(), wm, style);
            break;

        case wkbMultiLineString: // 5
            for (const OGRGeometry *child : geo->toMultiLineString())
            {
                render_geo(cr, child, wm, style);
            }
            break;

        case wkbPolygon: // 6
            render_poly(cr, geo->toPolygon(), wm, style);
            break;

        case wkbMultiPolygon: // 10
            for (const OGRPolygon *child : geo->toMultiPolygon())
            {
                render_poly(cr, child, wm, style);
            }
            break;

        case wkbGeometryCollection: // 7
            for (const OGRGeometry *child : geo->toGeometryCollection())
            {
                render_geo(cr, child, wm, style);
            }
            break;

        default:
            throw std::runtime_error("Unhandled geometry of type " +
                                     std::to_string(gtype));
    }
}

/**
 * Render Depth Value
 *
 * \param[out] cr Image context
 * \param[in] geo Feature geometry
 * \param[in] wm Web Mercator point mapper
 * \param[in] style Feature style
 */
void enc_renderer::render_depth(cairo_t *cr, const OGRPoint *geo,
                                const web_mercator &wm, const layer_style &style)
{
    // Convert lat/lon to pixel coordinates
    coord c = wm.point_to_pixels(*geo);

    // TODO - Could do this better?
    char text[64] = {};
    snprintf(text, sizeof(text)-1, "%.1f", geo->getZ());

    // Determine text render size
    cairo_text_extents_t extents = {};
    cairo_text_extents(cr, text, &extents);

    // Draw text
    set_color(cr, style.line_color);
    cairo_select_font_face(cr, "monospace",
                           CAIRO_FONT_SLANT_NORMAL,
                           CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size(cr, 15);
    cairo_move_to(cr, c.x - extents.width/2, c.y - extents.height/2);
    cairo_show_text(cr, text);
}

/**
 * Render Point Geometry
 *
 * \param[out] cr Image context
 * \param[in] geo Feature geometry
 * \param[in] wm Web Mercator point mapper
 * \param[in] style Feature style
 */
void enc_renderer::render_point(cairo_t *cr, const OGRPoint *geo,
                                const web_mercator &wm, const layer_style &style)
{
    // Skip render if not appropriate
    if (style.marker_size == 0)
    {
        return;
    }

    // Convert lat/lon to pixel coordinates
    coord c = wm.point_to_pixels(*geo);

    // Draw circle
    cairo_arc(cr, c.x, c.y, style.marker_size, 0, 2 * M_PI);

    // Draw line and fill
    set_color(cr, style.fill_color);
    cairo_fill_preserve(cr);
    set_color(cr, style.line_color);
    cairo_set_line_width(cr, style.line_width);
    cairo_stroke(cr);
}

/**
 * Render LineString Geometry
 *
 * \param[out] cr Image context
 * \param[in] geo Feature geometry
 * \param[in] wm Web Mercator point mapper
 * \param[in] style Feature style
 */
void enc_renderer::render_line(cairo_t *cr, const OGRLineString *geo,
                               const web_mercator &wm, const layer_style &style)
{
    // Pass OGR points to cairo
    bool first = true;
    for (auto &point : geo)
    {
        // Convert lat/lon to pixel coordinates
        coord c = wm.point_to_pixels(point);

        // Mark first point as pen-down
        if (first)
        {
            cairo_move_to(cr, c.x, c.y);
            first = false;
        }
        else
        {
            cairo_line_to(cr, c.x, c.y);
        }
    }

    // Draw line
    set_color(cr, style.line_color);
    cairo_set_line_width(cr, style.line_width);
    cairo_stroke(cr);
}

/**
 * Render Polygon Geometry
 *
 * \param[out] cr Image context
 * \param[in] geo Feature geometry
 * \param[in] wm Web Mercator point mapper
 * \param[in] style Feature style
 */
void enc_renderer::render_poly(cairo_t *cr, const OGRPolygon *geo,
                               const web_mercator &wm, const layer_style &style)
{
    // FIXME - Throw a fit if we see interior rings (not handled)
    if (geo->getNumInteriorRings() != 0)
    {
        //throw std::runtime_error("Unhandled polygon with interior rings");
    }

    // Pass OGR points to cairo
    bool first = true;
    for (auto &point : geo->getExteriorRing())
    {
        // Convert lat/lon to pixel coordinates
        coord c = wm.point_to_pixels(point);

        // Mark first point as pen-down
        if (first)
        {
            cairo_move_to(cr, c.x, c.y);
            first = false;
        }
        else
        {
            cairo_line_to(cr, c.x, c.y);
        }
    }

    // Draw line and fill
    set_color(cr, style.fill_color);
    cairo_fill_preserve(cr);
    set_color(cr, style.line_color);
    cairo_set_line_width(cr, style.line_width);
    cairo_stroke(cr);
}

/**
 * Set Render Color
 *
 * \param[out] cr Image context
 * \param[in] c RGB color
 */
void enc_renderer::set_color(cairo_t *cr, const color &c)
{
    cairo_set_source_rgb(cr,
                         float(c.red) / 0xff,
                         float(c.blue) / 0xff,
                         float(c.green) / 0xff);
}

}; // ~namespace encviz
