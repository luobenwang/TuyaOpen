/**
 * @file ui_water_drop.c
 * @brief Playful water baby GIFs (blue / yellow) roaming the whole screen
 * @version 1.7
 * @date 2026-05-11
 * @copyright Copyright (c) Tuya Inc.
 */
#include "tal_api.h"
#include "lvgl.h"

#include "app_water_stats.h"

LV_IMG_DECLARE(water_baby_blue);
LV_IMG_DECLARE(water_baby_yellow);

/* ---------------------------------------------------------------------------
 * Macros
 * --------------------------------------------------------------------------- */
/** Match embedded GIF pixel size (see tools/gen_water_baby_assets.py W,H) */
#define BABY_DISP_W        80
#define BABY_DISP_H        40
#define BABY_BEZIER_STEPS  1024
/** Today drink count >= this -> blue baby; below -> yellow (same ladder as water_time3) */
#define WATER_TIME3_BLUE_MIN 3

/* ---------------------------------------------------------------------------
 * Type definitions
 * --------------------------------------------------------------------------- */
typedef struct {
    lv_obj_t *obj;
    int32_t sx;
    int32_t sy;
    int32_t cx;
    int32_t cy;
    int32_t ex;
    int32_t ey;
} baby_move_t;

/* ---------------------------------------------------------------------------
 * File scope variables
 * --------------------------------------------------------------------------- */
static lv_obj_t *s_water_gif = NULL;
static baby_move_t s_move;

/* ---------------------------------------------------------------------------
 * Forward declarations
 * --------------------------------------------------------------------------- */
static void baby_move_exec(void *var, int32_t v);
static void baby_start_move_internal(void);
static void baby_move_completed(lv_anim_t *a);
static void baby_delay_cb(lv_timer_t *t);

/**
 * @brief Remove theme chrome so the GIF has no visible frame or letterbox fill
 * @param[in] obj water baby lv_gif handle
 * @return none
 */
static void baby_clear_gif_chrome(lv_obj_t *obj)
{
    if (obj == NULL) {
        return;
    }
    lv_obj_set_style_bg_opa(obj, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(obj, 0, 0);
    lv_obj_set_style_border_opa(obj, LV_OPA_TRANSP, 0);
    lv_obj_set_style_outline_width(obj, 0, 0);
    lv_obj_set_style_outline_opa(obj, LV_OPA_TRANSP, 0);
    lv_obj_set_style_shadow_width(obj, 0, 0);
    lv_obj_set_style_shadow_opa(obj, LV_OPA_TRANSP, 0);
    lv_obj_set_style_radius(obj, 0, 0);
    lv_obj_set_style_pad_all(obj, 0, 0);
    lv_obj_set_style_clip_corner(obj, false, 0);
}

/**
 * @brief Pick GIF skin from today drink count (water_time3 ladder)
 * @return none
 * @note Uses app_water_stats today total: >= WATER_TIME3_BLUE_MIN -> blue, else yellow.
 */
static void baby_apply_skin_by_water_time3(void)
{
    const lv_img_dsc_t *skin;
    int                 today_cnt = 0;
    OPERATE_RET         rt;

    if (s_water_gif == NULL) {
        return;
    }

    skin = &water_baby_yellow;
    rt   = app_water_stats_get_today_count(&today_cnt);
    if (rt == OPRT_OK && today_cnt >= WATER_TIME3_BLUE_MIN) {
        skin = &water_baby_blue;
    }

    lv_gif_set_src(s_water_gif, skin);
    lv_image_set_antialias(s_water_gif, false);
    baby_clear_gif_chrome(s_water_gif);
}

/* ---------------------------------------------------------------------------
 * Function implementations
 * --------------------------------------------------------------------------- */
/**
 * @brief Quadratic Bezier at parameter t in 0..BABY_BEZIER_STEPS
 * @param[in] p0 start value
 * @param[in] p1 control value
 * @param[in] p2 end value
 * @param[in] t parameter
 * @return interpolated coordinate
 */
static int32_t baby_quad_bezier(int32_t p0, int32_t p1, int32_t p2, int32_t t)
{
    int64_t om = (int64_t)BABY_BEZIER_STEPS - t;
    int64_t tt = t;
    int64_t den = (int64_t)BABY_BEZIER_STEPS * (int64_t)BABY_BEZIER_STEPS;

    return (int32_t)((om * om * (int64_t)p0 + 2LL * om * tt * (int64_t)p1 + tt * tt * (int64_t)p2 + den / 2) / den);
}

/**
 * @brief Pick a control point for a visible arc / S-curve feel
 * @param[in] sx start x
 * @param[in] sy start y
 * @param[in] ex end x
 * @param[in] ey end y
 * @param[out] cx control x
 * @param[out] cy control y
 * @param[in] start_offscreen true if start point is outside visible area (wider control clamp)
 * @return none
 */
static void baby_pick_control(int32_t sx, int32_t sy, int32_t ex, int32_t ey, int32_t *cx, int32_t *cy,
                              bool start_offscreen)
{
    int32_t mx = (sx + ex) / 2;
    int32_t my = (sy + ey) / 2;
    int32_t dx = ex - sx;
    int32_t dy = ey - sy;
    int32_t mag = (int32_t)lv_rand(14u, 42u);
    int32_t ox;
    int32_t oy;
    int32_t cx_min;
    int32_t cx_max;
    int32_t cy_min;
    int32_t cy_max;

    if (lv_rand(0u, 1u) == 0u) {
        mag = -mag;
    }

    /* Perpendicular offset scaled for 160x80 panel */
    ox = (-dy * mag) / 72;
    oy = (dx * mag) / 72;
    ox += (int32_t)lv_rand(-10, 10);
    oy += (int32_t)lv_rand(-8, 8);

    *cx = mx + ox;
    *cy = my + oy;

    /* Wider bounds when entering from outside so the arc can bow naturally */
    if (start_offscreen) {
        cx_min = -(int32_t)BABY_DISP_W - 48;
        cx_max = (int32_t)LV_HOR_RES + (int32_t)BABY_DISP_W + 48;
        cy_min = -(int32_t)BABY_DISP_H - 40;
        cy_max = (int32_t)LV_VER_RES + (int32_t)BABY_DISP_H + 40;
    } else {
        cx_min = -24;
        cx_max = (int32_t)LV_HOR_RES + 24;
        cy_min = -20;
        cy_max = (int32_t)LV_VER_RES + 20;
    }

    *cx = LV_CLAMP(cx_min, *cx, cx_max);
    *cy = LV_CLAMP(cy_min, *cy, cy_max);
}

/**
 * @brief Random easing path for playful motion
 * @return path callback
 */
static lv_anim_path_cb_t baby_pick_path(void)
{
    uint32_t r = lv_rand(0u, 4u);

    switch (r) {
        case 0u:
            return lv_anim_path_overshoot;
        case 1u:
            return lv_anim_path_bounce;
        case 2u:
            return lv_anim_path_ease_in_out;
        case 3u:
            return lv_anim_path_ease_out;
        default:
            return lv_anim_path_ease_in;
    }
}

/**
 * @brief Easing when sliding in from off-screen (softer landing)
 * @return path callback
 */
static lv_anim_path_cb_t baby_pick_path_entry(void)
{
    uint32_t r = lv_rand(0u, 3u);

    switch (r) {
        case 0u:
            return lv_anim_path_ease_out;
        case 1u:
            return lv_anim_path_ease_in_out;
        case 2u:
            return lv_anim_path_linear;
        default:
            return lv_anim_path_ease_in;
    }
}

/**
 * @brief Animation exec: move along quadratic Bezier
 * @param[in] var pointer to baby_move_t
 * @param[in] v parameter 0..BABY_BEZIER_STEPS
 * @return none
 */
static void baby_move_exec(void *var, int32_t v)
{
    baby_move_t *m = (baby_move_t *)var;
    lv_coord_t x;
    lv_coord_t y;

    if (m->obj == NULL) {
        return;
    }

    x = (lv_coord_t)baby_quad_bezier(m->sx, m->cx, m->ex, v);
    y = (lv_coord_t)baby_quad_bezier(m->sy, m->cy, m->ey, v);
    lv_obj_set_pos(m->obj, x, y);
}

/**
 * @brief After a pause, start the next roam segment
 * @param[in] t one-shot timer
 * @return none
 */
static void baby_delay_cb(lv_timer_t *t)
{
    lv_timer_delete(t);
    baby_start_move_internal();
}

/**
 * @brief When move ends: random pause, optional GIF restart, schedule next hop
 * @param[in] a finished animation
 * @return none
 */
static void baby_move_completed(lv_anim_t *a)
{
    uint32_t pause;
    lv_timer_t *tw;

    LV_UNUSED(a);

    if (s_water_gif != NULL) {
        lv_obj_move_foreground(s_water_gif);
        /* Occasionally restart GIF for a cheeky loop */
        if (lv_rand(0u, 9u) < 2u) {
            lv_gif_restart(s_water_gif);
        }
    }

    pause = lv_rand(650u, 3600u);
    tw = lv_timer_create(baby_delay_cb, pause, NULL);
    if (tw != NULL) {
        lv_timer_set_repeat_count(tw, 1);
    }
}

/**
 * @brief Start one Bezier roam from current position to a random destination
 * @return none
 */
static void baby_start_move_internal(void)
{
    lv_anim_t a;
    uint32_t max_x;
    uint32_t max_y;
    int32_t dx;
    int32_t dy;
    uint32_t roll;
    uint32_t duration_ms;
    lv_anim_path_cb_t path_cb;

    if (s_water_gif == NULL) {
        return;
    }

    max_x = (uint32_t)LV_MAX(0, (lv_coord_t)LV_HOR_RES - BABY_DISP_W);
    max_y = (uint32_t)LV_MAX(0, (lv_coord_t)LV_VER_RES - BABY_DISP_H);

    lv_anim_delete(&s_move, baby_move_exec);

    baby_apply_skin_by_water_time3();

    s_move.obj = s_water_gif;

    /* Destination always fully visible */
    s_move.ex = (int32_t)lv_rand(0u, max_x);
    s_move.ey = (int32_t)lv_rand(0u, max_y);

    roll = lv_rand(0u, 9u);
    if (roll <= 3u) {
        /* Roam inside screen from current position */
        s_move.sx = (int32_t)lv_obj_get_x(s_water_gif);
        s_move.sy = (int32_t)lv_obj_get_y(s_water_gif);
        dx = s_move.ex - s_move.sx;
        dy = s_move.ey - s_move.sy;
        if ((dx * dx + dy * dy) < (12 * 12)) {
            s_move.ex = (int32_t)lv_rand(0u, max_x);
            s_move.ey = (int32_t)lv_rand(0u, max_y);
        }
        baby_pick_control(s_move.sx, s_move.sy, s_move.ex, s_move.ey, &s_move.cx, &s_move.cy, false);
        duration_ms = lv_rand(900u, 2400u);
        path_cb = baby_pick_path();
    } else if (roll == 4u) {
        /* Enter from left */
        s_move.sx = -(int32_t)BABY_DISP_W - (int32_t)lv_rand(6u, 36u);
        s_move.sy = (int32_t)lv_rand(0u, max_y);
        baby_pick_control(s_move.sx, s_move.sy, s_move.ex, s_move.ey, &s_move.cx, &s_move.cy, true);
        duration_ms = lv_rand(1500u, 3400u);
        path_cb = baby_pick_path_entry();
    } else if (roll == 5u) {
        /* Enter from right */
        s_move.sx = (int32_t)LV_HOR_RES + (int32_t)lv_rand(6u, 36u);
        s_move.sy = (int32_t)lv_rand(0u, max_y);
        baby_pick_control(s_move.sx, s_move.sy, s_move.ex, s_move.ey, &s_move.cx, &s_move.cy, true);
        duration_ms = lv_rand(1500u, 3400u);
        path_cb = baby_pick_path_entry();
    } else if (roll == 6u) {
        /* Enter from top */
        s_move.sx = (int32_t)lv_rand(0u, max_x);
        s_move.sy = -(int32_t)BABY_DISP_H - (int32_t)lv_rand(6u, 32u);
        baby_pick_control(s_move.sx, s_move.sy, s_move.ex, s_move.ey, &s_move.cx, &s_move.cy, true);
        duration_ms = lv_rand(1500u, 3400u);
        path_cb = baby_pick_path_entry();
    } else if (roll == 7u) {
        /* Enter from bottom */
        s_move.sx = (int32_t)lv_rand(0u, max_x);
        s_move.sy = (int32_t)LV_VER_RES + (int32_t)lv_rand(6u, 32u);
        baby_pick_control(s_move.sx, s_move.sy, s_move.ex, s_move.ey, &s_move.cx, &s_move.cy, true);
        duration_ms = lv_rand(1500u, 3400u);
        path_cb = baby_pick_path_entry();
    } else if (roll == 8u) {
        /* Enter from top-left or top-right corner outside */
        if (lv_rand(0u, 1u) == 0u) {
            s_move.sx = -(int32_t)BABY_DISP_W - (int32_t)lv_rand(4u, 28u);
            s_move.sy = -(int32_t)BABY_DISP_H - (int32_t)lv_rand(4u, 24u);
        } else {
            s_move.sx = (int32_t)LV_HOR_RES + (int32_t)lv_rand(4u, 28u);
            s_move.sy = -(int32_t)BABY_DISP_H - (int32_t)lv_rand(4u, 24u);
        }
        baby_pick_control(s_move.sx, s_move.sy, s_move.ex, s_move.ey, &s_move.cx, &s_move.cy, true);
        duration_ms = lv_rand(1700u, 3800u);
        path_cb = baby_pick_path_entry();
    } else {
        /* Enter from bottom-left or bottom-right corner outside */
        if (lv_rand(0u, 1u) == 0u) {
            s_move.sx = -(int32_t)BABY_DISP_W - (int32_t)lv_rand(4u, 28u);
            s_move.sy = (int32_t)LV_VER_RES + (int32_t)lv_rand(4u, 24u);
        } else {
            s_move.sx = (int32_t)LV_HOR_RES + (int32_t)lv_rand(4u, 28u);
            s_move.sy = (int32_t)LV_VER_RES + (int32_t)lv_rand(4u, 24u);
        }
        baby_pick_control(s_move.sx, s_move.sy, s_move.ex, s_move.ey, &s_move.cx, &s_move.cy, true);
        duration_ms = lv_rand(1700u, 3800u);
        path_cb = baby_pick_path_entry();
    }

    lv_anim_init(&a);
    lv_anim_set_var(&a, &s_move);
    lv_anim_set_exec_cb(&a, baby_move_exec);
    lv_anim_set_values(&a, 0, BABY_BEZIER_STEPS);
    lv_anim_set_duration(&a, duration_ms);
    lv_anim_set_path_cb(&a, path_cb);
    lv_anim_set_completed_cb(&a, baby_move_completed);
    lv_anim_start(&a);
}

/**
 * @brief Initialize water baby GIF and start roaming
 * @return 0 on success, -1 on failure
 * @note Call from LVGL context with display lock held (same as other __ui_* init)
 */
int __ui_water_drop_init(void)
{
    lv_obj_t *scr = lv_screen_active();
    uint32_t max_x;
    uint32_t max_y;
    lv_coord_t px;
    lv_coord_t py;

    lv_rand_set_seed((uint32_t)tal_system_get_tick_count());

    s_water_gif = lv_gif_create(scr);
    if (s_water_gif == NULL) {
        PR_ERR("water baby: lv_gif_create failed");
        return -1;
    }

    baby_apply_skin_by_water_time3();
    lv_obj_set_size(s_water_gif, BABY_DISP_W, BABY_DISP_H);
    baby_clear_gif_chrome(s_water_gif);

    max_x = (uint32_t)LV_MAX(0, (lv_coord_t)LV_HOR_RES - BABY_DISP_W);
    max_y = (uint32_t)LV_MAX(0, (lv_coord_t)LV_VER_RES - BABY_DISP_H);

    px = (lv_coord_t)lv_rand(0u, max_x);
    py = (lv_coord_t)lv_rand(0u, max_y);
    lv_obj_set_pos(s_water_gif, px, py);
    lv_gif_restart(s_water_gif);
    lv_obj_move_foreground(s_water_gif);

    baby_start_move_internal();

    return 0;
}
