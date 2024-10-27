#include <filesystem>
#include <encviz/enc_dataset.h>

namespace encviz
{

/**
 * Constructor
 *
 * \param[in] enc_root ENC_ROOT base directory
 */
enc_dataset::enc_dataset(const std::string &enc_root)
{
    std::vector<std::string> enc_roots = {enc_root};
    index_charts(enc_roots);
}

/**
 * Constructor
 *
 * \param[in] enc_roots ENC_ROOT base directories
 */
enc_dataset::enc_dataset(const std::vector<std::string> &enc_roots)
{
    index_charts(enc_roots);
}

/**
 * Index ENC Charts
 *
 * \param[in] enc_roots ENC_ROOT base directories
 */
void enc_dataset::index_charts(const std::vector<std::string> &enc_roots)
{
    // In case this gets called more than once
    charts_.clear();

    // Scan for any .000 files in each passed root
    for (const std::string &enc_root : enc_roots)
    {
        auto rdi = std::filesystem::recursive_directory_iterator(enc_root);
        for (const std::filesystem::directory_entry &entry : rdi)
        {
            if (entry.path().extension() == ".000")
            {
                charts_.push_back(enc_metadata(entry.path()));
            }
        }
    }

    // Debug
    printf("Loaded %lu charts ...\n", charts_.size());
}

}; // ~namespace encviz
