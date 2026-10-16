// SPDX-License-Identifier: GPL-2.0
/*
 * Kunit test for drm_modes functions
 */

#include <drm/drm_drv.h>
#include <drm/drm_edid.h>
#include <drm/drm_kunit_helpers.h>
#include <drm/drm_modes.h>

#include <kunit/test.h>

#include <linux/units.h>

struct drm_test_modes_priv {
	struct drm_device *drm;
	struct device *dev;
};

static int drm_test_modes_init(struct kunit *test)
{
	struct drm_test_modes_priv *priv;

	priv = kunit_kzalloc(test, sizeof(*priv), GFP_KERNEL);
	KUNIT_ASSERT_NOT_NULL(test, priv);

	priv->dev = drm_kunit_helper_alloc_device(test);
	KUNIT_ASSERT_NOT_ERR_OR_NULL(test, priv->dev);

	priv->drm = __drm_kunit_helper_alloc_drm_device(test, priv->dev,
							sizeof(*priv->drm), 0,
							DRIVER_MODESET);
	KUNIT_ASSERT_NOT_ERR_OR_NULL(test, priv->drm);

	test->priv = priv;

	return 0;
}

static void drm_test_modes_analog_tv_ntsc_480i(struct kunit *test)
{
	struct drm_test_modes_priv *priv = test->priv;
	struct drm_display_mode *mode;
	int ret;

	mode = drm_analog_tv_mode(priv->drm,
				  DRM_MODE_TV_MODE_NTSC,
				  13500 * HZ_PER_KHZ, 720, 480,
				  true);
	KUNIT_ASSERT_NOT_NULL(test, mode);

	ret = drm_kunit_add_mode_destroy_action(test, mode);
	KUNIT_ASSERT_EQ(test, ret, 0);

	KUNIT_EXPECT_EQ(test, drm_mode_vrefresh(mode), 60);
	KUNIT_EXPECT_EQ(test, mode->hdisplay, 720);

	/* BT.601 defines hsync_start at 736 for 480i */
	KUNIT_EXPECT_EQ(test, mode->hsync_start, 736);

	/*
	 * The NTSC standard expects a line to take 63.556us. With a
	 * pixel clock of 13.5 MHz, a pixel takes around 74ns, so we
	 * need to have 63556ns / 74ns = 858.
	 *
	 * This is also mandated by BT.601.
	 */
	KUNIT_EXPECT_EQ(test, mode->htotal, 858);

	KUNIT_EXPECT_EQ(test, mode->vdisplay, 480);
	KUNIT_EXPECT_EQ(test, mode->vtotal, 525);
}

static void drm_test_modes_analog_tv_ntsc_480i_inlined(struct kunit *test)
{
	struct drm_test_modes_priv *priv = test->priv;
	struct drm_display_mode *expected, *mode;
	int ret;

	expected = drm_analog_tv_mode(priv->drm,
				      DRM_MODE_TV_MODE_NTSC,
				      13500 * HZ_PER_KHZ, 720, 480,
				      true);
	KUNIT_ASSERT_NOT_NULL(test, expected);

	ret = drm_kunit_add_mode_destroy_action(test, expected);
	KUNIT_ASSERT_EQ(test, ret, 0);

	mode = drm_mode_analog_ntsc_480i(priv->drm);
	KUNIT_ASSERT_NOT_NULL(test, mode);

	ret = drm_kunit_add_mode_destroy_action(test, mode);
	KUNIT_ASSERT_EQ(test, ret, 0);

	KUNIT_EXPECT_TRUE(test, drm_mode_equal(expected, mode));
}

static void drm_test_modes_analog_tv_pal_576i(struct kunit *test)
{
	struct drm_test_modes_priv *priv = test->priv;
	struct drm_display_mode *mode;
	int ret;

	mode = drm_analog_tv_mode(priv->drm,
				  DRM_MODE_TV_MODE_PAL,
				  13500 * HZ_PER_KHZ, 720, 576,
				  true);
	KUNIT_ASSERT_NOT_NULL(test, mode);

	ret = drm_kunit_add_mode_destroy_action(test, mode);
	KUNIT_ASSERT_EQ(test, ret, 0);

	KUNIT_EXPECT_EQ(test, drm_mode_vrefresh(mode), 50);
	KUNIT_EXPECT_EQ(test, mode->hdisplay, 720);

	/* BT.601 defines hsync_start at 732 for 576i */
	KUNIT_EXPECT_EQ(test, mode->hsync_start, 732);

	/*
	 * The PAL standard expects a line to take 64us. With a pixel
	 * clock of 13.5 MHz, a pixel takes around 74ns, so we need to
	 * have 64000ns / 74ns = 864.
	 *
	 * This is also mandated by BT.601.
	 */
	KUNIT_EXPECT_EQ(test, mode->htotal, 864);

	KUNIT_EXPECT_EQ(test, mode->vdisplay, 576);
	KUNIT_EXPECT_EQ(test, mode->vtotal, 625);
}

static void drm_test_modes_analog_tv_pal_576i_inlined(struct kunit *test)
{
	struct drm_test_modes_priv *priv = test->priv;
	struct drm_display_mode *expected, *mode;
	int ret;

	expected = drm_analog_tv_mode(priv->drm,
				      DRM_MODE_TV_MODE_PAL,
				      13500 * HZ_PER_KHZ, 720, 576,
				      true);
	KUNIT_ASSERT_NOT_NULL(test, expected);

	ret = drm_kunit_add_mode_destroy_action(test, expected);
	KUNIT_ASSERT_EQ(test, ret, 0);

	mode = drm_mode_analog_pal_576i(priv->drm);
	KUNIT_ASSERT_NOT_NULL(test, mode);

	ret = drm_kunit_add_mode_destroy_action(test, mode);
	KUNIT_ASSERT_EQ(test, ret, 0);

	KUNIT_EXPECT_TRUE(test, drm_mode_equal(expected, mode));
}

static void drm_test_modes_analog_tv_mono_576i(struct kunit *test)
{
	struct drm_test_modes_priv *priv = test->priv;
	struct drm_display_mode *mode;
	int ret;

	mode = drm_analog_tv_mode(priv->drm,
				  DRM_MODE_TV_MODE_MONOCHROME,
				  13500 * HZ_PER_KHZ, 720, 576,
				  true);
	KUNIT_ASSERT_NOT_NULL(test, mode);

	ret = drm_kunit_add_mode_destroy_action(test, mode);
	KUNIT_ASSERT_EQ(test, ret, 0);

	KUNIT_EXPECT_EQ(test, drm_mode_vrefresh(mode), 50);
	KUNIT_EXPECT_EQ(test, mode->hdisplay, 720);

	/* BT.601 defines hsync_start at 732 for 576i */
	KUNIT_EXPECT_EQ(test, mode->hsync_start, 732);

	/*
	 * The PAL standard expects a line to take 64us. With a pixel
	 * clock of 13.5 MHz, a pixel takes around 74ns, so we need to
	 * have 64000ns / 74ns = 864.
	 *
	 * This is also mandated by BT.601.
	 */
	KUNIT_EXPECT_EQ(test, mode->htotal, 864);

	KUNIT_EXPECT_EQ(test, mode->vdisplay, 576);
	KUNIT_EXPECT_EQ(test, mode->vtotal, 625);
}

static struct kunit_case drm_modes_analog_tv_tests[] = {
	KUNIT_CASE(drm_test_modes_analog_tv_mono_576i),
	KUNIT_CASE(drm_test_modes_analog_tv_ntsc_480i),
	KUNIT_CASE(drm_test_modes_analog_tv_ntsc_480i_inlined),
	KUNIT_CASE(drm_test_modes_analog_tv_pal_576i),
	KUNIT_CASE(drm_test_modes_analog_tv_pal_576i_inlined),
	{ }
};

static struct kunit_suite drm_modes_analog_tv_test_suite = {
	.name = "drm_modes_analog_tv",
	.init = drm_test_modes_init,
	.test_cases = drm_modes_analog_tv_tests,
};

kunit_test_suite(drm_modes_analog_tv_test_suite);

struct drm_modes_stereo_test {
	const char *name;
	u8 vic;
	u32 layout;
	int adjust;
	int clock;
	int hdisplay, hsync_start, hsync_end, htotal;
	int vdisplay, vsync_start, vsync_end, vtotal;
};

/*
 * The 3D frame of each HDMI 1.4a 3D structure, from the CTA-861 timing of
 * one of the formats Table H-5 gives as an example (or, for frame packing,
 * a mandatory format of section 8.2.3.2): Figures 8-3 and H-2 to H-6.
 */
static const struct drm_modes_stereo_test drm_modes_stereo_tests[] = {
	{ "frame-packing-1080p24", 32, DRM_MODE_FLAG_3D_FRAME_PACKING, 0,
	  148500, 1920, 2558, 2602, 2750, 2205, 2209, 2214, 2250 },
	{ "field-alternative-1080i60", 5, DRM_MODE_FLAG_3D_FIELD_ALTERNATIVE, 0,
	  148500, 1920, 2008, 2052, 2200, 2205, 2209, 2219, 2250 },
	{ "line-alternative-1080p60", 16, DRM_MODE_FLAG_3D_LINE_ALTERNATIVE, 0,
	  297000, 1920, 2008, 2052, 2200, 2160, 2168, 2178, 2250 },
	{ "side-by-side-full-1080p60", 16, DRM_MODE_FLAG_3D_SIDE_BY_SIDE_FULL, 0,
	  297000, 3840, 4016, 4104, 4400, 1080, 1084, 1089, 1125 },
	{ "side-by-side-full-keep-hblank-1080p60", 16,
	  DRM_MODE_FLAG_3D_SIDE_BY_SIDE_FULL, CRTC_STEREO_KEEP_HBLANK,
	  278100, 3840, 3928, 3972, 4120, 1080, 1084, 1089, 1125 },
	{ "l-depth-720p50", 19, DRM_MODE_FLAG_3D_L_DEPTH, 0,
	  148500, 1280, 1720, 1760, 1980, 1470, 1475, 1480, 1500 },
	{ "l-depth-gfx-gfx-depth-720p50", 19,
	  DRM_MODE_FLAG_3D_L_DEPTH_GFX_GFX_DEPTH, 0,
	  297000, 1280, 1720, 1760, 1980, 2970, 2975, 2980, 3000 },
	{ "top-and-bottom-1080p60", 16, DRM_MODE_FLAG_3D_TOP_AND_BOTTOM, 0,
	  148500, 1920, 2008, 2052, 2200, 1080, 1084, 1089, 1125 },
	{ "side-by-side-half-1080p60", 16, DRM_MODE_FLAG_3D_SIDE_BY_SIDE_HALF, 0,
	  148500, 1920, 2008, 2052, 2200, 1080, 1084, 1089, 1125 },
};

static void drm_test_modes_stereo_desc(const struct drm_modes_stereo_test *t,
				       char *desc)
{
	strscpy(desc, t->name, KUNIT_PARAM_DESC_SIZE);
}

KUNIT_ARRAY_PARAM(drm_modes_stereo, drm_modes_stereo_tests,
		  drm_test_modes_stereo_desc);

static void drm_test_modes_stereo_crtcinfo(struct kunit *test)
{
	struct drm_test_modes_priv *priv = test->priv;
	const struct drm_modes_stereo_test *t = test->param_value;
	struct drm_display_mode *mode;
	int hdisplay, vdisplay;

	mode = drm_display_mode_from_cea_vic(priv->drm, t->vic);
	KUNIT_ASSERT_NOT_NULL(test, mode);
	mode->flags |= t->layout;

	drm_mode_set_crtcinfo(mode, CRTC_STEREO_DOUBLE_ONLY | t->adjust);

	KUNIT_EXPECT_EQ(test, mode->crtc_clock, t->clock);
	KUNIT_EXPECT_EQ(test, mode->crtc_hdisplay, t->hdisplay);
	KUNIT_EXPECT_EQ(test, mode->crtc_hsync_start, t->hsync_start);
	KUNIT_EXPECT_EQ(test, mode->crtc_hsync_end, t->hsync_end);
	KUNIT_EXPECT_EQ(test, mode->crtc_htotal, t->htotal);
	KUNIT_EXPECT_EQ(test, mode->crtc_vdisplay, t->vdisplay);
	KUNIT_EXPECT_EQ(test, mode->crtc_vsync_start, t->vsync_start);
	KUNIT_EXPECT_EQ(test, mode->crtc_vsync_end, t->vsync_end);
	KUNIT_EXPECT_EQ(test, mode->crtc_vtotal, t->vtotal);

	/* the buffer holds the whole 3D frame, whatever the blanking */
	drm_mode_get_hv_timing(mode, &hdisplay, &vdisplay);
	KUNIT_EXPECT_EQ(test, hdisplay, t->hdisplay);
	KUNIT_EXPECT_EQ(test, vdisplay, t->vdisplay);

	/* mode validation uses the specification's timing */
	if (!t->adjust)
		KUNIT_EXPECT_EQ(test, drm_mode_stereo_clock(mode), t->clock);
	else
		KUNIT_EXPECT_GT(test, drm_mode_stereo_clock(mode), t->clock);
}

static struct kunit_case drm_modes_stereo_cases[] = {
	KUNIT_CASE_PARAM(drm_test_modes_stereo_crtcinfo,
			 drm_modes_stereo_gen_params),
	{ }
};

static struct kunit_suite drm_modes_stereo_test_suite = {
	.name = "drm_modes_stereo",
	.init = drm_test_modes_init,
	.test_cases = drm_modes_stereo_cases,
};

kunit_test_suite(drm_modes_stereo_test_suite);

MODULE_AUTHOR("Maxime Ripard <maxime@cerno.tech>");
MODULE_DESCRIPTION("Kunit test for drm_modes functions");
MODULE_LICENSE("GPL");
