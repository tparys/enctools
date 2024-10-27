#pragma once
#include <string>
#include <vector>
#include <encviz/enc_metadata.h>

namespace encviz
{

class enc_dataset
{
public:

    /**
     * Constructor
     *
     * \param[in] enc_root ENC_ROOT base directory
     */
    enc_dataset(const std::string &enc_root);

    /**
     * Constructor
     *
     * \param[in] enc_root ENC_ROOT base directories
     */
    enc_dataset(const std::vector<std::string> &enc_roots);

private:

    /**
     * Index ENC Charts
     *
     * \param[in] enc_roots ENC_ROOT base directories
     */
    void index_charts(const std::vector<std::string> &enc_root);

    std::vector<enc_metadata> charts_;
};

}; // ~namespace encviz
