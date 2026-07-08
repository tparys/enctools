#pragma once

/**
 * \file
 * \brief OS Path Utilities
 *
 * General utility functions for dealing with OS specific paths.
 */

#include <string>
#include <filesystem>

namespace encdata
{

/**
 * Get Config File Path
 *
 * \param[in] path Explicit path to use (optional)
 * \return Filesystem path
 */
std::filesystem::path get_user_config_path(const char *path = nullptr);

/**
 * Get Application Share Directory
 *
 * \param[in] path Application runtime path (argv[0])
 * \return Filesystem path
 */
std::filesystem::path get_user_share_path(const char *path);

}; // ~namespace encdata
