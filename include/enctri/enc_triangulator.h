#pragma once

#include <gdal_priv.h>
#include <ogrsf_frmts.h>
#include <enctri/triangulator.h>

namespace enctri
{

class enc_triangulator : public triangulator
{
public:

    /**
     * Constructor
     *
     * \param[in] ds ENC dataset
     */
    enc_triangulator(GDALDataset *ds);

    /**
     * Rasterize to GDAL Dataset
     *
     * \param[in] driver GDAL driver name
     * \param[in] filename Output file name
     * \param[in] res X/Y resolution
     * \param[in] nodata Sentinel value for no valid data
     */
    void gdal_rasterize(std::string const &driver,
                        std::string const &filename,
                        double res, float nodata);

private:

    /**
     * Load Land Areas (LNDARE)
     */
    void load_land_areas();

    /**
     * Load Land Areas (SOUNDG)
     */
    void load_soundings();

    encdata::path_2d convert_ogrlinestring(const OGRLineString *line);
    encdata::path_2d convert_ogrpoly(const OGRPolygon *poly);
    encdata::point_2d convert_ogrpoint2d(const OGRPoint *point);
    encdata::point_3d convert_ogrpoint3d(const OGRPoint *point);

    /// Internal GDAL dataset
    GDALDataset *ds_;

};

}; // ~namespace enctri
