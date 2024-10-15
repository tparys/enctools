#include <cmath>
#include <encviz/web_mercator.h>

/**
 * Constructor
 *
 * \param[in] x Tile x coordinate (TMS)
 * \param[in] y Tile y coordinate (TMS)
 * \param[in] y Tile z coordinate (TMS)
 * \param[in] tile_size Tile size in pixels
 */
web_mercator::web_mercator(std::size_t x, std::size_t y, std::size_t z, int tile_size)
{
    // Nominal planet radius
    double radius = 6378137;

    // Nominal dimentions in meters of Web Mercator map
    double tile_side = 2 * M_PI * radius;

    // Meter coordinates from bottom left, not center
    offset_m_ = tile_side / 2;

    // Number of tiles at this zoom level
    std::size_t ntiles = (std::size_t)pow(2, z);

    // Update side length, resolution
    tile_side /= ntiles;

    // Compute bounding box (meters)
    bbox_m_.min.x = x * tile_side - offset_m_;
    bbox_m_.min.y = y * tile_side - offset_m_;
    bbox_m_.max.x = bbox_m_.min.x + tile_side;
    bbox_m_.max.y = bbox_m_.min.y + tile_side;
}

/**
 * Get bounding box for requested tile
 *
 * \return Computed coordinates
 */
bound_box web_mercator::get_bbox_meters() const
{
    return bbox_m_;
}

/**
 * Get bounding box in degrees
 *
 * \return Computed coordinates
 */
bound_box web_mercator::get_bbox_deg() const
{
    bound_box bbox_deg;
    bbox_deg.min = meters_to_deg(bbox_m_.min);
    bbox_deg.max = meters_to_deg(bbox_m_.max);
    return bbox_deg;
}

/**
 * Convert coordinate from meters to degrees
 *
 * \param[in] in Input coordinate (meters)
 * \return Output coordinate (degrees)
 */
coord web_mercator::meters_to_deg(const coord &in) const
{
    coord out = {
	(in.x / offset_m_) * 180,
	(in.y / offset_m_) * 180
    };
    out.y = 180 / M_PI * (2 * atan(exp(out.y * M_PI / 180)) - M_PI / 2);
    return out;
}

/**
 * Convert coordinate from meters to degrees
 *
 * \param[in] in Input coordinate (degrees)
 * \return Output coordinate (meters)
 */
coord web_mercator::deg_to_meters(const coord &in) const
{
    coord out = {
	in.x * offset_m_ / 180.0,
        log(tan((90 + in.y) * M_PI / 360.0)) / (M_PI / 180.0)
    };
    out.y *= offset_m_ / 180.0;
    return out;
}
