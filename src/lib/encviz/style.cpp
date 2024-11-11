#include <cstdio>
#include <sstream>
#include <stdexcept>
#include <encviz/style.h>

#define GET8(x, y) (0xff & ((x) >> (y)))
#define GET4(x, y) (0x11 * (0xf & ((x) >> (y))))

namespace encviz
{

/**
 * Get Node Text with Checking
 *
 * \param[in] node Element with text
 * \return Tag text
 */
const char *tag_text(tinyxml2::XMLElement *node)
{
    if (node == nullptr)
    {
        throw std::runtime_error("Cannot get text from null element");
    }
    const char *ptr = node->GetText();
    if (ptr == nullptr)
    {
        std::ostringstream oss;
        oss << "Tag <" << node->Name() << "> may not be empty";
        throw std::runtime_error(oss.str());
    }
    return ptr;
}

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
color parse_color(tinyxml2::XMLElement *node)
{
    // Compute length of color code
    const char *code = tag_text(node);
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
layer_style parse_layer(tinyxml2::XMLElement *node)
{
    // Sanity check
    if (node == nullptr)
    {
        throw std::runtime_error("Layer style may not be null");
    }

    layer_style parsed;

    // Tick through children
    for (tinyxml2::XMLElement *child = node->FirstChildElement();
         child != nullptr;
         child = child->NextSiblingElement())
    {
        // What sort of tag is this?
        const char *tag = child->Name();

        if (strcmp(tag, "layer_name") == 0)
        {
            // Set background
            parsed.layer_name = tag_text(child);
        }
        else if (strcmp(tag, "fill_color") == 0)
        {
            // Set fill color
            parsed.fill_color = parse_color(child);
        }
        else if (strcmp(tag, "line_color") == 0)
        {
            // Set line color
            parsed.line_color = parse_color(child);
        }
        else if (strcmp(tag, "line_width") == 0)
        {
            // Set line color
            parsed.line_width = atoi(tag_text(child));
        }
        else if (strcmp(tag, "marker_size") == 0)
        {
            // Set line color
            parsed.marker_size = atoi(tag_text(child));
        }
        else
        {
            printf("ERROR: Unknown tag <%s>\n", tag);
        }
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

    render_style parsed;

    // Tick through children
    tinyxml2::XMLElement *root = doc.RootElement();
    for (tinyxml2::XMLElement *child = root->FirstChildElement();
         child != nullptr;
         child = child->NextSiblingElement())
    {
        // What sort of tag is this?
        const char *tag = child->Name();
        if (strcmp(tag, "background") == 0)
        {
            // Set background
            parsed.background = parse_color(child);
        }
        else if (strcmp(tag, "layer") == 0)
        {
            parsed.layers.push_back(parse_layer(child));
        }
        else
        {
            printf("ERROR: Unknown tag <%s>\n", tag);
        }
    }
    
    return parsed;
}

}; // ~namespace encviz
