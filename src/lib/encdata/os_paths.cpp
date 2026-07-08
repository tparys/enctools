/**
 * \file
 * \brief OS Path Utilities
 *
 * General utility functions for dealing with OS specific paths.
 */

#include <cstdlib>
#include <encdata/os_paths.h>

namespace encdata
{

/**
 * Get Config File Path
 *
 * \param[in] path Explicit path to use (optional)
 * \return Filesystem path
 */
std::filesystem::path get_user_config_path(const char *path)
{
    // Load specified, or default config path
    if (path != nullptr)
    {
        return path;
    }
    else
    {
        // Default to ~/.enctools
        std::filesystem::path default_path = getenv("HOME");
        default_path.append(".enctools");
        return default_path;
    }
}

/**
 * Get Application Share Directory
 *
 * \param[in] path Application runtime path (argv[0])
 * \return Filesystem path
 */
std::filesystem::path get_user_share_path(const char *path)
{
    // Attempt to map:
    //  - /usr/bin/app -> /usr/share/enctools
    //  - /usr/local/bin/app -> /usr/local/share/enctools
    //  - build/bin/app -> build/share/enctools
    std::filesystem::path work = path;
    work = work.parent_path().parent_path();
    work = work.append("share").append("enctools");
    return work;
}

}; // ~namespace encdata
