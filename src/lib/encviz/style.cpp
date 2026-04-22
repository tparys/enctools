/**
 * \file
 * \brief ENC Layer Styles
 *
 * Structures and definitions for loading information on styling ENC(S-57) data.
 */

#include <cstdio>
#include <sstream>
#include <stdexcept>
#include <encviz/style.h>
#include <encviz/xml_config.h>

#define GET8(x, y) (0xff & ((x) >> (y)))
#define GET4(x, y) (0x11 * (0xf & ((x) >> (y))))

namespace encviz
{

/**
 * Parse Color Code
 *
 * Color code pattern can be one of:
 *  - 4 bit RGB  : "f0f"
 *  - 4 bit ARGB : "ff0f"
 *  - 8 bit RGB  : "ff00ff"
 *  - 8 bit ARGB : "ffff00ff"
 *
 * \param[in] text Color code text
 * \return Parsed color code
 */
color parse_color(const char *code)
{
    int len = strlen(code);

    // Ensure this parses correctly as a hex code
    char *eptr = nullptr;
    unsigned long bits = strtoul(code, &eptr, 16);
    if ((eptr - code) != len)
    {
        throw std::runtime_error("Invalid color code");
    }

    // Interpret bits
    color parsed;
    switch (len)
    {
        case 3:
            // 4 bit, 3 channel (RGB)
            parsed.alpha = 0xff;
            parsed.red   = GET4(bits, 8);
            parsed.green = GET4(bits, 4);
            parsed.blue  = GET4(bits, 0);
            break;

        case 4:
            // 4 bit, 4 channel (ARGB)
            parsed.alpha = GET4(bits, 12);
            parsed.red   = GET4(bits, 8);
            parsed.green = GET4(bits, 4);
            parsed.blue  = GET4(bits, 0);
            break;
            
        case 6:
            // 8 bit, 3 channel (RGB)
            parsed.alpha = 0xff;
            parsed.red   = GET8(bits, 16);
            parsed.green = GET8(bits, 8);
            parsed.blue  = GET8(bits, 0);
            break;

        case 8:
            // 8 bit, 4 channel (ARGB)
            parsed.alpha = GET8(bits, 24);
            parsed.red   = GET8(bits, 16);
            parsed.green = GET8(bits, 8);
            parsed.blue  = GET8(bits, 0);
            break;

        default:
            throw std::runtime_error("Invalid color code");
            break;
    }

    return parsed;
}

/**
 * Parse Layer Style
 *
 * \param[in] node Layer element
 * \return Parsed layer style
 */
layer_style parse_layer_style(tinyxml2::XMLElement *node)
{
    // Sanity check
    if (node == nullptr)
    {
        throw std::runtime_error("Layer style may not be null");
    }

    // Required XML style bits
    layer_style parsed;
    parsed.layer_name = xml_text(xml_query(node, "name"));
    tinyxml2::XMLElement *child;
    child = xml_query(node, "style");
    parsed.style = parse_simple_style(child);

    // Optional attribute based cutoffs
    child = xml_query(node, "cutoff_attr", true);
    if (child)
    {
        parsed.cutoff_attr = xml_text(child);
        for (auto style_node : xml_query_all(node, "cutoff"))
        {
            child = xml_query(style_node, "value");
            parsed.cutoff_values.push_back(atof(xml_text(child)));
            child = xml_query(style_node, "style");
            parsed.cutoff_styles.push_back(parse_simple_style(child, parsed.style));
        }
    }
    
    return parsed;
}

/**
 * Parse Simple Style
 *
 * \param[in] node Layer element
 * \return Parsed layer style
 */
simple_style parse_simple_style(tinyxml2::XMLElement *node)
{
    simple_style defaults;
    return parse_simple_style(node, defaults);
}

/**
 * Parse Simple Style with default
 *
 * \param[in] node Layer element
 * \param[in] defaults Default style
 * \return Parsed layer style
 */
simple_style parse_simple_style(tinyxml2::XMLElement *node,
                                const simple_style &defaults)
{
    // Sanity check
    if (node == nullptr)
    {
        throw std::runtime_error("Layer style may not be null");
    }

    // Start from specified defaults
    simple_style parsed = defaults;

    // Load any specified elements
    tinyxml2::XMLElement *child;
    child = xml_query(node, "fill_color", true);
    if (child)
    {
        parsed.fill_color = parse_color(xml_text(child));
    }
    child = xml_query(node, "line_color", true);
    if (child)
    {
        parsed.line_color = parse_color(xml_text(child));
    }
    child = xml_query(node, "line_width", true);
    if (child)
    {
        parsed.line_width = atoi(xml_text(child));
    }
    child = xml_query(node, "marker_size", true);
    if (child)
    {
        parsed.marker_size = atoi(xml_text(child));
    }
    child = xml_query(node, "text_color", true);
    if (child)
    {
        parsed.text_color = parse_color(xml_text(child));
    }
    child = xml_query(node, "text_font", true);
    if (child)
    {
        parsed.text_font = xml_text(child);
    }
    child = xml_query(node, "text_size", true);
    if (child)
    {
        parsed.text_size = atoi(xml_text(child));
    }
    
    return parsed;
}

/**
 * Load Style from File
 *
 * \param[in] filename Path to style file
 * \return Loaded style
 */
render_style load_style(const std::string &filename)
{
    // Load XML document
    tinyxml2::XMLDocument doc;
    if (doc.LoadFile(filename.c_str()))
    {
        // Parse error?
        throw std::runtime_error("Cannot parse " + filename);
    }

    // Load Style
    tinyxml2::XMLElement *root = doc.RootElement();
    render_style parsed;
    try
    {
        parsed.background = parse_color(xml_text(xml_query(root, "background")));
    }
    catch (...) {}
    for (tinyxml2::XMLElement *child : xml_query_all(root, "layer"))
    {
        parsed.layers.push_back(parse_layer_style(child));
    }
    
    return parsed;
}

}; // ~namespace encviz
