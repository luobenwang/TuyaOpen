/**
 * @file xteink_x4_display.c
 * @brief X4 lightweight 1bpp EPD UI (GfxRenderer-style, no LVGL heap).
 * @version 2.0
 * @date 2026-07-16
 * @copyright Copyright (c) 2026 Tuya Inc. All Rights Reserved.
 */
#include "tuya_cloud_types.h"

#include "tal_api.h"
#include "tal_system.h"
#include "tkl_output.h"

#include "board_com_api.h"
#include "board_config.h"
#include "tuya_config.h"
#include "x4_gfx.h"
#include "xteink_x4_buttons.h"
#include "xteink_x4_display.h"

#include <stdio.h>
#include <string.h>

/* ---------------------------------------------------------------------------
 * Macros
 * --------------------------------------------------------------------------- */
#define X4_PWR_HOLD_MS 3000U

#define X4_PWR_OFF_FLASH_UPDATES  5U
#define X4_PWR_OFF_FLASH_PAUSE_MS 240U

#define X4_EPD_W        ((int32_t)X4_EPD_WIDTH)
#define X4_EPD_H        ((int32_t)X4_EPD_HEIGHT)
#define X4_EPD_STRIDE   (X4_EPD_W / 8U)
#define X4_EPD_BUF_SIZE (X4_EPD_STRIDE * X4_EPD_H)

#define X4_SPLASH_HOLD_MS    2800U
#define X4_GRAY16_HOLD_MS    1000U
#define X4_BULLSEYE_HOLD_MS  800U

#define X4_RENDER_W ((int32_t)X4_PANEL_VIEWABLE_WIDTH)
#define X4_RENDER_H ((int32_t)X4_PANEL_VIEWABLE_HEIGHT)
#define X4_ORIGIN_X ((int32_t)X4_PANEL_VIEWABLE_LEFT_PX)
#define X4_ORIGIN_Y ((int32_t)X4_PANEL_VIEWABLE_TOP_PX)

#define X4_BAR_H   84
#define X4_FOOT_H  64
#define X4_QUAD_GAP 3
#define X4_MID_H   (X4_RENDER_H - X4_BAR_H - X4_FOOT_H)
#define X4_QUAD_H  ((X4_MID_H - X4_QUAD_GAP) / 2)
#define X4_QUAD_W  ((X4_RENDER_W - X4_QUAD_GAP) / 2)

#define X4_INPUT_POLL_MS 40U
#define X4_EPD_PUSH_MS   100U
#define X4_HUB_SLOW_N    5U

#if XTEINK_X4_ENABLE_CLOUD
/* Scheme E: equal-diameter circles, top 4 / bottom 3, shared pitch module. */
#define X4_KEY_D         96
#define X4_KEY_R         (X4_KEY_D / 2)
#define X4_KEY_PITCH     124
#define X4_KEY_ROW_GAP   40
#define X4_KEY_STROKE    2
#define X4_KEY_TITLE_H   64
#define X4_KEY_HINT_H    48
#define X4_KEY_GRID_W    (X4_KEY_D + 3 * X4_KEY_PITCH)
#define X4_KEY_GRID_H    (X4_KEY_D + X4_KEY_ROW_GAP + X4_KEY_D)
#define X4_KEY_PAD       8
#define X4_KEYS_MAX_H    (X4_KEY_GRID_H + 2 * X4_KEY_PAD)
#define X4_KEYS_OLD_MAX  (X4_EPD_STRIDE * (X4_KEYS_MAX_H + 16))
#endif

/* ---------------------------------------------------------------------------
 * File scope variables
 * --------------------------------------------------------------------------- */
static uint8_t         s_epd_fb[X4_EPD_BUF_SIZE];
static X4_GFX_T        s_gfx;
static volatile BOOL_T   s_epd_dirty;
static uint32_t        s_boot_ms;
static BOOL_T          s_sd_mounted;
static BOOL_T          s_power_off_started;
static uint32_t        s_pwr_hold_ms;
static uint8_t         s_hub_slow_tick;
static uint8_t         s_last_key_st = 0xFFU; /* force first key paint */
#if XTEINK_X4_ENABLE_CLOUD
static uint8_t         s_keys_old_region[X4_KEYS_OLD_MAX];
#endif
static char            s_sd_smoke_msg[192];
static char            s_cloud_status[96] = "Cloud: idle";
static THREAD_HANDLE   s_display_thread   = NULL;
static SEM_HANDLE      s_display_ready_sem = NULL;
static volatile BOOL_T s_display_ready     = FALSE;

/**
 * @brief Signal that EPD init and first frame are complete.
 * @return none
 */
static void __signal_display_ready(void)
{
    s_display_ready = TRUE;
    if (s_display_ready_sem != NULL) {
        (void)tal_semaphore_post(s_display_ready_sem);
    }
}

/* ---------------------------------------------------------------------------
 * Forward declarations
 * --------------------------------------------------------------------------- */
static void __epd_push_if_dirty(BOOL_T full_refresh);
static void __build_dashboard(void);
static void __build_keys_test_screen(void);
static void __build_splash_screen(void);
static void __dashboard_refresh_slow(void);
static void __draw_quad_frame(int32_t x, int32_t y, const char *title);
static void __draw_key_chip(int32_t x, int32_t y, const char *label, BOOL_T pressed);
static void __refresh_keys_quadrant(uint8_t st);
#if XTEINK_X4_ENABLE_CLOUD
static void __keys_grid_origin(int32_t *ox, int32_t *oy);
static void __keys_region_get(uint16_t *x, uint16_t *y, uint16_t *w, uint16_t *h);
#endif

/**
 * @brief Mark framebuffer dirty for next EPD push.
 * @return none
 */
static void __mark_dirty(void)
{
    s_epd_dirty = TRUE;
}

/**
 * @brief Push framebuffer to EPD when dirty.
 * @param[in] full_refresh use full refresh mode
 * @return none
 */
static void __epd_push_if_dirty(BOOL_T full_refresh)
{
    if (!s_epd_dirty) {
        return;
    }

    if (full_refresh) {
        (void)board_x4_epd_display_full_refresh(s_epd_fb);
    } else {
        (void)board_x4_epd_display(s_epd_fb);
    }
    s_epd_dirty = FALSE;
}

/**
 * @brief Fill gray16 Bayer pattern into framebuffer.
 * @return none
 */
static void __fill_gray16_pattern_fb(void)
{
    static const uint8_t s_bayer4[4][4] = {
        {0, 8, 2, 10},
        {12, 4, 14, 6},
        {3, 11, 1, 9},
        {15, 7, 13, 5},
    };
    int32_t  x;
    int32_t  y;
    int32_t  band;
    uint32_t thr;
    uint8_t  m;
    BOOL_T   white;

    x4_gfx_clear(&s_gfx, TRUE);

    for (y = 0; y < X4_EPD_H; y++) {
        for (x = 0; x < X4_EPD_W; x++) {
            band = (x * 16) / X4_EPD_W;
            if (band > 15) {
                band = 15;
            }
            thr   = (uint32_t)band * 16U + 16U;
            if (thr > 256U) {
                thr = 256U;
            }
            m     = s_bayer4[(unsigned)x % 4U][(unsigned)y % 4U];
            white = (((uint32_t)m * 16U + 8U) >= thr) ? TRUE : FALSE;
            x4_gfx_set_pixel(&s_gfx, x, y, white);
        }
    }
}

/**
 * @brief Fill bullseye test pattern into framebuffer.
 * @return none
 */
static void __fill_fb_bullseye(void)
{
    int32_t cx = X4_EPD_W / 2;
    int32_t cy = X4_EPD_H / 2;
    int32_t x;
    int32_t y;

    x4_gfx_clear(&s_gfx, TRUE);

    for (y = 0; y < X4_EPD_H; y++) {
        for (x = 0; x < X4_EPD_W; x++) {
            int32_t  dx    = x - cx;
            int32_t  dy    = y - cy;
            uint32_t d2    = (uint32_t)(dx * dx + dy * dy);
            uint32_t ring  = d2 / (uint32_t)(38 * 38);
            BOOL_T   white = ((ring & 1U) == 0U) ? TRUE : FALSE;

            x4_gfx_set_pixel(&s_gfx, x, y, white);
        }
    }
}

/**
 * @brief Draw checkerboard boot test pattern.
 * @return none
 */
static void __draw_checker_screen(void)
{
    int32_t cols = 16;
    int32_t rows = 10;
    int32_t tw   = X4_RENDER_W / cols;
    int32_t th   = X4_RENDER_H / rows;
    int32_t cx;
    int32_t cy;

    x4_gfx_clear(&s_gfx, TRUE);

    for (cy = 0; cy < rows; cy++) {
        for (cx = 0; cx < cols; cx++) {
            BOOL_T white = (((cx + cy) & 1) == 0) ? FALSE : TRUE;
            x4_gfx_fill_rect(&s_gfx, X4_ORIGIN_X + cx * tw, X4_ORIGIN_Y + cy * th, tw, th, white);
        }
    }
}

/**
 * @brief Mount SD card when available.
 * @return none
 */
static void __mount_sd_if_possible(void)
{
    OPERATE_RET rt = OPRT_OK;

    s_sd_mounted = FALSE;
    rt           = board_x4_sdcard_mount();
    if (OPRT_OK == rt) {
        s_sd_mounted = TRUE;
    }
}

/**
 * @brief Format SD usage line for dashboard.
 * @param[out] buf output buffer
 * @param[in] len buffer size
 * @return none
 */
static void __format_sd_line(char *buf, size_t len)
{
    uint64_t     total_bytes = 0;
    uint64_t     free_bytes  = 0;
    uint64_t     used_bytes  = 0;
    OPERATE_RET  rt          = OPRT_OK;
    double       total_gb;
    double       free_gb;
    double       used_gb;
    const double gib = (double)(1024ULL * 1024ULL * 1024ULL);

    if (!s_sd_mounted) {
        snprintf(buf, len, "SD: not mounted");
        return;
    }

    rt = board_x4_sdcard_get_usage(&total_bytes, &free_bytes);
    if (OPRT_OK != rt) {
        snprintf(buf, len, "SD: mounted (no df)");
        return;
    }

    used_bytes = (total_bytes > free_bytes) ? (total_bytes - free_bytes) : 0ULL;
    total_gb   = (double)total_bytes / gib;
    free_gb    = (double)free_bytes / gib;
    used_gb    = (double)used_bytes / gib;

    snprintf(buf, len, "SD: used %.2f / free %.2f (%.2f GB tot)", used_gb, free_gb, total_gb);
}

/**
 * @brief Run one-shot SD read/write smoke test.
 * @return none
 */
static void __sd_smoke_once(void)
{
    OPERATE_RET rt;
    char         rb[96];
    size_t       br = 0;

    if (!s_sd_mounted) {
        snprintf(s_sd_smoke_msg, sizeof(s_sd_smoke_msg), "Insert a card for FATFS smoke test.");
        return;
    }

    (void)board_x4_sdcard_ensure_dir("/x4lab");
    rt = board_x4_sdcard_write_file("/x4lab/smoke.txt", "x4-lab-ok\n", 9U);
    if (OPRT_OK != rt) {
        snprintf(s_sd_smoke_msg, sizeof(s_sd_smoke_msg), "Write failed: %d", rt);
        return;
    }
    (void)memset(rb, 0, sizeof(rb));
    rt = board_x4_sdcard_read_file_to_buffer("/x4lab/smoke.txt", rb, sizeof(rb), 0U, &br);
    if (OPRT_OK != rt) {
        snprintf(s_sd_smoke_msg, sizeof(s_sd_smoke_msg), "Read failed: %d", rt);
        return;
    }
    snprintf(s_sd_smoke_msg, sizeof(s_sd_smoke_msg), "Write+read OK (%u bytes). Path /x4lab/smoke.txt", (unsigned)br);
}

/**
 * @brief Draw one dashboard quadrant frame and title.
 * @param[in] x left coordinate in viewable area
 * @param[in] y top coordinate in viewable area
 * @param[in] title quadrant title
 * @return none
 */
static void __draw_quad_frame(int32_t x, int32_t y, const char *title)
{
    int32_t px = X4_ORIGIN_X + x;
    int32_t py = X4_ORIGIN_Y + X4_BAR_H + y;

    x4_gfx_fill_rect(&s_gfx, px, py, X4_QUAD_W, X4_QUAD_H, TRUE);
    x4_gfx_draw_rect(&s_gfx, px, py, X4_QUAD_W, X4_QUAD_H, 2, FALSE);
    x4_gfx_draw_text(&s_gfx, px + 6, py + 4, title, FALSE);
}

/**
 * @brief Draw splash screen into framebuffer.
 * @return none
 */
static void __build_splash_screen(void)
{
    int32_t y = X4_ORIGIN_Y + X4_RENDER_H / 2 - 48;

    x4_gfx_clear(&s_gfx, TRUE);
    x4_gfx_draw_text(&s_gfx, X4_ORIGIN_X + 220, y, "Hello World", FALSE);
    x4_gfx_draw_text(&s_gfx, X4_ORIGIN_X + 280, y + 28, "TuyaOpen", FALSE);
    x4_gfx_draw_text_wrap(&s_gfx, X4_ORIGIN_X + 120, y + 56, X4_RENDER_W - 240, "XTEINK X4 | ESP32-C3 | SSD1677 800x480", FALSE);
    __mark_dirty();
}

/**
 * @brief Draw key chip in keys quadrant.
 * @param[in] x chip left
 * @param[in] y chip top
 * @param[in] label chip label
 * @param[in] pressed highlight when pressed
 * @return none
 */
static void __draw_key_chip(int32_t x, int32_t y, const char *label, BOOL_T pressed)
{
    int32_t w = (X4_QUAD_W - 28) / 2;

    if (pressed) {
        x4_gfx_fill_rect(&s_gfx, x, y, w, 18, FALSE);
        x4_gfx_draw_rect(&s_gfx, x, y, w, 18, 1, FALSE);
        x4_gfx_draw_text(&s_gfx, x + 4, y + 2, label, TRUE);
    } else {
        x4_gfx_fill_rect(&s_gfx, x, y, w, 18, TRUE);
        x4_gfx_draw_rect(&s_gfx, x, y, w, 18, 1, FALSE);
        x4_gfx_draw_text(&s_gfx, x + 4, y + 2, label, FALSE);
    }
}

/**
 * @brief Refresh key quadrant highlight from button state.
 * @param[in] st button bitmask
 * @return none
 */
static void __refresh_keys_quadrant(uint8_t st)
{
    static const char *key_names[7] = {"Back", "OK", "Left", "Right", "Up", "Down", "PWR"};
#if XTEINK_X4_ENABLE_CLOUD
    /* Top row 4 + bottom row 3 (scheme E). Bit order matches ADC ladder. */
    static const uint8_t s_key_row[7] = {0, 0, 0, 0, 1, 1, 1};
    static const uint8_t s_key_col[7] = {0, 1, 2, 3, 0, 1, 2};
    int32_t ox = 0;
    int32_t oy = 0;
    int32_t i;
#else
    int32_t base_x = X4_ORIGIN_X + 12;
    int32_t base_y = X4_ORIGIN_Y + X4_BAR_H + X4_QUAD_H + X4_QUAD_GAP + 28;
    int32_t i;
#endif

#if XTEINK_X4_ENABLE_CLOUD
    __keys_grid_origin(&ox, &oy);
    x4_gfx_fill_rect(&s_gfx, ox - X4_KEY_PAD, oy - X4_KEY_PAD, X4_KEY_GRID_W + 2 * X4_KEY_PAD,
                     X4_KEY_GRID_H + 2 * X4_KEY_PAD, TRUE);

    for (i = 0; i < 7; i++) {
        int32_t cx;
        int32_t cy;
        int32_t tw;
        BOOL_T  pressed = (0U != (st & (1U << (unsigned)i))) ? TRUE : FALSE;

        cx = ox + X4_KEY_R + ((int32_t)s_key_col[i] * X4_KEY_PITCH);
        if (0U != s_key_row[i]) {
            cx += X4_KEY_PITCH / 2;
        }
        cy = oy + X4_KEY_R + ((int32_t)s_key_row[i] * (X4_KEY_D + X4_KEY_ROW_GAP));

        if (pressed) {
            x4_gfx_fill_circle(&s_gfx, cx, cy, X4_KEY_R, FALSE);
            x4_gfx_draw_circle(&s_gfx, cx, cy, X4_KEY_R, X4_KEY_STROKE, FALSE);
        } else {
            x4_gfx_fill_circle(&s_gfx, cx, cy, X4_KEY_R, TRUE);
            x4_gfx_draw_circle(&s_gfx, cx, cy, X4_KEY_R, X4_KEY_STROKE, FALSE);
        }

        tw = x4_gfx_text_width(key_names[i]);
        x4_gfx_draw_text(&s_gfx, cx - tw / 2, cy - x4_gfx_line_height() / 2, key_names[i],
                         pressed ? TRUE : FALSE);
    }
#else
    for (i = 0; i < 7; i++) {
        int32_t col = i % 2;
        int32_t row = i / 2;
        char    line[40];
        BOOL_T  pressed = (0U != (st & (1U << (unsigned)i))) ? TRUE : FALSE;

        snprintf(line, sizeof(line), "%u %s", (unsigned)i, key_names[i]);
        __draw_key_chip(base_x + col * ((X4_QUAD_W - 28) / 2 + 4), base_y + row * 22, line, pressed);
    }
#endif
}

#if XTEINK_X4_ENABLE_CLOUD
/**
 * @brief Compute top-left of the scheme-E key grid (centered in content area).
 * @param[out] ox grid left (first circle bbox left)
 * @param[out] oy grid top
 * @return none
 */
static void __keys_grid_origin(int32_t *ox, int32_t *oy)
{
    int32_t top;
    int32_t bot;
    int32_t avail;

    if (NULL != ox) {
        *ox = X4_ORIGIN_X + (X4_RENDER_W - X4_KEY_GRID_W) / 2;
    }

    top   = X4_ORIGIN_Y + X4_KEY_TITLE_H + X4_KEY_HINT_H;
    bot   = X4_ORIGIN_Y + X4_RENDER_H - 16;
    avail = bot - top;
    if (avail < X4_KEY_GRID_H) {
        avail = X4_KEY_GRID_H;
    }
    if (NULL != oy) {
        *oy = top + (avail - X4_KEY_GRID_H) / 2;
    }
}

/**
 * @brief Compute byte-aligned keys window in panel coordinates.
 * @param[out] x left
 * @param[out] y top
 * @param[out] w width
 * @param[out] h height
 * @return none
 */
static void __keys_region_get(uint16_t *x, uint16_t *y, uint16_t *w, uint16_t *h)
{
    int32_t  ox = 0;
    int32_t  oy = 0;
    int32_t  x0;
    int32_t  y0;
    int32_t  x1;
    int32_t  y1;
    uint16_t ax;
    uint16_t aw;

    __keys_grid_origin(&ox, &oy);
    x0 = ox - X4_KEY_PAD;
    y0 = oy - X4_KEY_PAD;
    x1 = ox + X4_KEY_GRID_W + X4_KEY_PAD;
    y1 = oy + X4_KEY_GRID_H + X4_KEY_PAD;
    if (x0 < 0) {
        x0 = 0;
    }
    if (y0 < 0) {
        y0 = 0;
    }
    if (x1 > X4_EPD_W) {
        x1 = X4_EPD_W;
    }
    if (y1 > X4_EPD_H) {
        y1 = X4_EPD_H;
    }

    ax = (uint16_t)((uint16_t)x0 & (uint16_t)~0x7U);
    aw = (uint16_t)(((uint16_t)x1 + 7U) & (uint16_t)~0x7U);
    if (aw > (uint16_t)X4_EPD_W) {
        aw = (uint16_t)X4_EPD_W;
    }
    aw = (uint16_t)(aw - ax);

    if (NULL != x) {
        *x = ax;
    }
    if (NULL != y) {
        *y = (uint16_t)y0;
    }
    if (NULL != w) {
        *w = aw;
    }
    if (NULL != h) {
        *h = (uint16_t)(y1 - y0);
    }
}

/**
 * @brief Pack one framebuffer rectangle into a tight row buffer.
 * @param[out] dst packed destination (h * (w/8) bytes)
 * @param[in] fb full framebuffer
 * @param[in] x left (byte-aligned)
 * @param[in] y top
 * @param[in] w width (multiple of 8)
 * @param[in] h height
 * @return none
 */
static void __pack_fb_region(uint8_t *dst, const uint8_t *fb, uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
    uint16_t x_bytes = (uint16_t)(x / 8U);
    uint16_t w_bytes = (uint16_t)(w / 8U);
    uint16_t row;

    if (NULL == dst || NULL == fb || 0U == w_bytes || 0U == h) {
        return;
    }
    if ((uint32_t)w_bytes * (uint32_t)h > (uint32_t)X4_KEYS_OLD_MAX) {
        return;
    }

    for (row = 0; row < h; row++) {
        (void)memcpy(dst + ((uint32_t)row * (uint32_t)w_bytes),
                     fb + (((uint32_t)y + (uint32_t)row) * X4_EPD_STRIDE) + x_bytes, w_bytes);
    }
}

/**
 * @brief Redraw keys and push only the keys window (fast partial refresh).
 * @param[in] st button bitmask
 * @return none
 */
static void __keys_partial_update(uint8_t st)
{
    uint16_t x = 0;
    uint16_t y = 0;
    uint16_t w = 0;
    uint16_t h = 0;

    __keys_region_get(&x, &y, &w, &h);
    __pack_fb_region(s_keys_old_region, s_epd_fb, x, y, w, h);
    __refresh_keys_quadrant(st);
    (void)board_x4_epd_display_partial(s_epd_fb, s_keys_old_region, x, y, w, h);
}
#endif

/**
 * @brief Cloud-mode UI: keys test only (no battery/uptime live updates).
 * @return none
 */
static void __build_keys_test_screen(void)
{
    uint8_t st0 = 0;

    x4_gfx_clear(&s_gfx, TRUE);
    x4_gfx_fill_rect(&s_gfx, X4_ORIGIN_X, X4_ORIGIN_Y, X4_RENDER_W, 64, FALSE);
    x4_gfx_fill_rect(&s_gfx, X4_ORIGIN_X + 2, X4_ORIGIN_Y + 2, X4_RENDER_W - 4, 60, TRUE);
    x4_gfx_draw_text(&s_gfx, X4_ORIGIN_X + 24, X4_ORIGIN_Y + 24, "Keys test (ADC ladder)", FALSE);
    x4_gfx_draw_text_wrap(&s_gfx, X4_ORIGIN_X + 24, X4_ORIGIN_Y + 72, X4_RENDER_W - 48,
                          "Top 4 / bottom 3 circles. Press to highlight. Long PWR 3s -> sleep", FALSE);

    s_last_key_st = 0xFFU;
    if (OPRT_OK == board_x4_buttons_get_state(&st0)) {
        __refresh_keys_quadrant(st0);
        s_last_key_st = st0;
    } else {
        __refresh_keys_quadrant(0);
        s_last_key_st = 0;
    }
    __mark_dirty();
}

/**
 * @brief Draw full dashboard into framebuffer.
 * @return none
 */
static void __build_dashboard(void)
{
    char line[288];
    int32_t i;
    int32_t bar_w;
    uint8_t st0 = 0;

    __sd_smoke_once();
    x4_gfx_clear(&s_gfx, TRUE);

    x4_gfx_fill_rect(&s_gfx, X4_ORIGIN_X, X4_ORIGIN_Y, X4_RENDER_W, X4_BAR_H, FALSE);
    x4_gfx_fill_rect(&s_gfx, X4_ORIGIN_X + 2, X4_ORIGIN_Y + 2, X4_RENDER_W - 4, X4_BAR_H - 4, TRUE);
    x4_gfx_draw_text_wrap(&s_gfx, X4_ORIGIN_X + 8, X4_ORIGIN_Y + 8, X4_RENDER_W - 160,
                          "TuyaOpen + XTEInk X4 | Demo App Hardware Func Test", FALSE);
    snprintf(line, sizeof(line), "up 0 s");
    x4_gfx_draw_text(&s_gfx, X4_ORIGIN_X + X4_RENDER_W - 120, X4_ORIGIN_Y + 12, line, FALSE);

    __draw_quad_frame(0, 0, "Power / battery");
    __draw_quad_frame(X4_QUAD_W + X4_QUAD_GAP, 0, "microSD / FATFS");
    __draw_quad_frame(0, X4_QUAD_H + X4_QUAD_GAP, "Keys (ADC ladder)");
    __draw_quad_frame(X4_QUAD_W + X4_QUAD_GAP, X4_QUAD_H + X4_QUAD_GAP, "EPD / SPI + about");

    x4_gfx_fill_rect(&s_gfx, X4_ORIGIN_X, X4_ORIGIN_Y + X4_RENDER_H - X4_FOOT_H, X4_RENDER_W, X4_FOOT_H, FALSE);
    x4_gfx_fill_rect(&s_gfx, X4_ORIGIN_X + 2, X4_ORIGIN_Y + X4_RENDER_H - X4_FOOT_H + 2, X4_RENDER_W - 4,
                     X4_FOOT_H - 4, TRUE);

    bar_w = (X4_QUAD_W - 40) / 10;
    if (bar_w < 8) {
        bar_w = 8;
    }
    for (i = 0; i < 10; i++) {
        int32_t bx = X4_ORIGIN_X + X4_QUAD_W + X4_QUAD_GAP + 12 + i * (bar_w + 4);
        int32_t by = X4_ORIGIN_Y + X4_BAR_H + X4_QUAD_H + X4_QUAD_GAP + 28;
        BOOL_T  black = ((i & 1) != 0) ? TRUE : FALSE;

        x4_gfx_fill_rect(&s_gfx, bx, by, bar_w, 44, black ? FALSE : TRUE);
        x4_gfx_draw_rect(&s_gfx, bx, by, bar_w, 44, 1, FALSE);
    }

    s_hub_slow_tick = 0U;
    __dashboard_refresh_slow();
    if (OPRT_OK == board_x4_buttons_get_state(&st0)) {
        __refresh_keys_quadrant(st0);
    }
    __mark_dirty();
}

/**
 * @brief Update slow-changing dashboard fields.
 * @return none
 */
static void __dashboard_refresh_slow(void)
{
    char        line[288];
    char        sd_line[160];
    uint32_t    mv   = 0;
    uint8_t     pct  = 0;
    OPERATE_RET rt   = OPRT_OK;
    // bool        chg  = false;
    SYS_TIME_T  now  = tal_system_get_millisecond();
    uint32_t    up_s = (uint32_t)((now - s_boot_ms) / 1000U);

    snprintf(line, sizeof(line), "up %lu s", (unsigned long)up_s);
    x4_gfx_fill_rect(&s_gfx, X4_ORIGIN_X + X4_RENDER_W - 128, X4_ORIGIN_Y + 10, 120, 18, TRUE);
    x4_gfx_draw_text(&s_gfx, X4_ORIGIN_X + X4_RENDER_W - 120, X4_ORIGIN_Y + 12, line, FALSE);

    rt = board_x4_battery_read(&mv, &pct);
    if (OPRT_OK == rt) {
        // (void)board_x4_charge_sense_get(&chg); /* triggers ESP-IDF GPIO[20] log spam */
        snprintf(line, sizeof(line), "%u%%\n%lu mV", (unsigned)pct, (unsigned long)mv);
    } else {
        snprintf(line, sizeof(line), "read err %d", rt);
    }
    x4_gfx_fill_rect(&s_gfx, X4_ORIGIN_X + 6, X4_ORIGIN_Y + X4_BAR_H + 24, X4_QUAD_W - 12, X4_QUAD_H - 30, TRUE);
    x4_gfx_draw_text_wrap(&s_gfx, X4_ORIGIN_X + 6, X4_ORIGIN_Y + X4_BAR_H + 24, X4_QUAD_W - 12, line, FALSE);

    __format_sd_line(sd_line, sizeof(sd_line));
    snprintf(line, sizeof(line), "%s\n%s", sd_line, s_sd_smoke_msg);
    x4_gfx_fill_rect(&s_gfx, X4_ORIGIN_X + X4_QUAD_W + X4_QUAD_GAP + 6, X4_ORIGIN_Y + X4_BAR_H + 24, X4_QUAD_W - 12,
                     X4_QUAD_H - 30, TRUE);
    x4_gfx_draw_text_wrap(&s_gfx, X4_ORIGIN_X + X4_QUAD_W + X4_QUAD_GAP + 6, X4_ORIGIN_Y + X4_BAR_H + 24,
                          X4_QUAD_W - 12, line, FALSE);

    snprintf(line, sizeof(line), "SSD1677 soft-SPI\nBuilt " __DATE__ "\nLong PWR 3s -> sleep");
    x4_gfx_fill_rect(&s_gfx, X4_ORIGIN_X + X4_QUAD_W + X4_QUAD_GAP + 6,
                     X4_ORIGIN_Y + X4_BAR_H + X4_QUAD_H + X4_QUAD_GAP + 80, X4_QUAD_W - 12, 56, TRUE);
    x4_gfx_draw_text_wrap(&s_gfx, X4_ORIGIN_X + X4_QUAD_W + X4_QUAD_GAP + 6,
                          X4_ORIGIN_Y + X4_BAR_H + X4_QUAD_H + X4_QUAD_GAP + 80, X4_QUAD_W - 12, line, FALSE);

    snprintf(line, sizeof(line), "%s | %s | %s", PLATFORM_BOARD, PROJECT_NAME, s_cloud_status);
    x4_gfx_fill_rect(&s_gfx, X4_ORIGIN_X + 8, X4_ORIGIN_Y + X4_RENDER_H - X4_FOOT_H + 8, X4_RENDER_W - 16,
                     X4_FOOT_H - 16, TRUE);
    x4_gfx_draw_text_wrap(&s_gfx, X4_ORIGIN_X + 10, X4_ORIGIN_Y + X4_RENDER_H - X4_FOOT_H + 12, X4_RENDER_W - 20,
                          line, FALSE);
    __mark_dirty();
}

/**
 * @brief Draw deep-sleep power-off screen.
 * @param[in] invert invert colors
 * @return none
 */
static void __draw_power_off_screen(BOOL_T invert)
{
    x4_gfx_clear(&s_gfx, invert ? FALSE : TRUE);
    if (invert) {
        x4_gfx_draw_text(&s_gfx, X4_ORIGIN_X + 280, X4_ORIGIN_Y + X4_RENDER_H / 2 - 24, "DEEP SLEEP", TRUE);
        x4_gfx_draw_text_wrap(&s_gfx, X4_ORIGIN_X + 80, X4_ORIGIN_Y + X4_RENDER_H / 2 + 8, X4_RENDER_W - 160,
                              "Hold PWR 3s after wake to run.\nRelease sooner to stay asleep.", TRUE);
    } else {
        x4_gfx_draw_text(&s_gfx, X4_ORIGIN_X + 280, X4_ORIGIN_Y + X4_RENDER_H / 2 - 24, "DEEP SLEEP", FALSE);
        x4_gfx_draw_text_wrap(&s_gfx, X4_ORIGIN_X + 80, X4_ORIGIN_Y + X4_RENDER_H / 2 + 8, X4_RENDER_W - 160,
                              "Hold PWR 3s after wake to run.\nRelease sooner to stay asleep.", FALSE);
    }
    __mark_dirty();
}

/**
 * @brief Flash power-off screen then enter deep sleep.
 * @return none
 */
static void __user_power_off_sequence(void)
{
    uint32_t pass;

    if (s_power_off_started) {
        return;
    }
    s_power_off_started = TRUE;

    __draw_power_off_screen(FALSE);
    __epd_push_if_dirty(FALSE);

    for (pass = 0U; pass < X4_PWR_OFF_FLASH_UPDATES; pass++) {
        BOOL_T invert = (((unsigned)pass + 1U) & 1U) != 0U;

        __draw_power_off_screen(invert);
        __epd_push_if_dirty(FALSE);
        tal_system_sleep(X4_PWR_OFF_FLASH_PAUSE_MS);
    }

    __draw_power_off_screen(FALSE);
    __epd_push_if_dirty(TRUE);
    tal_system_sleep(400);

    if (s_sd_mounted) {
        (void)board_x4_sdcard_unmount();
        s_sd_mounted = FALSE;
    }

    (void)board_x4_power_shutdown();
}

/**
 * @brief Poll buttons and refresh dashboard regions.
 * @return none
 */
static void __poll_input(void)
{
    OPERATE_RET rt_btn = OPRT_OK;
    uint8_t     st     = 0;

    if (s_power_off_started) {
        return;
    }

    rt_btn = board_x4_buttons_get_state(&st);
    if (OPRT_OK != rt_btn) {
        s_pwr_hold_ms = 0U;
#if !XTEINK_X4_ENABLE_CLOUD
        s_hub_slow_tick = 0U;
        __dashboard_refresh_slow();
#endif
        return;
    }

    if (0U != (st & X4_BTN_POWER)) {
        s_pwr_hold_ms += X4_INPUT_POLL_MS;
    } else {
        s_pwr_hold_ms = 0U;
    }
    if (s_pwr_hold_ms >= X4_PWR_HOLD_MS) {
        __user_power_off_sequence();
        return;
    }

#if XTEINK_X4_ENABLE_CLOUD
    /* Keys-only UI: partial window refresh (not full panel). */
    if (st != s_last_key_st) {
        __keys_partial_update(st);
        s_last_key_st = st;
    }
#else
    __refresh_keys_quadrant(st);
    s_hub_slow_tick++;
    if (s_hub_slow_tick >= X4_HUB_SLOW_N) {
        s_hub_slow_tick = 0U;
        __dashboard_refresh_slow();
    }
#endif
}

/**
 * @brief Display thread: board bring-up, boot patterns, dashboard loop.
 * @param[in] arg unused
 * @return none
 */
static void __display_thread(void *arg)
{
    uint32_t    last_poll_ms = 0;
    uint32_t    last_push_ms = 0;
    OPERATE_RET rt           = OPRT_OK;

    (void)arg;

    PR_NOTICE("xteink_x4_display thread start (lightweight gfx)");

    (void)memset(s_epd_fb, 0xFF, sizeof(s_epd_fb));
    x4_gfx_init(&s_gfx, s_epd_fb, X4_EPD_W, X4_EPD_H);
    s_epd_dirty         = FALSE;
    s_boot_ms           = tal_system_get_millisecond();
    s_pwr_hold_ms       = 0U;
    s_power_off_started = FALSE;
    s_hub_slow_tick     = 0U;

    TUYA_CALL_ERR_LOG(board_register_hardware());
    if (OPRT_OK != rt) {
        PR_ERR("X4 display: board_register_hardware failed, UI disabled");
        __signal_display_ready();
        return;
    }
#if !XTEINK_X4_ENABLE_CLOUD
    {
        X4_WAKEUP_CLASS_E cls;

        if (OPRT_OK == board_x4_sleep_classify_wakeup(&cls)) {
            if (X4_WAKEUP_CLASS_AFTER_USB_POWER == cls) {
                PR_NOTICE("X4: wake classified as USB power boot -> deep sleep (CrossPoint policy)");
                (void)board_x4_power_shutdown();
            } else if (X4_WAKEUP_CLASS_POWER_BUTTON == cls) {
                rt = board_x4_power_verify_gpio_wake((uint32_t)X4_PWR_HOLD_MS, FALSE);
                TUYA_CALL_ERR_LOG(rt);
            }
        }
    }
#endif
#if !XTEINK_X4_ENABLE_CLOUD
    __mount_sd_if_possible();

    __build_splash_screen();
    __epd_push_if_dirty(TRUE);
    tal_system_sleep((uint32_t)X4_SPLASH_HOLD_MS);

    __draw_checker_screen();
    __epd_push_if_dirty(TRUE);
    tal_system_sleep(400);

    __fill_gray16_pattern_fb();
    __epd_push_if_dirty(TRUE);
    tal_system_sleep((uint32_t)X4_GRAY16_HOLD_MS);

    __fill_fb_bullseye();
    __epd_push_if_dirty(TRUE);
    tal_system_sleep((uint32_t)X4_BULLSEYE_HOLD_MS);
#else
    /* Cloud: keys-test screen only; ready after first frame so WiFi/BLE can start. */
    __build_keys_test_screen();
    __epd_push_if_dirty(TRUE);
    __signal_display_ready();
#endif

#if !XTEINK_X4_ENABLE_CLOUD
    __build_dashboard();
    __epd_push_if_dirty(FALSE);
#endif

    last_poll_ms = tal_system_get_millisecond();
    last_push_ms = last_poll_ms;

    for (;;) {
        uint32_t now = tal_system_get_millisecond();

        if ((now - last_poll_ms) >= X4_INPUT_POLL_MS) {
            last_poll_ms = now;
            __poll_input();
        }
        if ((now - last_push_ms) >= X4_EPD_PUSH_MS) {
            last_push_ms = now;
            __epd_push_if_dirty(FALSE);
        }
        tal_system_sleep(5);
    }
}

/**
 * @brief Update cloud status text shown on the dashboard footer.
 * @param[in] status Short status string
 * @return none
 */
void xteink_x4_display_set_cloud_status(const char *status)
{
    if (status == NULL) {
        return;
    }
    snprintf(s_cloud_status, sizeof(s_cloud_status), "%s", status);
}

/**
 * @brief Block until EPD hardware init and first splash frame are done.
 * @param[in] timeout_ms Max wait in milliseconds (0 = poll only)
 * @return OPRT_OK when ready, OPRT_TIMEOUT on timeout, OPRT_COM_ERROR if display not started
 */
OPERATE_RET xteink_x4_display_wait_ready(uint32_t timeout_ms)
{
    if (TRUE == s_display_ready) {
        return OPRT_OK;
    }
    if (s_display_thread == NULL) {
        return OPRT_COM_ERROR;
    }
    if (0U == timeout_ms) {
        return OPRT_TIMEOUT;
    }
    if (s_display_ready_sem == NULL) {
        return OPRT_COM_ERROR;
    }
    return tal_semaphore_wait(s_display_ready_sem, timeout_ms);
}

/**
 * @brief Start lightweight EPD UI in a background thread.
 * @return OPRT_OK on success
 */
OPERATE_RET xteink_x4_display_start(void)
{
    THREAD_CFG_T cfg = {0};
    static char  display_thread_name[] = "x4_display";
    OPERATE_RET  rt = OPRT_OK;

    if (s_display_thread != NULL) {
        return OPRT_OK;
    }

    if (s_display_ready_sem == NULL) {
        TUYA_CALL_ERR_RETURN(tal_semaphore_create_init(&s_display_ready_sem, 0, 1));
    }

    cfg.stackDepth = 1024 * 6;
    cfg.priority   = THREAD_PRIO_2;
    cfg.thrdname   = display_thread_name;

    return tal_thread_create_and_start(&s_display_thread, NULL, NULL, __display_thread, NULL, &cfg);
}
