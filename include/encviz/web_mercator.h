#pragma once
#include <cstddef>
#include <ogr_core.h>
#include <encviz/common.h>

namespace encviz
{

class web_mercator
{
public:

    /**
     * Constructor
     *
     * \param[in] x Tile x coordinate (TMS)
     * \param[in] y Tile y coordinate (TMS)
     * \param[in] y Tile z coordinate (TMS)
     */
    web_mercator(std::size_t x, std::size_t y, std::size_t z, int tile_size = 256);

    /**
     * Get bounding box in meters (EPSG:3875)
     *
     * \return Computed coordinates
     */
    OGREnvelope get_bbox_meters() const;

    /**
     * Get bounding box in degrees (EPSG:4326)
     *
     * \return Computed coordinates
     */
    OGREnvelope get_bbox_deg() const;

    /**
     * Convert coordinate from meters to degrees
     *
     * \param[in] in Input coordinate (degrees)
     * \return Output coordinate (meters)
     */
    coord deg_to_meters(const coord &in) const;

    /**
     * Convert coordinate from meters to degrees
     *
     * \param[in] in Input coordinate (meters)
     * \return Output coordinate (degrees)
     */
    coord meters_to_deg(const coord &in) const;

    /**
     * Convert coordinate from meters to pixels
     *
     * \param[in] in Input coordinate (degrees)
     * \return Output coordinate (meters)
     */
    coord meters_to_pixels(const coord &in) const;

    /**
     * Convert coordinate from pixels to meters
     *
     * \param[in] in Input coordinate (meters)
     * \return Output coordinate (degrees)
     */
    coord pixels_to_meters(const coord &in) const;

private:

    /// Map offset from bottom left (meters)
    double offset_m_;

    /// Pixels per meter
    double ppm_;

    /// Computed bounding box (meters)
    OGREnvelope bbox_m_;
};

}; // ~namespace encviz
