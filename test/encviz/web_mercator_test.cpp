#include <array>
#include <vector>
#include <gtest/gtest.h>
#include <encviz/web_mercator.h>
using namespace testing;
using namespace encviz;

struct web_mercator_testcase
{
    std::array<std::size_t, 3> tile;
    coord bbox_min_m;
    coord bbox_max_m;
    coord bbox_min_d;
    coord bbox_max_d;
};

std::vector<web_mercator_testcase> testcases = {
    {
	// Full map
	{0, 0, 0},
	{-20037508, -20037508},
        {20037508,20037508},
	{-180, -85.051129},
        {180, 85.051129}
    },
    {
	// Florida
	{8, 18, 5},
	{-10018754, 2504689},
        {-8766410, 3757033},
	{-90, 21.943046},
        {-78.75, 31.952162}
    }
};

TEST(web_mercator, testcases)
{
    for (const auto &tc : testcases)
    {
	std::cout << "Running tests for tile: X=" << tc.tile[0]
		  << ", Y=" << tc.tile[1] << ", Z=" << tc.tile[2] << std::endl;

	// Create coordinate mapper
	web_mercator wm(tc.tile[0], tc.tile[1], tc.tile[2]);

	// Query bounding box in meters
	OGREnvelope bb_m = wm.get_bbox_meters();
	ASSERT_NEAR(bb_m.MinX, tc.bbox_min_m.x, 1);
	ASSERT_NEAR(bb_m.MinY, tc.bbox_min_m.y, 1);
	ASSERT_NEAR(bb_m.MaxX, tc.bbox_max_m.x, 1);
	ASSERT_NEAR(bb_m.MaxY, tc.bbox_max_m.y, 1);

	// Query bounding box in degrees, which checks meters_to_deg()
	OGREnvelope bb_d = wm.get_bbox_deg();
	ASSERT_NEAR(bb_d.MinX, tc.bbox_min_d.x, 1e-6);
	ASSERT_NEAR(bb_d.MinY, tc.bbox_min_d.y, 1e-6);
	ASSERT_NEAR(bb_d.MaxX, tc.bbox_max_d.x, 1e-6);
	ASSERT_NEAR(bb_d.MaxY, tc.bbox_max_d.y, 1e-6);

	// Check that reverse deg_to_meters() is working too
	{
            coord cd = { bb_d.MinX, bb_d.MinY };
	    coord cm = wm.deg_to_meters(cd);
	    ASSERT_NEAR(bb_m.MinX, cm.x, 1);
	    ASSERT_NEAR(bb_m.MinY, cm.y, 1);
	}

	// Check pixel coordinates of Min X/Y coords
	{
	    coord c = wm.meters_to_pixels(tc.bbox_min_m);
	    ASSERT_NEAR(c.x, 0, 1e-3);
	    ASSERT_NEAR(c.y, 256, 1e-3);

	    coord c2 = wm.pixels_to_meters(c);
	    ASSERT_NEAR(tc.bbox_min_m.x, c2.x, 1e-6);
	    ASSERT_NEAR(tc.bbox_min_m.y, c2.y, 1e-6);
	}

	// Check pixel coordinates of Max X/Y coords
	{
	    coord c = wm.meters_to_pixels(tc.bbox_max_m);
	    ASSERT_NEAR(c.x, 256, 1e-3);
	    ASSERT_NEAR(c.y, 0, 1e-3);

	    coord c2 = wm.pixels_to_meters(c);
	    ASSERT_NEAR(tc.bbox_max_m.x, c2.x, 1e-6);
	    ASSERT_NEAR(tc.bbox_max_m.y, c2.y, 1e-6);
	}
    }
}
