#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <optional>

namespace encviz
{

/// Generalized 2D coordinate
struct coord
{
    /// Horizontal coordinate (Longitude, column, etc..)
    double x;

    /// Vertical coordinate (Latitude, row, etc..)
    double y;
};

/// Simple color map
struct color
{
    /// Red channel (8 bit)
    uint8_t red{0};

    /// Blue channel (8 bit)
    uint8_t blue{0};

    /// Green channel (8 bit)
    uint8_t green{0};
};

/// Style for a single layer
struct layer_style
{
    /// Name of layer
    std::string layer_name;

    /// Fill color
    color fill_color;

    /// Line color
    color line_color;

    /// Line color
    int line_width{1};

    /// Circular marker radius
    int marker_size;

    /// Text render attribute
    std::string attr_name;
};

/// Full rendering style
struct render_style
{
    /// Background fill
    std::optional<color> background;

    /// Layers
    std::vector<layer_style> layers;
};

}; // ~namespace encviz
