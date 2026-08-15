/**
 * \file
 * \brief JSON Configuration Reader
 *
 * Configuration helper class
 */

#include <cstdio>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <json/reader.h>
#include <encdata/config.h>

namespace encdata
{

/**
 * Constructor
 *
 * \param[in] path Specified path for config file
 */
config::config(const char *path)
{
    // Use default path if one not specified
    std::string final_path;
    if (path == nullptr)
    {
        std::filesystem::path default_path = getenv("HOME");
        default_path.append(".enctools");
        default_path.append("config.json");
        final_path = default_path.string();
    }
    else
    {
        final_path = path;
    }
    
    // Open input file
    printf("Using config file: %s\n", final_path.c_str());
    std::ifstream handle(final_path.c_str());
    if (!handle.good())
    {
        throw std::runtime_error("Cannot open config file");
    }

    // Parse from input file
    Json::Reader reader;
    if (!reader.parse(handle, root_))
    {
        std::string msg = "Cannot parse config file: ";
        msg += reader.getFormattedErrorMessages();
        throw std::runtime_error(msg);
    }
}

/**
 * Get Configuration as JSON
 *
 * \param[in] path Path in JSON value
 * \param[in] default_value Default value if not found (optional)
 * \param[in] base Alternate search root (optional)
 * \return Found value, or default if available
 */
Json::Value config::get_json(const std::string &path,
                             const Json::Value &default_value,
                             const Json::Value &base)
{
    // Build search path
    Json::Path jpath(path);

    // Query default or given JSON structure
    const Json::Value &search_root = ( base ? base : root_ );
    const Json::Value &result = jpath.resolve(search_root, default_value);

    // Ensure we have something to return
    if (result.type() == Json::ValueType::nullValue)
    {
        throw std::runtime_error("JSON path not found: " + path);
    }

    return result;
}

}; // ~namespace encdata
