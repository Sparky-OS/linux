// SPDX-License-Identifier: GPL-2.0
/*
 * Kunit test for the HDMI 3D structures the EDID parser lists
 */

#include <drm/drm_atomic_state_helper.h>
#include <drm/drm_connector.h>
#include <drm/drm_drv.h>
#include <drm/drm_edid.h>
#include <drm/drm_kunit_helpers.h>
#include <drm/drm_modes.h>

#include <kunit/test.h>

struct drm_edid_3d_priv {
	struct drm_device *drm;
	struct drm_connector connector;
};

/*
 * Two sinks with the five 3D structures that mainline did not list, made by
 * a script and read back with edid-decode. The first declares them through
 * 3D_present, 3D_Multi_present 01 and 3D_Structure_ALL (HDMI 1.4a, Table
 * H-7): frame packing, field alternative, line alternative, L + depth and
 * L + depth + graphics + graphics-depth, over VICs 16, 4, 19, 31 and 5. The
 * second through 3D_Structure_X only (Table H-2): VIC 19 L + depth, VIC 5
 * L + depth, VIC 16 line alternative, VIC 5 field alternative, VIC 4 L +
 * depth + graphics + graphics-depth, and VIC 16 with the reserved value 7.
 */
static const u8 edid_3d_all[] = {
	0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00,
	0x31, 0xd8, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x24, 0x01, 0x03, 0x81, 0xa0, 0x5a, 0x78,
	0x0a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x01, 0x01,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x3a,
	0x80, 0x18, 0x71, 0x38, 0x2d, 0x40, 0x58, 0x2c,
	0x45, 0x00, 0x40, 0x84, 0x63, 0x00, 0x00, 0x1e,
	0x00, 0x00, 0x00, 0xfc, 0x00, 0x33, 0x44, 0x20,
	0x41, 0x4c, 0x4c, 0x0a, 0x20, 0x20, 0x20, 0x20,
	0x20, 0x20, 0x00, 0x00, 0x00, 0xfd, 0x00, 0x17,
	0x4c, 0x0f, 0x8c, 0x3c, 0x00, 0x0a, 0x20, 0x20,
	0x20, 0x20, 0x20, 0x20, 0x00, 0x00, 0x00, 0x10,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x29,
	0x02, 0x03, 0x1a, 0x81, 0x45, 0x10, 0x04, 0x13,
	0x1f, 0x05, 0x6c, 0x03, 0x0c, 0x00, 0x10, 0x00,
	0x00, 0x44, 0x20, 0xa0, 0x02, 0x00, 0x37, 0xe2,
	0x00, 0x4a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xdc,
};

static const u8 edid_3d_x[] = {
	0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00,
	0x31, 0xd8, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x24, 0x01, 0x03, 0x81, 0xa0, 0x5a, 0x78,
	0x0a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x01, 0x01,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x3a,
	0x80, 0x18, 0x71, 0x38, 0x2d, 0x40, 0x58, 0x2c,
	0x45, 0x00, 0x40, 0x84, 0x63, 0x00, 0x00, 0x1e,
	0x00, 0x00, 0x00, 0xfc, 0x00, 0x33, 0x44, 0x20,
	0x58, 0x0a, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
	0x20, 0x20, 0x00, 0x00, 0x00, 0xfd, 0x00, 0x17,
	0x4c, 0x0f, 0x8c, 0x3c, 0x00, 0x0a, 0x20, 0x20,
	0x20, 0x20, 0x20, 0x20, 0x00, 0x00, 0x00, 0x10,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x6a,
	0x02, 0x03, 0x1e, 0x81, 0x45, 0x10, 0x04, 0x13,
	0x1f, 0x05, 0x70, 0x03, 0x0c, 0x00, 0x10, 0x00,
	0x00, 0x44, 0x20, 0x00, 0x06, 0x24, 0x44, 0x02,
	0x41, 0x15, 0x07, 0xe2, 0x00, 0x4a, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0,
};

static const struct drm_connector_funcs drm_edid_3d_connector_funcs = {
	.reset = drm_atomic_helper_connector_reset,
	.atomic_duplicate_state = drm_atomic_helper_connector_duplicate_state,
	.atomic_destroy_state = drm_atomic_helper_connector_destroy_state,
};

static int drm_edid_3d_init(struct kunit *test)
{
	struct drm_edid_3d_priv *priv;
	struct device *dev;
	int ret;

	priv = kunit_kzalloc(test, sizeof(*priv), GFP_KERNEL);
	KUNIT_ASSERT_NOT_NULL(test, priv);

	dev = drm_kunit_helper_alloc_device(test);
	KUNIT_ASSERT_NOT_ERR_OR_NULL(test, dev);

	priv->drm = __drm_kunit_helper_alloc_drm_device(test, dev,
							sizeof(*priv->drm), 0,
							DRIVER_MODESET);
	KUNIT_ASSERT_NOT_ERR_OR_NULL(test, priv->drm);

	ret = drmm_connector_init(priv->drm, &priv->connector,
				  &drm_edid_3d_connector_funcs,
				  DRM_MODE_CONNECTOR_HDMIA, NULL);
	KUNIT_ASSERT_EQ(test, ret, 0);

	priv->connector.stereo_allowed = true;
	test->priv = priv;

	return 0;
}

static void drm_edid_3d_add_modes(struct kunit *test, const u8 *edid,
				  size_t size)
{
	struct drm_edid_3d_priv *priv = test->priv;
	struct drm_connector *connector = &priv->connector;
	const struct drm_edid *drm_edid;
	int ret;

	drm_edid = drm_edid_alloc(edid, size);
	KUNIT_ASSERT_NOT_NULL(test, drm_edid);
	KUNIT_ASSERT_TRUE(test, drm_edid_valid(drm_edid));

	mutex_lock(&priv->drm->mode_config.mutex);
	ret = drm_edid_connector_update(connector, drm_edid);
	KUNIT_EXPECT_EQ(test, ret, 0);
	drm_edid_connector_add_modes(connector);
	mutex_unlock(&priv->drm->mode_config.mutex);

	drm_edid_free(drm_edid);
}

/*
 * How many probed modes of that format carry that 3D structure. The clock
 * tells a format from its 1000/1001 twin, which add_alternate_cea_modes()
 * adds for every 60 Hz mode, 3D flag included.
 */
static int drm_edid_3d_count(struct kunit *test, int hdisplay, int vdisplay,
			     int vrefresh, int clock, bool interlaced, u32 layout)
{
	struct drm_edid_3d_priv *priv = test->priv;
	struct drm_display_mode *mode;
	int n = 0;

	list_for_each_entry(mode, &priv->connector.probed_modes, head) {
		if (mode->hdisplay == hdisplay && mode->vdisplay == vdisplay &&
		    drm_mode_vrefresh(mode) == vrefresh && mode->clock == clock &&
		    !!(mode->flags & DRM_MODE_FLAG_INTERLACE) == interlaced &&
		    (mode->flags & DRM_MODE_FLAG_3D_MASK) == layout)
			n++;
	}

	return n;
}

/* how many probed modes carry that 3D structure, 1000/1001 twins included */
static int drm_edid_3d_count_layout(struct kunit *test, u32 layout)
{
	struct drm_edid_3d_priv *priv = test->priv;
	struct drm_display_mode *mode;
	int n = 0;

	list_for_each_entry(mode, &priv->connector.probed_modes, head)
		if ((mode->flags & DRM_MODE_FLAG_3D_MASK) == layout)
			n++;

	return n;
}

#define FIELD_ALT	DRM_MODE_FLAG_3D_FIELD_ALTERNATIVE
#define LINE_ALT	DRM_MODE_FLAG_3D_LINE_ALTERNATIVE
#define L_DEPTH		DRM_MODE_FLAG_3D_L_DEPTH
#define L_DEPTH_GFX	DRM_MODE_FLAG_3D_L_DEPTH_GFX_GFX_DEPTH

static void drm_test_edid_3d_structure_all(struct kunit *test)
{
	drm_edid_3d_add_modes(test, edid_3d_all, sizeof(edid_3d_all));

	/* field alternative: the one interlaced format, 1080i60 (VIC 5) */
	KUNIT_EXPECT_EQ(test, drm_edid_3d_count(test, 1920, 1080, 60, 74250, true,
						FIELD_ALT), 1);
	KUNIT_EXPECT_EQ(test, drm_edid_3d_count(test, 1920, 1080, 60, 148500, false,
						FIELD_ALT), 0);

	/*
	 * line alternative and picture plus depth: the four progressive
	 * formats, VICs 16, 4, 19 and 31, and none on VIC 5
	 */
	KUNIT_EXPECT_EQ(test, drm_edid_3d_count(test, 1920, 1080, 60, 148500, false,
						LINE_ALT), 1);
	KUNIT_EXPECT_EQ(test, drm_edid_3d_count(test, 1280, 720, 60, 74250, false,
						LINE_ALT), 1);
	KUNIT_EXPECT_EQ(test, drm_edid_3d_count(test, 1280, 720, 50, 74250, false,
						L_DEPTH), 1);
	KUNIT_EXPECT_EQ(test, drm_edid_3d_count(test, 1920, 1080, 50, 148500, false,
						L_DEPTH), 1);
	KUNIT_EXPECT_EQ(test, drm_edid_3d_count(test, 1280, 720, 60, 74250, false,
						L_DEPTH_GFX), 1);
	KUNIT_EXPECT_EQ(test, drm_edid_3d_count(test, 1920, 1080, 60, 74250, true,
						LINE_ALT), 0);
	KUNIT_EXPECT_EQ(test, drm_edid_3d_count(test, 1920, 1080, 60, 74250, true,
						L_DEPTH), 0);
	KUNIT_EXPECT_EQ(test, drm_edid_3d_count(test, 1920, 1080, 60, 74250, true,
						L_DEPTH_GFX), 0);

	/* with the twins of VICs 16 and 4, six of each; field alternative two */
	KUNIT_EXPECT_EQ(test, drm_edid_3d_count_layout(test, LINE_ALT), 6);
	KUNIT_EXPECT_EQ(test, drm_edid_3d_count_layout(test, L_DEPTH), 6);
	KUNIT_EXPECT_EQ(test, drm_edid_3d_count_layout(test, L_DEPTH_GFX), 6);
	KUNIT_EXPECT_EQ(test, drm_edid_3d_count_layout(test, FIELD_ALT), 2);

	/* nothing the sink did not declare */
	KUNIT_EXPECT_EQ(test, drm_edid_3d_count_layout(test,
						       DRM_MODE_FLAG_3D_SIDE_BY_SIDE_FULL), 0);
}

static void drm_test_edid_3d_structure_x(struct kunit *test)
{
	drm_edid_3d_add_modes(test, edid_3d_x, sizeof(edid_3d_x));

	/* L + depth on VIC 19 (720p50), refused on the interlaced VIC 5 */
	KUNIT_EXPECT_EQ(test, drm_edid_3d_count(test, 1280, 720, 50, 74250, false,
						L_DEPTH), 1);
	KUNIT_EXPECT_EQ(test, drm_edid_3d_count_layout(test, L_DEPTH), 1);

	/* line alternative on VIC 16, field alternative on VIC 5 */
	KUNIT_EXPECT_EQ(test, drm_edid_3d_count(test, 1920, 1080, 60, 148500, false,
						LINE_ALT), 1);
	KUNIT_EXPECT_EQ(test, drm_edid_3d_count_layout(test, LINE_ALT), 2);
	KUNIT_EXPECT_EQ(test, drm_edid_3d_count(test, 1920, 1080, 60, 74250, true,
						FIELD_ALT), 1);
	KUNIT_EXPECT_EQ(test, drm_edid_3d_count_layout(test, FIELD_ALT), 2);

	/* L + depth + graphics + graphics-depth on VIC 4 (720p60) */
	KUNIT_EXPECT_EQ(test, drm_edid_3d_count(test, 1280, 720, 60, 74250, false,
						L_DEPTH_GFX), 1);
	KUNIT_EXPECT_EQ(test, drm_edid_3d_count_layout(test, L_DEPTH_GFX), 2);

	/* no 3D_present: no mandatory formats; the reserved value adds nothing */
	KUNIT_EXPECT_EQ(test, drm_edid_3d_count_layout(test,
						       DRM_MODE_FLAG_3D_FRAME_PACKING), 0);
	KUNIT_EXPECT_EQ(test, drm_edid_3d_count_layout(test,
						       DRM_MODE_FLAG_3D_TOP_AND_BOTTOM), 0);
	KUNIT_EXPECT_EQ(test, drm_edid_3d_count_layout(test,
						       DRM_MODE_FLAG_3D_SIDE_BY_SIDE_HALF), 0);
	KUNIT_EXPECT_EQ(test, drm_edid_3d_count_layout(test,
						       DRM_MODE_FLAG_3D_SIDE_BY_SIDE_FULL), 0);
}

static struct kunit_case drm_edid_3d_tests[] = {
	KUNIT_CASE(drm_test_edid_3d_structure_all),
	KUNIT_CASE(drm_test_edid_3d_structure_x),
	{ }
};

static struct kunit_suite drm_edid_3d_test_suite = {
	.name = "drm_edid_hdmi_3d",
	.init = drm_edid_3d_init,
	.test_cases = drm_edid_3d_tests,
};

kunit_test_suite(drm_edid_3d_test_suite);

MODULE_DESCRIPTION("Kunit test for the HDMI 3D structures listed from an EDID");
MODULE_LICENSE("GPL");
