#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <optional>
#include <tinyxml2.h>

namespace encviz
{

/// Simple color map
struct color
{
    /// Red channel (8 bit)
    uint8_t alpha{255};

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

/**
 * Get Node Text with Checking
 *
 * \param[in] node Element with text
 * \return Tag text
 */
const char *tag_text(tinyxml2::XMLElement *node);

/**
 * Parse Color Code
 *
 * Color code pattern can be one of:
 *  - 4 bit RGB  : "f0f"
 *  - 4 bit ARGB : "ff0f"
 *  - 8 bit RGB  : "ff00ff"
 *  - 8 bit ARGB : "ffff00ff"
 *
 * \param[in] node Color code element
 * \return Parsed color code
 */
color parse_color(tinyxml2::XMLElement *node);

/**
 * Parse Layer Style
 *
 * \param[in] node Layer element
 * \return Parsed layer style
 */
layer_style parse_layer(tinyxml2::XMLElement *node);

/**
 * Load Style from File
 *
 * \param[in] filename Path to style file
 * \return Loaded style
 */
render_style load_style(const std::string &filename);

}; // ~namespace encviz
