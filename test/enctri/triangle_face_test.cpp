#include <array>
#include <vector>
#include <gtest/gtest.h>
#include <enctri/triangle_face.h>
using namespace testing;
using namespace encdata;
using namespace enctri;

#define EPSILON 1e-3

TEST(triangle_face, testcases)
{
    std::array<point_3d, 3> vertices = {
        {
            { -1, -1, 0 },
            { 1, -1, 2 },
            { 0, 1, -2 }
        }
    };

    triangle_face face(vertices);

    // Check bounding box
    bbox_2d bbox = face.get_bbox();
    EXPECT_NEAR(bbox.min.x, -1, EPSILON);
    EXPECT_NEAR(bbox.min.y, -1, EPSILON);
    EXPECT_NEAR(bbox.max.x, 1, EPSILON);
    EXPECT_NEAR(bbox.max.y, 1, EPSILON);

    // Check centroid
    point_2d centroid = face.get_centroid();
    EXPECT_NEAR(centroid.x, 0, EPSILON);
    EXPECT_NEAR(centroid.y, -.333, EPSILON);

    // Check a few points for inside / outside
    point_2d pa = { 0, -2 };
    point_2d pb = { -1, 1 };
    point_2d pc = { 1, 1 };
    EXPECT_TRUE(face.contains(centroid));
    EXPECT_FALSE(face.contains(pa));
    EXPECT_FALSE(face.contains(pb));
    EXPECT_FALSE(face.contains(pc));

    // Check height at vertices
    pa = { -1, -1 };
    pb = { 1, -1 };
    pc = { 0, 1 };
    EXPECT_NEAR(face.height(pa), 0, EPSILON);
    EXPECT_NEAR(face.height(pb), 2, EPSILON);
    EXPECT_NEAR(face.height(pc), -2, EPSILON);
}
