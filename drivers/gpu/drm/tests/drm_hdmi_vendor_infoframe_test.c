// SPDX-License-Identifier: GPL-2.0
/*
 * Kunit test for the 3D fields of the HDMI vendor InfoFrame
 */

#include <linux/hdmi.h>

#include <kunit/test.h>

/*
 * Parallax information as ISO/IEC 23002-3 orders it and HDMI 1.4a, Appendix
 * H, Table H-4 sends it: parallax_zero 128, parallax_scale 256, dref 200 cm,
 * wref 100 cm, most significant byte first.
 */
static const u8 parallax[HDMI_3D_METADATA_PARALLAX_SIZE] = {
	0x00, 0x80, 0x01, 0x00, 0x00, 0xc8, 0x00, 0x64,
};

static void set_parallax(struct hdmi_vendor_infoframe *frame)
{
	frame->s3d_meta_present = true;
	frame->s3d_meta_type = HDMI_3D_METADATA_PARALLAX;
	frame->s3d_meta_length = sizeof(parallax);
	memcpy(frame->s3d_meta, parallax, sizeof(parallax));
}

static u8 checksum(const u8 *buf, size_t len)
{
	u8 sum = 0;

	while (len--)
		sum += *buf++;

	return sum;
}

/* pack @frame, check the body against @body, unpack it and compare */
static void pack_and_unpack(struct kunit *test,
			    struct hdmi_vendor_infoframe *frame,
			    const u8 *body, size_t body_len)
{
	u8 buf[HDMI_INFOFRAME_HEADER_SIZE + HDMI_MAX_INFOFRAME_SIZE];
	union hdmi_infoframe unpacked;
	struct hdmi_vendor_infoframe *out = &unpacked.vendor.hdmi;
	ssize_t len;
	int ret;

	len = hdmi_vendor_infoframe_pack(frame, buf, sizeof(buf));
	KUNIT_ASSERT_EQ(test, len, HDMI_INFOFRAME_HEADER_SIZE + body_len);

	KUNIT_EXPECT_EQ(test, buf[0], HDMI_INFOFRAME_TYPE_VENDOR);
	KUNIT_EXPECT_EQ(test, buf[1], 1);
	KUNIT_EXPECT_EQ(test, buf[2], body_len);
	KUNIT_EXPECT_EQ(test, checksum(buf, len), 0);
	KUNIT_EXPECT_MEMEQ(test, &buf[HDMI_INFOFRAME_HEADER_SIZE], body, body_len);

	ret = hdmi_infoframe_unpack(&unpacked, buf, len);
	KUNIT_ASSERT_EQ(test, ret, 0);
	KUNIT_EXPECT_EQ(test, out->s3d_struct, frame->s3d_struct);
	KUNIT_EXPECT_EQ(test, out->s3d_ext_data, frame->s3d_ext_data);
	KUNIT_EXPECT_EQ(test, out->s3d_meta_present, frame->s3d_meta_present);
	if (frame->s3d_meta_present) {
		KUNIT_EXPECT_EQ(test, out->s3d_meta_type, frame->s3d_meta_type);
		KUNIT_EXPECT_EQ(test, out->s3d_meta_length, frame->s3d_meta_length);
		KUNIT_EXPECT_MEMEQ(test, out->s3d_meta, frame->s3d_meta,
				   frame->s3d_meta_length);
	}
}

/* L + depth: 3D_Structure 0100 with 3D_Meta_present, metadata from PB6 */
static void hdmi_test_vendor_infoframe_l_depth_parallax(struct kunit *test)
{
	static const u8 body[] = {
		0x03, 0x0c, 0x00,	/* IEEE OUI 0x000c03 */
		0x40,			/* HDMI_Video_Format: 3D */
		0x48,			/* 3D_Structure 0100, 3D_Meta_present */
		0x08,			/* 3D_Metadata_type 000, length 8 */
		0x00, 0x80, 0x01, 0x00, 0x00, 0xc8, 0x00, 0x64,
	};
	struct hdmi_vendor_infoframe frame;

	KUNIT_ASSERT_EQ(test, hdmi_vendor_infoframe_init(&frame), 0);
	frame.s3d_struct = HDMI_3D_STRUCTURE_L_DEPTH;
	set_parallax(&frame);

	pack_and_unpack(test, &frame, body, sizeof(body));
}

/* the four-region structure has no 3D_Ext_Data either */
static void hdmi_test_vendor_infoframe_l_depth_gfx_parallax(struct kunit *test)
{
	static const u8 body[] = {
		0x03, 0x0c, 0x00, 0x40,
		0x58,			/* 3D_Structure 0101, 3D_Meta_present */
		0x08,
		0x00, 0x80, 0x01, 0x00, 0x00, 0xc8, 0x00, 0x64,
	};
	struct hdmi_vendor_infoframe frame;

	KUNIT_ASSERT_EQ(test, hdmi_vendor_infoframe_init(&frame), 0);
	frame.s3d_struct = HDMI_3D_STRUCTURE_L_DEPTH_GFX_GFX_DEPTH;
	set_parallax(&frame);

	pack_and_unpack(test, &frame, body, sizeof(body));
}

/* side by side (half): the metadata follows 3D_Ext_Data, at PB7 */
static void hdmi_test_vendor_infoframe_sbs_half_parallax(struct kunit *test)
{
	static const u8 body[] = {
		0x03, 0x0c, 0x00, 0x40,
		0x88,			/* 3D_Structure 1000, 3D_Meta_present */
		0x00,			/* 3D_Ext_Data: horizontal sub-sampling */
		0x08,
		0x00, 0x80, 0x01, 0x00, 0x00, 0xc8, 0x00, 0x64,
	};
	struct hdmi_vendor_infoframe frame;

	KUNIT_ASSERT_EQ(test, hdmi_vendor_infoframe_init(&frame), 0);
	frame.s3d_struct = HDMI_3D_STRUCTURE_SIDE_BY_SIDE_HALF;
	set_parallax(&frame);

	pack_and_unpack(test, &frame, body, sizeof(body));
}

/* top and bottom: the metadata takes PB6, where the zero byte would be */
static void hdmi_test_vendor_infoframe_top_and_bottom_parallax(struct kunit *test)
{
	static const u8 body[] = {
		0x03, 0x0c, 0x00, 0x40,
		0x68,			/* 3D_Structure 0110, 3D_Meta_present */
		0x08,
		0x00, 0x80, 0x01, 0x00, 0x00, 0xc8, 0x00, 0x64,
	};
	struct hdmi_vendor_infoframe frame;

	KUNIT_ASSERT_EQ(test, hdmi_vendor_infoframe_init(&frame), 0);
	frame.s3d_struct = HDMI_3D_STRUCTURE_TOP_AND_BOTTOM;
	set_parallax(&frame);

	pack_and_unpack(test, &frame, body, sizeof(body));
}

/* without metadata, top and bottom keeps its 6-byte body */
static void hdmi_test_vendor_infoframe_top_and_bottom(struct kunit *test)
{
	static const u8 body[] = { 0x03, 0x0c, 0x00, 0x40, 0x60, 0x00 };
	struct hdmi_vendor_infoframe frame;

	KUNIT_ASSERT_EQ(test, hdmi_vendor_infoframe_init(&frame), 0);
	frame.s3d_struct = HDMI_3D_STRUCTURE_TOP_AND_BOTTOM;

	pack_and_unpack(test, &frame, body, sizeof(body));
}

/* and frame packing its 5-byte one */
static void hdmi_test_vendor_infoframe_frame_packing(struct kunit *test)
{
	static const u8 body[] = { 0x03, 0x0c, 0x00, 0x40, 0x00 };
	struct hdmi_vendor_infoframe frame;

	KUNIT_ASSERT_EQ(test, hdmi_vendor_infoframe_init(&frame), 0);
	frame.s3d_struct = HDMI_3D_STRUCTURE_FRAME_PACKING;

	pack_and_unpack(test, &frame, body, sizeof(body));
}

static void hdmi_test_vendor_infoframe_bad_metadata(struct kunit *test)
{
	struct hdmi_vendor_infoframe frame;

	/* Table H-4: parallax information is 8 bytes */
	KUNIT_ASSERT_EQ(test, hdmi_vendor_infoframe_init(&frame), 0);
	frame.s3d_struct = HDMI_3D_STRUCTURE_L_DEPTH;
	set_parallax(&frame);
	frame.s3d_meta_length = 7;
	KUNIT_EXPECT_EQ(test, hdmi_vendor_infoframe_check(&frame), -EINVAL);

	/* no metadata without a 3D structure */
	KUNIT_ASSERT_EQ(test, hdmi_vendor_infoframe_init(&frame), 0);
	set_parallax(&frame);
	KUNIT_EXPECT_EQ(test, hdmi_vendor_infoframe_check(&frame), -EINVAL);

	/* nor more than the InfoFrame holds */
	KUNIT_ASSERT_EQ(test, hdmi_vendor_infoframe_init(&frame), 0);
	frame.s3d_struct = HDMI_3D_STRUCTURE_L_DEPTH;
	frame.s3d_meta_present = true;
	frame.s3d_meta_type = 1;
	frame.s3d_meta_length = HDMI_3D_METADATA_MAX_SIZE + 1;
	KUNIT_EXPECT_EQ(test, hdmi_vendor_infoframe_check(&frame), -EINVAL);
}

/* a metadata length that runs past the received body is refused */
static void hdmi_test_vendor_infoframe_unpack_short(struct kunit *test)
{
	u8 buf[] = {
		0x81, 0x01, 0x0a, 0x00,
		0x03, 0x0c, 0x00, 0x40, 0x48, 0x08,
		0x00, 0x80, 0x01, 0x00,
	};
	union hdmi_infoframe unpacked;

	buf[3] = -checksum(buf, sizeof(buf));
	KUNIT_EXPECT_EQ(test, hdmi_infoframe_unpack(&unpacked, buf, sizeof(buf)),
			-EINVAL);
}

static struct kunit_case hdmi_vendor_infoframe_tests[] = {
	KUNIT_CASE(hdmi_test_vendor_infoframe_l_depth_parallax),
	KUNIT_CASE(hdmi_test_vendor_infoframe_l_depth_gfx_parallax),
	KUNIT_CASE(hdmi_test_vendor_infoframe_sbs_half_parallax),
	KUNIT_CASE(hdmi_test_vendor_infoframe_top_and_bottom_parallax),
	KUNIT_CASE(hdmi_test_vendor_infoframe_top_and_bottom),
	KUNIT_CASE(hdmi_test_vendor_infoframe_frame_packing),
	KUNIT_CASE(hdmi_test_vendor_infoframe_bad_metadata),
	KUNIT_CASE(hdmi_test_vendor_infoframe_unpack_short),
	{ }
};

static struct kunit_suite hdmi_vendor_infoframe_test_suite = {
	.name = "hdmi_vendor_infoframe_3d",
	.test_cases = hdmi_vendor_infoframe_tests,
};

kunit_test_suite(hdmi_vendor_infoframe_test_suite);

MODULE_DESCRIPTION("Kunit test for the 3D fields of the HDMI vendor InfoFrame");
MODULE_LICENSE("GPL");
