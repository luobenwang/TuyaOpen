/**
 * @file main.c
 * @brief Minimal toolchain smoke test for Allwinner T113 (glibc).
 * @version 1.0
 * @date 2026-06-01
 * @copyright Copyright (c) 2026 Tuya Inc.
 */
#include <stdio.h>

/**
 * @brief Entry point: print a greeting and exit.
 * @return 0 on success
 */
int main(void)
{
    printf("hello world\n");
    fflush(stdout);
    return 0;
}
