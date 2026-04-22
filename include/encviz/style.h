#pragma once

/**
 * \file
 * \brief ENC Layer Styles
 *
 * Structures and definitions for loading information on styling ENC(S-57) data.
 */

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

/// Simple style
struct simple_style
{
    /// Fill color
    color fill_color;

    /// Line color
    color line_color;

    /// Line color
    int line_width{1};

    /// Circular marker radius
    int marker_size;

    /// Text color
    color text_color;

    /// Text font
    std::string text_font{"monospace"};

    /// Text size
    int text_size{12};
};

/// Style for a single layer
struct layer_style
{
    /// Name of layer
    std::string layer_name;

    /// Default render style
    simple_style style;

    /// Attribute used for cutoffs
    std::string cutoff_attr;

    /// Other styles
    std::vector<simple_style> cutoff_styles;

    /// Attribute cutoffs for those styles
    std::vector<double> cutoff_values;
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
 * Parse Color Code
 *
 * Color code pattern can be one of:
 *  - 4 bit RGB  : "f0f"
 *  - 4 bit ARGB : "ff0f"
 *  - 8 bit RGB  : "ff00ff"
 *  - 8 bit ARGB : "ffff00ff"
 *
 * \param[in] code Color code text
 * \return Parsed color code
 */
color parse_color(const char *code);

/**
 * Parse Layer Style
 *
 * \param[in] node Layer element
 * \return Parsed layer style
 */
layer_style parse_layer_style(tinyxml2::XMLElement *node);

/**
 * Parse Simple Style
 *
 * \param[in] node Layer element
 * \return Parsed layer style
 */
simple_style parse_simple_style(tinyxml2::XMLElement *node);

/**
 * Parse Simple Style with default
 *
 * \param[in] node Layer element
 * \param[in] defaults Default style
 * \return Parsed layer style
 */
simple_style parse_simple_style(tinyxml2::XMLElement *node,
                                const simple_style &defaults);

/**
 * Load Style from File
 *
 * \param[in] filename Path to style file
 * \return Loaded style
 */
render_style load_style(const std::string &filename);

}; // ~namespace encviz
