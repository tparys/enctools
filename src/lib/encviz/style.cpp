/**
 * \file
 * \brief ENC Layer Styles
 *
 * Structures and definitions for loading information on styling ENC(S-57) data.
 */

#include <cstdio>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <json/reader.h>
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
 * Load Color from Code or Theme
 *
 * \param[in] color Color code text
 * \param[in] theme Color code theme
 * \return Parsed color code
 */
color load_color(const char *color, const color_theme &theme)
{
    if (color[0] == '@')
    {
        // Theme color
        std::string name = color;
        name = name.substr(1);
        auto it = theme.find(name);
        if (it == theme.end())
        {
            throw std::runtime_error("Invalid theme color");
        }
        return it->second;
    }
    else
    {
        // Base color code
        return parse_color(color);
    }
}

/**
 * Parse Layer Style
 *
 * \param[in] node Layer element
 * \param[in] theme Color theme for file
 * \return Parsed layer style
 */
layer_style parse_layer_style(tinyxml2::XMLElement *node,
                              const color_theme &theme)
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
    parsed.style = parse_simple_style(child, theme);

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
            parsed.cutoff_styles.push_back(parse_simple_style(child, theme, parsed.style));
        }
    }
    
    return parsed;
}

/**
 * Parse Layer Style
 *
 * \param[in] node Layer element
 * \param[in] theme Color theme for file
 * \return Parsed layer style
 */
layer_style parse_layer_style_json(Json::Value &node,
                                   const color_theme &theme)
{
    // Required XML style bits
    layer_style parsed;
    parsed.layer_name = node["name"].asString();
    if (node.isMember("style"))
    {
        parsed.style = parse_simple_style_json(node["style"], theme);
    }
    if (node.isMember("cutoff"))
    {
        Json::Value cutoff = node["cutoff"];
        parsed.cutoff_attr = cutoff["attr"].asString();
        Json::Value values = cutoff["values"];
        for (Json::ArrayIndex i = 0; i < values.size(); i++)
        {
            parsed.cutoff_values.push_back(values[i].asFloat());
        }
        Json::Value styles = cutoff["styles"];
        for (Json::ArrayIndex i = 0; i < styles.size(); i++)
        {
            parsed.cutoff_styles.push_back(parse_simple_style_json(styles[i], theme, parsed.style));
        }
     }
    
    return parsed;
}

/**
 * Parse Simple Style
 *
 * \param[in] node Layer element
 * \param[in] theme Color theme for file
 * \return Parsed layer style
 */
simple_style parse_simple_style(tinyxml2::XMLElement *node,
                                const color_theme &theme)
{
    simple_style defaults;
    return parse_simple_style(node, theme, defaults);
}

/**
 * Parse Simple Style
 *
 * \param[in] node Layer element
 * \param[in] theme Color theme for file
 * \return Parsed layer style
 */
simple_style parse_simple_style_json(Json::Value &node,
                                     const color_theme &theme)
{
    simple_style defaults;
    return parse_simple_style_json(node, theme, defaults);
}

/**
 * Parse Simple Style with default
 *
 * \param[in] node Layer element
 * \param[in] theme Color theme for file
 * \param[in] defaults Default style
 * \return Parsed layer style
 */
simple_style parse_simple_style(tinyxml2::XMLElement *node,
                                const color_theme &theme,
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
        parsed.fill_color = load_color(xml_text(child), theme);
    }
    child = xml_query(node, "line_color", true);
    if (child)
    {
        parsed.line_color = load_color(xml_text(child), theme);
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
        parsed.text_color = load_color(xml_text(child), theme);
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
 * Parse Simple Style with default
 *
 * \param[in] node Layer element
 * \param[in] theme Color theme for file
 * \param[in] defaults Default style
 * \return Parsed layer style
 */
simple_style parse_simple_style_json(Json::Value &node,
                                     const color_theme &theme,
                                     const simple_style &defaults)
{
    // Start from specified defaults
    simple_style parsed = defaults;

    // Load any specified elements
    if (node.isMember("fill_color"))
    {
        printf("   - fill_color: %s \n", node["fill_color"].asString().c_str());
        parsed.fill_color = load_color(node["fill_color"].asString().c_str(), theme);
    }
    if (node.isMember("line_color"))
    {
        parsed.line_color = load_color(node["line_color"].asString().c_str(), theme);
    }
    if (node.isMember("line_width"))
    {
        parsed.line_width = node["line_width"].asInt();
    }
    if (node.isMember("marker_size"))
    {
        parsed.marker_size = node["marker_size"].asInt();
    }
    if (node.isMember("text_color"))
    {
        parsed.text_color = load_color(node["text_color"].asString().c_str(), theme);
    }
    if (node.isMember("text_font"))
    {
        parsed.text_font = node["text_font"].asString();
    }
    if (node.isMember("text_size"))
    {
        parsed.text_size = node["text_size"].asInt();
    }
    
    return parsed;
}

/**
 * Load Style from File
 *
 * \param[in] filename Path to style file
 * \return Loaded style
 */
render_style load_style(const std::string &filename, const color_theme &theme)
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
        parsed.background = load_color(xml_text(xml_query(root, "background")), theme);
    }
    catch (...) {}
    for (tinyxml2::XMLElement *child : xml_query_all(root, "layer"))
    {
        parsed.layers.push_back(parse_layer_style(child, theme));
    }
    
    return parsed;
}

/**
 * Load Style from File
 *
 * \param[in] filename Path to style file
 * \return Loaded style
 */
render_style load_style_json(const std::string &filename, const color_theme &theme)
{
    // Open input file
    std::ifstream handle(filename.c_str());
    if (!handle.good())
    {
        throw std::runtime_error("Cannot open style file");
    }

    // Parse from input file
    Json::Reader reader;
    Json::Value root;
    if (!reader.parse(handle, root))
    {
        std::string msg = "Cannot parse: " + filename + ": ";
        msg += reader.getFormattedErrorMessages();
        throw std::runtime_error(msg);
    }

    // Load Style
    render_style parsed;

    // Background color
    if (root.isMember("background"))
    {
        printf(" - background\n");
        parsed.background = load_color(root["background"].asString().c_str(), theme);
    }

    // Layer style
    Json::Value layers = root["layers"];
    for (Json::ArrayIndex i = 0; i < layers.size(); i++)
    {
        printf(" - layer %u\n", i);
        parsed.layers.push_back(parse_layer_style_json(layers[i], theme));
    }
    
    return parsed;
}

/**
 * Load Color Themes from File
 *
 * \param[in] filename Path to style file
 * \return Loaded themes
 */
color_theme_map load_themes(const std::string &filename)
{
    // Open input file
    std::ifstream handle(filename.c_str());
    if (!handle.good())
    {
        throw std::runtime_error("Cannot open theme file");
    }

    // Parse from input file
    Json::Reader reader;
    Json::Value root;
    if (!reader.parse(handle, root))
    {
        std::string msg = "Cannot parse theme file: ";
        msg += reader.getFormattedErrorMessages();
        throw std::runtime_error(msg);
    }

    // Load theme names
    std::vector<std::string> theme_names;
    if (!root.isMember("styles") ||
        (root["styles"].type() != Json::ValueType::arrayValue))
    {
        throw std::runtime_error("Theme styles not found");
    }
    Json::Value styles = root["styles"];
    for (Json::ArrayIndex i = 0; i < styles.size(); i++)
    {
        if (styles[i].type() == Json::ValueType::stringValue)
        {
            theme_names.push_back(styles[i].asString());
        }
    }

    // Load theme colors
    color_theme_map themes;
    if (!root.isMember("table") ||
        (root["table"].type() != Json::ValueType::objectValue))
    {
        throw std::runtime_error("Theme colors not found");
    }
    Json::Value table = root["table"];
    for (const auto &color_name : table.getMemberNames())
    {
        if ((table[color_name].type() != Json::ValueType::arrayValue) ||
            (table[color_name].size() != theme_names.size()))
        {
            throw std::runtime_error("Theme names and color code count must be equal");
        }
        for (Json::ArrayIndex i = 0; i < theme_names.size(); i++)
        {
            themes[theme_names[i]][color_name] =
                parse_color(table[color_name][i].asString().c_str());
        }
    }

    // All done
    printf("Loaded %lu colors across %lu themes\n",
           themes[theme_names[0]].size(), theme_names.size());
    return themes;
}

}; // ~namespace encviz
