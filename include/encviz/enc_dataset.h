#pragma once
#include <string>
#include <vector>
#include <filesystem>
#include <encviz/enc_metadata.h>

class enc_dataset
{
public:

    /**
     * Constructor
     *
     * \param[in] enc_root ENC_ROOT base directory
     */
    enc_dataset(const char *enc_root);

    /**
     * Constructor
     *
     * \param[in] enc_root ENC_ROOT base directory
     */
    enc_dataset(const std::filesystem::path &enc_root);

private:

    /**
     * Index ENC Charts
     *
     * \param[in] enc_root ENC_ROOT base directory
     */
    void index_charts(const std::filesystem::path &enc_root);

    /**
     * Load ENC Metadata
     *
     * \param[in] enc_path Path to ENC chart
     * \return Loaded metadata
     */
    enc_metadata load_metadata(const std::filesystem::path &enc_path);

    std::vector<enc_metadata> charts_;
};
