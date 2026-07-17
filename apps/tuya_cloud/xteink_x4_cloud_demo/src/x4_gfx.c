/**
 * @file x4_gfx.c
 * @brief Lightweight 1bpp framebuffer renderer for XTEINK X4 EPD.
 * @version 1.0
 * @date 2026-07-16
 * @copyright Copyright (c) 2026 Tuya Inc. All Rights Reserved.
 */
#include "tuya_cloud_types.h"
#include "x4_gfx.h"

#include <string.h>

/* ---------------------------------------------------------------------------
 * Macros
 * --------------------------------------------------------------------------- */
#define X4_GFX_FONT_FIRST 32
#define X4_GFX_FONT_LAST  126
#define X4_GFX_FONT_COUNT (X4_GFX_FONT_LAST - X4_GFX_FONT_FIRST + 1)
#define X4_GFX_FONT_W     8
#define X4_GFX_FONT_H     16
#define X4_GFX_CHAR_ADV   8

#include "x4_font_8x16.inc"

/* ---------------------------------------------------------------------------
 * Function implementations
 * --------------------------------------------------------------------------- */
/**
 * @brief Bind renderer to a 1bpp MSB-first framebuffer.
 * @param[out] gfx renderer context
 * @param[in] fb framebuffer pointer
 * @param[in] width panel width in pixels
 * @param[in] height panel height in pixels
 * @return none
 */
void x4_gfx_init(X4_GFX_T *gfx, uint8_t *fb, int32_t width, int32_t height)
{
    if (gfx == NULL) {
        return;
    }

    gfx->fb     = fb;
    gfx->width  = width;
    gfx->height = height;
    gfx->stride = width / 8;
}

/**
 * @brief Fill framebuffer with white or black.
 * @param[in] gfx renderer context
 * @param[in] white true for white (1), false for black (0)
 * @return none
 */
void x4_gfx_clear(X4_GFX_T *gfx, BOOL_T white)
{
    size_t bytes;

    if ((gfx == NULL) || (gfx->fb == NULL)) {
        return;
    }

    bytes = (size_t)gfx->stride * (size_t)gfx->height;
    (void)memset(gfx->fb, white ? 0xFF : 0x00, bytes);
}

/**
 * @brief Set one pixel.
 * @param[in] gfx renderer context
 * @param[in] x horizontal coordinate
 * @param[in] y vertical coordinate
 * @param[in] white true for white, false for black
 * @return none
 */
void x4_gfx_set_pixel(X4_GFX_T *gfx, int32_t x, int32_t y, BOOL_T white)
{
    uint32_t off;
    uint8_t  mask;

    if ((gfx == NULL) || (gfx->fb == NULL)) {
        return;
    }
    if ((x < 0) || (x >= gfx->width) || (y < 0) || (y >= gfx->height)) {
        return;
    }

    off  = (uint32_t)y * (uint32_t)gfx->stride + (uint32_t)x / 8U;
    mask = (uint8_t)(0x80U >> (unsigned)(x % 8));
    if (white) {
        gfx->fb[off] |= mask;
    } else {
        gfx->fb[off] = (uint8_t)(gfx->fb[off] & (uint8_t)~mask);
    }
}

/**
 * @brief Fill axis-aligned rectangle.
 * @param[in] gfx renderer context
 * @param[in] x left coordinate
 * @param[in] y top coordinate
 * @param[in] w width in pixels
 * @param[in] h height in pixels
 * @param[in] white fill color
 * @return none
 */
void x4_gfx_fill_rect(X4_GFX_T *gfx, int32_t x, int32_t y, int32_t w, int32_t h, BOOL_T white)
{
    int32_t row;
    int32_t col;

    if ((gfx == NULL) || (w <= 0) || (h <= 0)) {
        return;
    }

    for (row = 0; row < h; row++) {
        for (col = 0; col < w; col++) {
            x4_gfx_set_pixel(gfx, x + col, y + row, white);
        }
    }
}

/**
 * @brief Draw rectangle outline.
 * @param[in] gfx renderer context
 * @param[in] x left coordinate
 * @param[in] y top coordinate
 * @param[in] w width in pixels
 * @param[in] h height in pixels
 * @param[in] line_w line width in pixels
 * @param[in] white line color
 * @return none
 */
void x4_gfx_draw_rect(X4_GFX_T *gfx, int32_t x, int32_t y, int32_t w, int32_t h, int32_t line_w, BOOL_T white)
{
    int32_t i;

    if ((gfx == NULL) || (w <= 0) || (h <= 0) || (line_w <= 0)) {
        return;
    }

    for (i = 0; i < line_w; i++) {
        x4_gfx_fill_rect(gfx, x + i, y + i, w - 2 * i, 1, white);
        x4_gfx_fill_rect(gfx, x + i, y + h - 1 - i, w - 2 * i, 1, white);
        x4_gfx_fill_rect(gfx, x + i, y + i, 1, h - 2 * i, white);
        x4_gfx_fill_rect(gfx, x + w - 1 - i, y + i, 1, h - 2 * i, white);
    }
}

/**
 * @brief Get line height for the built-in font.
 * @return line height in pixels
 */
int32_t x4_gfx_line_height(void)
{
    return X4_GFX_FONT_H;
}

/**
 * @brief Measure text width in pixels.
 * @param[in] text string to measure
 * @return width in pixels
 */
int32_t x4_gfx_text_width(const char *text)
{
    size_t len;

    if (text == NULL) {
        return 0;
    }

    len = strlen(text);
    return (int32_t)len * X4_GFX_CHAR_ADV;
}

/**
 * @brief Draw one glyph at pixel position.
 * @param[in] gfx renderer context
 * @param[in] x left coordinate
 * @param[in] y top coordinate
 * @param[in] ch character code
 * @param[in] white text color
 * @return none
 */
static void __draw_char(X4_GFX_T *gfx, int32_t x, int32_t y, char ch, BOOL_T white)
{
    uint32_t idx;
    int32_t  row;
    int32_t  col;
    uint8_t  bits;

    if ((gfx == NULL) || ((uint8_t)ch < X4_GFX_FONT_FIRST) || ((uint8_t)ch > X4_GFX_FONT_LAST)) {
        ch = '?';
    }

    idx = (uint32_t)((uint8_t)ch - X4_GFX_FONT_FIRST);
    if (idx >= X4_GFX_FONT_COUNT) {
        return;
    }

    for (row = 0; row < X4_GFX_FONT_H; row++) {
        bits = s_font8x16[idx][row];
        for (col = 0; col < X4_GFX_FONT_W; col++) {
            BOOL_T on = ((bits & (uint8_t)(0x80U >> (unsigned)col)) != 0U) ? TRUE : FALSE;
            if (on) {
                x4_gfx_set_pixel(gfx, x + col, y + row, white ? FALSE : TRUE);
            }
        }
    }
}

/**
 * @brief Draw 8x16 ASCII text (printable 32..126).
 * @param[in] gfx renderer context
 * @param[in] x left baseline origin
 * @param[in] y top of glyph box
 * @param[in] text UTF-8/ASCII string
 * @param[in] white text color (false = black on white)
 * @return none
 */
void x4_gfx_draw_text(X4_GFX_T *gfx, int32_t x, int32_t y, const char *text, BOOL_T white)
{
    int32_t cx;

    if ((gfx == NULL) || (text == NULL)) {
        return;
    }

    cx = x;
    while (*text != '\0') {
        __draw_char(gfx, cx, y, *text, white);
        cx += X4_GFX_CHAR_ADV;
        text++;
    }
}

/**
 * @brief Draw wrapped text within max width.
 * @param[in] gfx renderer context
 * @param[in] x left coordinate
 * @param[in] y top coordinate
 * @param[in] max_w maximum line width in pixels
 * @param[in] text string to draw
 * @param[in] white text color
 * @return none
 */
void x4_gfx_draw_text_wrap(X4_GFX_T *gfx, int32_t x, int32_t y, int32_t max_w, const char *text, BOOL_T white)
{
    int32_t     cx;
    int32_t     cy;
    int32_t     line_w;
    const char *p;
    const char *line_start;

    if ((gfx == NULL) || (text == NULL) || (max_w <= 0)) {
        return;
    }

    cx         = x;
    cy         = y;
    line_start = text;
    line_w     = 0;
    p          = text;

    while (*p != '\0') {
        if (*p == '\n') {
            x4_gfx_draw_text(gfx, cx, cy, line_start, white);
            cy += X4_GFX_FONT_H;
            p++;
            line_start = p;
            line_w     = 0;
            continue;
        }

        line_w += X4_GFX_CHAR_ADV;
        if (line_w > max_w) {
            size_t len = (size_t)(p - line_start);
            char buf[96];

            if (len >= sizeof(buf)) {
                len = sizeof(buf) - 1U;
            }
            (void)memcpy(buf, line_start, len);
            buf[len] = '\0';
            x4_gfx_draw_text(gfx, cx, cy, buf, white);
            cy += X4_GFX_FONT_H;
            line_start = p;
            line_w     = X4_GFX_CHAR_ADV;
        }
        p++;
    }

    if (line_start < p) {
        x4_gfx_draw_text(gfx, cx, cy, line_start, white);
    }
}
