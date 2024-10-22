#include <filesystem>
#include <encviz/enc_dataset.h>

/**
 * Constructor
 *
 * \param[in] enc_root ENC_ROOT base directory
 */
enc_dataset::enc_dataset(const char *enc_root)
{
    std::string as_string(enc_root);
    index_charts(enc_root);
}

/**
 * Constructor
 *
 * \param[in] enc_root ENC_ROOT base directory
 */
enc_dataset::enc_dataset(const std::filesystem::path &enc_root)
{
    index_charts(enc_root);
}

/**
 * Index ENC Charts
 *
 * \param[in] enc_root ENC_ROOT base directory
 */
void enc_dataset::index_charts(const std::filesystem::path &enc_root)
{
    namespace fs = std::filesystem;

    // In case this gets called more than once
    charts_.clear();

    // Quickly scan for any .000 files in our directory root
    for (const fs::directory_entry &entry : fs::recursive_directory_iterator(enc_root))
    {
	if (entry.path().extension() != ".000")
	    continue;
	charts_.push_back(load_metadata(entry));
    }
    printf("Loaded %lu charts ...\n", charts_.size());
}

/**
 * Load ENC Metadata
 *
 * \param[in] enc_path Path to ENC chart
 * \return Loaded metadata
 */
enc_metadata enc_dataset::load_metadata(const std::filesystem::path &enc_path)
{
    // FIXME
    enc_metadata next =
    {
	.base_name = enc_path.stem(),
	.full_path = enc_path.string(),
	.scale = 0,
	.bbox_deg_ = {}
    };
    return next;
}
       
