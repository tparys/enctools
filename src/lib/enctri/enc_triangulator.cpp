#include <stdexcept>
#include <enctri/enc_triangulator.h>

namespace enctri
{

/**
 * Constructor
 *
 * \param[in] ds ENC dataset
 */
enc_triangulator::enc_triangulator(GDALDataset *ds)
    : ds_(ds)
{
    load_land_areas();
    load_soundings();
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
            case wkbPoint: // 1
            case wkbPoint25D: // 0x80000001
                add_point(convert_ogrpoint3d(geo->toPoint()));
                break;

            case wkbMultiPoint: // 4
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
    return p;
}

encdata::point_3d enc_triangulator::convert_ogrpoint3d(const OGRPoint *point)
{
    encdata::point_3d p = { point->getX(), point->getY(), 0 };
    if (point->getDimension() > 2)
    {
        p.z = point->getZ();
    }
    return p;
}

}; // ~namespace enctri
