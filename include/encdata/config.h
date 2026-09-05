#pragma once

/**
 * \file
 * \brief JSON Configuration Reader
 *
 * Configuration helper class
 */

#include <string>
#include <filesystem>
#include <optional>
#include <json/value.h>

namespace encdata
{

/// Configuration helper class
class config
{

public:

    /**
     * Constructor
     *
     * \param[in] argv0 Application run path
     * \param[in] config_dir Specified directory with config file
     */
    config(char const *argv0, const char *path = nullptr);

    /**
     * Get Configuration Directory
     */
    const std::filesystem::path &get_user_config_dir();

    /**
     * Get Data Share Directory
     */
    const std::filesystem::path &get_user_share_dir();

    /**
     * Get Configuration as Type
     *
     * \param[in] path Path in JSON value
     * \param[in] default_value Default value if not found (optional)
     * \return Found value, or default if available
     */
    template <class T>
    T get(const std::string &path,
          const std::optional<T> &default_value = {},
          const Json::Value &base = {})
    {
        // If given, convert default value to JSON
        Json::Value default_json;
        if (default_value)
        {
            default_json = default_value.value();
        }

        // Query value and convert
        Json::Value value = get_json(path, default_json, base);
        if (!value.is<T>())
        {
            throw std::runtime_error("JSON path wrong type: " + path);
        }

        return value.as<T>();
    };

    /**
     * Get Configuration as JSON
     *
     * \param[in] path Path in JSON value
     * \param[in] default_value Default value if not found (optional)
     * \return Found value, or default if available
     */
    Json::Value get_json(const std::string &path,
                         const Json::Value &default_value = {},
                         const Json::Value &base = {});

private:

    /// Config directory
    std::filesystem::path config_dir_;

    /// Data share directory
    std::filesystem::path share_dir_;

    /// Parsed config file
    Json::Value root_;
};

}; // ~namespace encdata
