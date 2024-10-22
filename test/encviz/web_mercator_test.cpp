#include <array>
#include <vector>
#include <gtest/gtest.h>
#include <encviz/web_mercator.h>
using namespace testing;

struct web_mercator_testcase
{
    std::array<std::size_t, 3> tile;
    bound_box bbox_m;
    bound_box bbox_d;
};
std::vector<web_mercator_testcase> testcases = {
    {
	// Full map
	{0, 0, 0},
	{{-20037508,-20037508}, {20037508,20037508}},
	{{-180, -85.051129}, {180, 85.051129}}
    },
    {
	// Florida
	{8, 18, 5},
	{{-10018754, 2504689}, {-8766410, 3757033}},
	{{-90, 21.943046}, {-78.75, 31.952162}}
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
	bound_box bb_m = wm.get_bbox_meters();
	ASSERT_NEAR(bb_m.min.x, tc.bbox_m.min.x, 1);
	ASSERT_NEAR(bb_m.min.y, tc.bbox_m.min.y, 1);
	ASSERT_NEAR(bb_m.max.x, tc.bbox_m.max.x, 1);
	ASSERT_NEAR(bb_m.max.y, tc.bbox_m.max.y, 1);

	// Query bounding box in degrees, which checks meters_to_deg()
	bound_box bb_d = wm.get_bbox_deg();
	ASSERT_NEAR(bb_d.min.x, tc.bbox_d.min.x, 1e-6);
	ASSERT_NEAR(bb_d.min.y, tc.bbox_d.min.y, 1e-6);
	ASSERT_NEAR(bb_d.max.x, tc.bbox_d.max.x, 1e-6);
	ASSERT_NEAR(bb_d.max.y, tc.bbox_d.max.y, 1e-6);

	// Check that reverse deg_to_meters() is working too
	{
	    coord c = wm.deg_to_meters(bb_d.min);
	    ASSERT_NEAR(bb_m.min.x, c.x, 1);
	    ASSERT_NEAR(bb_m.min.y, c.y, 1);
	}

	// Check pixel coordinates of Min X/Y coords
	{
	    coord c = wm.meters_to_pixels(bb_m.min);
	    ASSERT_NEAR(c.x, 0, 1e-6);
	    ASSERT_NEAR(c.y, 256, 1e-6);

	    coord c2 = wm.pixels_to_meters(c);
	    ASSERT_NEAR(bb_m.min.x, c2.x, 1e-6);
	    ASSERT_NEAR(bb_m.min.y, c2.y, 1e-6);
	}

	// Check pixel coordinates of Max X/Y coords
	{
	    coord c = wm.meters_to_pixels(bb_m.max);
	    ASSERT_NEAR(c.x, 256, 1e-6);
	    ASSERT_NEAR(c.y, 0, 1e-6);

	    coord c2 = wm.pixels_to_meters(c);
	    ASSERT_NEAR(bb_m.max.x, c2.x, 1e-6);
	    ASSERT_NEAR(bb_m.max.y, c2.y, 1e-6);
	}
    }
}
