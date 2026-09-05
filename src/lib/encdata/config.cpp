/**
 * \file
 * \brief JSON Configuration Reader
 *
 * Configuration helper class
 */

#include <cstdio>
#include <iostream>
#include <fstream>
#include <json/reader.h>
#include <encdata/config.h>

namespace encdata
{

/**
 * Constructor
 *
 * \param[in] argv0 Application run path
 * \param[in] path Specified directory with config file
 */
config::config(char const *argv0, const char *config_dir)
{
    // Determine config directory location
    if (config_dir == nullptr)
    {
        config_dir_ = getenv("HOME");
        config_dir_.append(".enctools");
    }
    else
    {
        config_dir_ = config_dir;
    }

    // Determine data share location
    //  - /usr/bin/app -> /usr/share/enctools
    //  - /usr/local/bin/app -> /usr/local/share/enctools
    //  - build/bin/app -> build/share/enctools
    share_dir_ = argv0;
    share_dir_ = share_dir_.parent_path().parent_path();
    share_dir_ = share_dir_.append("share").append("enctools");

    // Config file location
    std::string config_file = (config_dir_ / "config.json").string();

    // Open config file
    printf("Using config file: %s\n", config_file.c_str());
    std::ifstream handle(config_file.c_str());
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
 * Get Configuration Directory
 */
const std::filesystem::path &config::get_user_config_dir()
{
    return config_dir_;
}

/**
 * Get Data Share Directory
 */
const std::filesystem::path &config::get_user_share_dir()
{
    return share_dir_;
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
