/**
 * @file tkl_memory.c
 * @brief Linux HAL memory: system libc allocator by default.
 * @version 0.2
 * @date 2020-05-15
 *
 * @copyright Copyright 2020-2021 Tuya Inc. All Rights Reserved.
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/sysinfo.h>
#include "tkl_memory.h"
#include "tuya_kconfig.h"

#if !defined(CONFIG_LINUX_USE_LIBC_MALLOC) || (CONFIG_LINUX_USE_LIBC_MALLOC == 1)

/* ---------------------------------------------------------------------------
 * System libc allocator (default on Linux)
 * --------------------------------------------------------------------------- */
/**
 * @brief Alloc memory of system
 * @param[in] size memory size
 * @return allocated pointer or NULL
 */
void *tkl_system_malloc(const SIZE_T size)
{
    return malloc(size);
}

/**
 * @brief Free memory of system
 * @param[in] ptr memory pointer
 * @return none
 */
void tkl_system_free(void *ptr)
{
    if (ptr != NULL) {
        free(ptr);
    }
}

/**
 * @brief Allocate and clear memory
 * @param[in] nitems number of elements
 * @param[in] size element size
 * @return allocated pointer or NULL
 */
void *tkl_system_calloc(size_t nitems, size_t size)
{
    return calloc(nitems, size);
}

/**
 * @brief Re-allocate memory
 * @param[in] ptr existing pointer
 * @param[in] size new size
 * @return reallocated pointer or NULL
 */
void *tkl_system_realloc(void *ptr, size_t size)
{
    return realloc(ptr, size);
}

/**
 * @brief Get approximate free RAM from the kernel
 * @return free bytes, or 0 on failure
 */
int tkl_system_get_free_heap_size(void)
{
    struct sysinfo info;

    if (sysinfo(&info) != 0) {
        return 0;
    }
    return (int)info.freeram;
}

#else /* CONFIG_LINUX_USE_LIBC_MALLOC */

#include "tuya_mem_heap.h"

#ifndef CONFIG_LINUX_MEM_HEAP_SIZE_MB
#define CONFIG_LINUX_MEM_HEAP_SIZE_MB 32
#endif

#define HEAP_SIZE_MB_TO_BYTES(mb) ((size_t)(mb) * 1024U * 1024U)

static pthread_mutex_t s_heap_mutex = PTHREAD_MUTEX_INITIALIZER;
static HEAP_HANDLE s_heap_handle = NULL;
static int s_heap_inited = 0;

static void __heap_lock(void)
{
    pthread_mutex_lock(&s_heap_mutex);
}

static void __heap_unlock(void)
{
    pthread_mutex_unlock(&s_heap_mutex);
}

/**
 * @brief Initialize the optional custom heap pool.
 * @return none
 */
static void __heap_init(void)
{
    size_t try_mb[] = {
        (size_t)CONFIG_LINUX_MEM_HEAP_SIZE_MB,
        32,
        16,
        8,
        4,
    };
    size_t i = 0;
    char *buf = NULL;
    size_t pool_size = 0;
    heap_context_t ctx = {0};

    if (s_heap_inited) {
        return;
    }
    s_heap_inited = 1;

    ctx.dbg_output = (void (*)(char *, ...))printf;
    ctx.enter_critical = __heap_lock;
    ctx.exit_critical = __heap_unlock;

    if (tuya_mem_heap_init(&ctx) != 0) {
        return;
    }

    for (i = 0; i < (sizeof(try_mb) / sizeof(try_mb[0])); i++) {
        if (try_mb[i] == 0) {
            continue;
        }
        pool_size = HEAP_SIZE_MB_TO_BYTES(try_mb[i]);
        buf = (char *)malloc(pool_size);
        if (buf == NULL) {
            continue;
        }
        if (tuya_mem_heap_create(buf, (unsigned int)pool_size, &s_heap_handle) == 0) {
            return;
        }
        free(buf);
    }
}

/**
 * @brief Ensure custom heap is initialized.
 * @return none
 */
static void __heap_ensure(void)
{
    if (!s_heap_inited) {
        __heap_init();
    }
}

/**
 * @brief Alloc memory from custom pool
 * @param[in] size memory size
 * @return allocated pointer or NULL
 */
void *tkl_system_malloc(const SIZE_T size)
{
    __heap_ensure();
    return tuya_mem_heap_malloc(s_heap_handle, size);
}

/**
 * @brief Free memory from custom pool
 * @param[in] ptr memory pointer
 * @return none
 */
void tkl_system_free(void *ptr)
{
    if (ptr == NULL) {
        return;
    }
    __heap_ensure();
    tuya_mem_heap_free(s_heap_handle, ptr);
}

/**
 * @brief Allocate and clear memory from custom pool
 * @param[in] nitems number of elements
 * @param[in] size element size
 * @return allocated pointer or NULL
 */
void *tkl_system_calloc(size_t nitems, size_t size)
{
    __heap_ensure();
    return tuya_mem_heap_calloc(s_heap_handle, nitems * size);
}

/**
 * @brief Re-allocate memory in custom pool
 * @param[in] ptr existing pointer
 * @param[in] size new size
 * @return reallocated pointer or NULL
 */
void *tkl_system_realloc(void *ptr, size_t size)
{
    __heap_ensure();
    return tuya_mem_heap_realloc(s_heap_handle, ptr, size);
}

/**
 * @brief Get free bytes in custom pool
 * @return free bytes
 */
int tkl_system_get_free_heap_size(void)
{
    __heap_ensure();
    return tuya_mem_heap_available(s_heap_handle);
}

#endif /* CONFIG_LINUX_USE_LIBC_MALLOC */

/**
 * @brief Set memory
 * @param[in] src destination
 * @param[in] ch fill byte
 * @param[in] n length
 * @return destination pointer
 */
TUYA_WEAK_ATTRIBUTE void *tkl_system_memset(void *src, int ch, const SIZE_T n)
{
    return memset(src, ch, n);
}

/**
 * @brief Copy memory
 * @param[in] src destination
 * @param[in] dst source
 * @param[in] n length
 * @return destination pointer
 */
TUYA_WEAK_ATTRIBUTE void *tkl_system_memcpy(void *src, const void *dst, const SIZE_T n)
{
    return memcpy(src, dst, n);
}

/**
 * @brief Alloc memory (PSRAM alias on Linux)
 * @param[in] size memory size
 * @return allocated pointer or NULL
 */
VOID_T *tkl_system_psram_malloc(CONST SIZE_T size)
{
    return tkl_system_malloc(size);
}

/**
 * @brief Free memory (PSRAM alias on Linux)
 * @param[in] ptr memory pointer
 * @return none
 */
VOID_T tkl_system_psram_free(VOID_T *ptr)
{
    tkl_system_free(ptr);
}

/**
 * @brief Calloc in PSRAM alias
 * @param[in] nitems number of elements
 * @param[in] size element size
 * @return allocated pointer or NULL
 */
VOID_T *tkl_system_psram_calloc(size_t nitems, size_t size)
{
    return tkl_system_calloc(nitems, size);
}

/**
 * @brief Realloc in PSRAM alias
 * @param[in] ptr existing pointer
 * @param[in] size new size
 * @return reallocated pointer or NULL
 */
VOID_T *tkl_system_psram_realloc(VOID_T *ptr, size_t size)
{
    return tkl_system_realloc(ptr, size);
}

/**
 * @brief Get free heap (PSRAM alias)
 * @return free bytes
 */
INT_T tkl_system_psram_get_free_heap_size(VOID_T)
{
    return tkl_system_get_free_heap_size();
}
