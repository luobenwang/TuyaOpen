//--------------------------------------------------------------
//-- Oscillator.c
//-- Generate sinusoidal oscillations in the servos
//--------------------------------------------------------------
//-- Original work (c) Juan Gonzalez-Gomez (Obijuan), Dec 2011
//-- GPL license
//-- Ported to Tuya AI development board by [txp666], 2025
//--------------------------------------------------------------

#ifndef __OTTO_MOVEMENTS_H__
#define __OTTO_MOVEMENTS_H__

#include "tuya_cloud_types.h"
#include "tal_api.h"
#include "tkl_output.h"
#include "tkl_pwm.h"
#include "oscillator.h"

//-- Constants
#define FORWARD 1
#define BACKWARD -1
#define LEFT 1
#define RIGHT -1
#define SMALL 5
#define MEDIUM 15
#define BIG 30

// -- Servo delta limit default. degree / sec
#define SERVO_LIMIT_DEFAULT 240

// -- Servo indexes for easy access
#define LEFT_LEG 0
#define RIGHT_LEG 1
#define LEFT_FOOT 2
#define RIGHT_FOOT 3
#define SERVO_COUNT 4

// Otto结构体定义
typedef struct {
    int oscillator_indices[SERVO_COUNT];  // 存储oscillator索引
    int servo_pins[SERVO_COUNT];
    int servo_trim[SERVO_COUNT];

    unsigned long final_time;
    unsigned long partial_time;
    float increment[SERVO_COUNT];

    bool is_otto_resting;
} Otto_t;

// Otto接口函数声明
void otto_init(int left_leg, int right_leg, int left_foot, int right_foot);
void otto_attach_servos(void);
void otto_detach_servos(void);
void otto_set_trims(int left_leg, int right_leg, int left_foot, int right_foot);
void otto_move_servos(int time, int servo_target[]);
void otto_move_single(int position, int servo_number);
void otto_oscillate_servos(int amplitude[SERVO_COUNT], int offset[SERVO_COUNT], int period, double phase_diff[SERVO_COUNT], float cycle);
void otto_home(void);
bool otto_get_rest_state(void);
void otto_set_rest_state(bool state);
void otto_jump(float steps, int period);
void otto_walk(float steps, int period, int dir);
void otto_turn(float steps, int period, int dir);
void otto_bend(int steps, int period, int dir);
void otto_shake_leg(int steps, int period, int dir);
void otto_up_down(float steps, int period, int height);
void otto_swing(float steps, int period, int height);
void otto_tiptoe_swing(float steps, int period, int height);
void otto_jitter(float steps, int period, int height);
void otto_ascending_turn(float steps, int period, int height);
void otto_moonwalker(float steps, int period, int height, int dir);
void otto_crusaito(float steps, int period, int height, int dir);
void otto_flapping(float steps, int period, int height, int dir);
void otto_enable_servo_limit(int speed_limit_degree_per_sec);
void otto_disable_servo_limit(void);
void otto_execute(int amplitude[SERVO_COUNT], int offset[SERVO_COUNT], int period, double phase_diff[SERVO_COUNT], float steps);

#endif  // __OTTO_MOVEMENTS_H__ 