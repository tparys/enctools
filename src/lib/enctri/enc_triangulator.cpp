#include <stdexcept>
#include <enctri/enc_triangulator.h>

namespace enctri
{

/**
 * Constructor
 *
 * \param[in] ds ENC dataset
 */
enc_triangulator::enc_triangulator(GDALDataset *ds,
                                   OGRCoordinateTransformation *ds_ct)
    : ds_(ds)
    , ds_ct_(ds_ct)
{
    load_land_areas();
    load_soundings();
}

/**
 * Rasterize to GDAL Dataset
 *
 */
void enc_triangulator::gdal_rasterize(std::string const &driver,
                                      std::string const &filename,
                                      double res, float nodata)
{
    // Rasterize to internal buffer
    enctri::raster grid;
    rasterize(grid, res, nodata);

    // Get driver by name
    GDALDriver *drv = GetGDALDriverManager()->GetDriverByName(driver.c_str());
    if (drv == nullptr)
    {
        throw std::runtime_error("Invalid GDAL driver");
    }

    // Create output dataset
    GDALDataset *ds = drv->Create(filename.c_str(), grid.size_x, grid.size_y,
                                  1, GDT_Float32, nullptr);
    if (ds == nullptr)
    {
        throw std::runtime_error("Cannot create output dataset");
    }

    // Set geo transform
    double xform[6] = { grid.bbox.min.x, res, 0, grid.bbox.max.y, 0, -res };
    ds->SetGeoTransform(xform);

    // Get raster band
    GDALRasterBand *band = ds->GetRasterBand(1);
    band->SetNoDataValue(nodata);

    // Write raster one row at a time
    std::vector<float> row;
    for (size_t y = 0; y < grid.size_y; y++)
    {
        size_t yinv = grid.size_y - y - 1;
        size_t offset = (yinv * grid.size_x);
        if (band->RasterIO(GF_Write, 0, y, grid.size_x, 1,
                           grid.data.data() + offset,
                           grid.size_x, 1, GDT_Float32, 0, 0))
        {
            throw std::runtime_error("Cannot write to output dataset");
        }
    }

    // Cleanup
    delete ds;
}

/**
 * Load Land Areas (LNDARE)
 */
void enc_triangulator::load_land_areas()
{
    // Load layer
    OGRLayer *layer = ds_->GetLayerByName("LNDARE");
    if (!layer)
    {
        throw std::runtime_error("Cannot get layer: LNDARE");
    }

    // Iterate features in layer
    for (const auto &feat : layer)
    {
        OGRGeometry *geo = feat->GetGeometryRef();
        OGRwkbGeometryType gtype = geo->getGeometryType();

        switch (gtype)
        {
            case wkbPoint: // 1
                add_point(convert_ogrpoint3d(geo->toPoint()));
                break;

            case wkbLineString: // 2
                add_path(convert_ogrlinestring(geo->toLineString()), 0);
                break;

            case wkbPolygon: // 3
                add_poly(convert_ogrpoly(geo->toPolygon()), 0);
                break;

            case wkbMultiPoint: // 4
                for (const OGRPoint *child : geo->toMultiPoint())
                {
                    add_point(convert_ogrpoint3d(child));
                }
                break;

            default:
                printf("Unhandled OGR type %08X on line %d\n", gtype, __LINE__);
                exit(1);
                break;
        }
    }
}

/**
 * Load Soundings (SOUNDG)
 */
void enc_triangulator::load_soundings()
{
    // Load layer
    OGRLayer *layer = ds_->GetLayerByName("SOUNDG");
    if (!layer)
    {
        throw std::runtime_error("Cannot get layer: SOUNDG");
    }

    // Iterate features in layer
    for (const auto &feat : layer)
    {
        OGRGeometry *geo = feat->GetGeometryRef();
        OGRwkbGeometryType gtype = geo->getGeometryType();

        switch (gtype)
        {
            case wkbPoint25D: // 0x80000001
                add_point(convert_ogrpoint3d(geo->toPoint()));
                break;

            case wkbMultiPoint25D: // 0x80000004
                for (const OGRPoint *child : geo->toMultiPoint())
                {
                    add_point(convert_ogrpoint3d(child));
                }
                break;

            default:
                printf("Unhandled OGR type %08X on line %d\n", gtype, __LINE__);
                exit(1);
                break;
        }
    }

}

encdata::path_2d enc_triangulator::convert_ogrpoly(const OGRPolygon *poly)
{
    encdata::path_2d points;
    for (auto &point : poly->getExteriorRing())
    {
        points.push_back(convert_ogrpoint2d(&point));
    }

    // OGR polygons repeat first point as last, as we don't really want/need
    // repeat points in the triangulation
    if (points.size() > 2)
    {
        size_t last = points.size() - 1;
        if ((points[0].x == points[last].x) &&
            (points[0].y == points[last].y))
        {
            points.resize(points.size() - 1);
        }
    }

    return points;
}

encdata::path_2d enc_triangulator::convert_ogrlinestring(const OGRLineString *line)
{
    encdata::path_2d points;
    for (auto &point : line)
    {
        points.push_back(convert_ogrpoint2d(&point));
    }
    return points;

}

encdata::point_2d enc_triangulator::convert_ogrpoint2d(const OGRPoint *point)
{
    encdata::point_2d p = { point->getX(), point->getY() };
    if (ds_ct_)
    {
        ds_ct_->Transform(1, &p.x, &p.y);
    }
    return p;
}

encdata::point_3d enc_triangulator::convert_ogrpoint3d(const OGRPoint *point)
{
    encdata::point_3d p = { point->getX(), point->getY(), point->getZ() };
    if (ds_ct_)
    {
        ds_ct_->Transform(1, &p.x, &p.y);
    }
    return p;
}

}; // ~namespace enctri
