/*******************************************************************************
 * Size: 24 px
 * Bpp: 1
 * Opts: 
 ******************************************************************************/
#define LV_LVGL_H_INCLUDE_SIMPLE

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif

#ifndef ICONS
#define ICONS 1
#endif

#if ICONS

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t glyph_bitmap[] = {
    /* U+E1FF "" */
    0x1e, 0xf, 0xc3, 0x30, 0xdc, 0x33, 0xc, 0xc3,
    0x70, 0xcc, 0x33, 0xf, 0xc3, 0xf1, 0xfe, 0x7f,
    0xbf, 0xff, 0xff, 0xff, 0xff, 0xdf, 0xe7, 0xf8,
    0x78,

    /* U+E425 "" */
    0x3, 0xf0, 0x0, 0xfc, 0x0, 0x0, 0x0, 0xf,
    0xc0, 0xf, 0xfd, 0x87, 0xff, 0xe3, 0xff, 0xf1,
    0xfc, 0xfe, 0x7f, 0x3f, 0xbf, 0xcf, 0xff, 0xf3,
    0xff, 0xfc, 0xff, 0xff, 0x3f, 0xff, 0xff, 0xff,
    0xff, 0xfd, 0xff, 0xfe, 0x7f, 0xff, 0x8f, 0xff,
    0xc1, 0xff, 0xe0, 0x3f, 0xf0, 0x3, 0xf0, 0x0,

    /* U+E8B5 "" */
    0x1, 0xf8, 0x0, 0x7f, 0xe0, 0x1e, 0x7, 0x83,
    0x80, 0x1c, 0x30, 0x0, 0xc6, 0x6, 0x6, 0x60,
    0x60, 0x6c, 0x6, 0x3, 0xc0, 0x60, 0x3c, 0x6,
    0x3, 0xc0, 0x60, 0x3c, 0x3, 0x83, 0xc0, 0xe,
    0x36, 0x0, 0x66, 0x60, 0x0, 0x63, 0x0, 0xc,
    0x38, 0x1, 0xc1, 0xe0, 0x78, 0x7, 0xfe, 0x0,
    0x1f, 0x80
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 384, .box_w = 10, .box_h = 20, .ofs_x = 7, .ofs_y = 2},
    {.bitmap_index = 25, .adv_w = 384, .box_w = 18, .box_h = 21, .ofs_x = 3, .ofs_y = 2},
    {.bitmap_index = 73, .adv_w = 384, .box_w = 20, .box_h = 20, .ofs_x = 2, .ofs_y = 2}
};

/*---------------------
 *  CHARACTER MAPPING
 *--------------------*/

static const uint16_t unicode_list_0[] = {
    0x0, 0x226, 0x6b6
};

/*Collect the unicode lists and glyph_id offsets*/
static const lv_font_fmt_txt_cmap_t cmaps[] =
{
    {
        .range_start = 57855, .range_length = 1719, .glyph_id_start = 1,
        .unicode_list = unicode_list_0, .glyph_id_ofs_list = NULL, .list_length = 3, .type = LV_FONT_FMT_TXT_CMAP_SPARSE_TINY
    }
};



/*--------------------
 *  ALL CUSTOM DATA
 *--------------------*/

#if LV_VERSION_CHECK(8, 0, 0)
/*Store all the custom data of the font*/
static  lv_font_fmt_txt_glyph_cache_t cache;
static const lv_font_fmt_txt_dsc_t font_dsc = {
#else
static lv_font_fmt_txt_dsc_t font_dsc = {
#endif
    .glyph_bitmap = glyph_bitmap,
    .glyph_dsc = glyph_dsc,
    .cmaps = cmaps,
    .kern_dsc = NULL,
    .kern_scale = 0,
    .cmap_num = 1,
    .bpp = 1,
    .kern_classes = 0,
    .bitmap_format = 0,
#if LV_VERSION_CHECK(8, 0, 0)
    .cache = &cache
#endif
};


/*-----------------
 *  PUBLIC FONT
 *----------------*/

/*Initialize a public general font descriptor*/
#if LV_VERSION_CHECK(8, 0, 0)
const lv_font_t icons = {
#else
lv_font_t icons = {
#endif
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,    /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .line_height = 21,          /*The maximum line height required by the font*/
    .base_line = -2,             /*Baseline measured from the bottom of the line*/
#if !(LVGL_VERSION_MAJOR == 6 && LVGL_VERSION_MINOR == 0)
    .subpx = LV_FONT_SUBPX_NONE,
#endif
#if LV_VERSION_CHECK(7, 4, 0) || LVGL_VERSION_MAJOR >= 8
    .underline_position = -6,
    .underline_thickness = 2,
#endif
    .dsc = &font_dsc           /*The custom font data. Will be accessed by `get_glyph_bitmap/dsc` */
};



#endif /*#if ICONS*/

