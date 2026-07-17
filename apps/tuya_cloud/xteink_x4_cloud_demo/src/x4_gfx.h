/**
 * @file x4_gfx.h
 * @brief Lightweight 1bpp framebuffer renderer for XTEINK X4 EPD (GfxRenderer-style).
 * @version 1.0
 * @date 2026-07-16
 * @copyright Copyright (c) 2026 Tuya Inc. All Rights Reserved.
 */
#ifndef __X4_GFX_H__
#define __X4_GFX_H__

#include "tuya_cloud_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ---------------------------------------------------------------------------
 * Type definitions
 * --------------------------------------------------------------------------- */
typedef struct {
    uint8_t *fb;
    int32_t  width;
    int32_t  height;
    int32_t  stride;
} X4_GFX_T;

/* ---------------------------------------------------------------------------
 * Function declarations
 * --------------------------------------------------------------------------- */
/**
 * @brief Bind renderer to a 1bpp MSB-first framebuffer.
 * @param[out] gfx renderer context
 * @param[in] fb framebuffer pointer
 * @param[in] width panel width in pixels
 * @param[in] height panel height in pixels
 * @return none
 */
void x4_gfx_init(X4_GFX_T *gfx, uint8_t *fb, int32_t width, int32_t height);

/**
 * @brief Fill framebuffer with white or black.
 * @param[in] gfx renderer context
 * @param[in] white true for white (1), false for black (0)
 * @return none
 */
void x4_gfx_clear(X4_GFX_T *gfx, BOOL_T white);

/**
 * @brief Set one pixel.
 * @param[in] gfx renderer context
 * @param[in] x horizontal coordinate
 * @param[in] y vertical coordinate
 * @param[in] white true for white, false for black
 * @return none
 */
void x4_gfx_set_pixel(X4_GFX_T *gfx, int32_t x, int32_t y, BOOL_T white);

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
void x4_gfx_fill_rect(X4_GFX_T *gfx, int32_t x, int32_t y, int32_t w, int32_t h, BOOL_T white);

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
void x4_gfx_draw_rect(X4_GFX_T *gfx, int32_t x, int32_t y, int32_t w, int32_t h, int32_t line_w, BOOL_T white);

/**
 * @brief Fill a circle.
 * @param[in] gfx renderer context
 * @param[in] cx center x
 * @param[in] cy center y
 * @param[in] r radius in pixels
 * @param[in] white fill color
 * @return none
 */
void x4_gfx_fill_circle(X4_GFX_T *gfx, int32_t cx, int32_t cy, int32_t r, BOOL_T white);

/**
 * @brief Draw circle outline.
 * @param[in] gfx renderer context
 * @param[in] cx center x
 * @param[in] cy center y
 * @param[in] r radius in pixels
 * @param[in] line_w stroke width in pixels
 * @param[in] white line color
 * @return none
 */
void x4_gfx_draw_circle(X4_GFX_T *gfx, int32_t cx, int32_t cy, int32_t r, int32_t line_w, BOOL_T white);

/**
 * @brief Draw 8x16 ASCII text (printable 32..126).
 * @param[in] gfx renderer context
 * @param[in] x left baseline origin
 * @param[in] y top of glyph box
 * @param[in] text UTF-8/ASCII string
 * @param[in] white text color (false = black on white)
 * @return none
 */
void x4_gfx_draw_text(X4_GFX_T *gfx, int32_t x, int32_t y, const char *text, BOOL_T white);

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
void x4_gfx_draw_text_wrap(X4_GFX_T *gfx, int32_t x, int32_t y, int32_t max_w, const char *text, BOOL_T white);

/**
 * @brief Measure text width in pixels.
 * @param[in] text string to measure
 * @return width in pixels
 */
int32_t x4_gfx_text_width(const char *text);

/**
 * @brief Get line height for the built-in font.
 * @return line height in pixels
 */
int32_t x4_gfx_line_height(void);

#ifdef __cplusplus
}
#endif

#endif /* __X4_GFX_H__ */
