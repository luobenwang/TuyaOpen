/*
 * linux/gpio.h - compatibility UAPI header for the bundled sunxi toolchain.
 *
 * The bundled OpenWrt sunxi kernel headers predate the GPIO character device
 * UAPI, so <linux/gpio.h> is missing. This file provides the stable v1 GPIO
 * chardev ABI (struct layouts and ioctl numbers) exactly as defined by the
 * upstream Linux kernel uapi, so code built against it is binary compatible
 * with the GPIO chardev interface on the target T113 kernel (>= 4.8).
 *
 * Only the v1 handle/event interface used by the TuyaOpen Linux adapter is
 * provided here.
 */
#ifndef _COMPAT_UAPI_GPIO_H_
#define _COMPAT_UAPI_GPIO_H_

#include <linux/ioctl.h>
#include <linux/types.h>

/* Maximum number of requested lines. */
#define GPIOHANDLES_MAX 64

/* Line request flags for struct gpiohandle_request. */
#define GPIOHANDLE_REQUEST_INPUT          (1UL << 0)
#define GPIOHANDLE_REQUEST_OUTPUT         (1UL << 1)
#define GPIOHANDLE_REQUEST_ACTIVE_LOW     (1UL << 2)
#define GPIOHANDLE_REQUEST_OPEN_DRAIN     (1UL << 3)
#define GPIOHANDLE_REQUEST_OPEN_SOURCE    (1UL << 4)
#define GPIOHANDLE_REQUEST_BIAS_PULL_UP   (1UL << 5)
#define GPIOHANDLE_REQUEST_BIAS_PULL_DOWN (1UL << 6)
#define GPIOHANDLE_REQUEST_BIAS_DISABLE   (1UL << 7)

struct gpiohandle_request {
    __u32 lineoffsets[GPIOHANDLES_MAX];
    __u32 flags;
    __u8 default_values[GPIOHANDLES_MAX];
    char consumer_label[32];
    __u32 lines;
    int fd;
};

struct gpiohandle_config {
    __u32 flags;
    __u8 default_values[GPIOHANDLES_MAX];
    __u32 padding[4];
};

struct gpiohandle_data {
    __u8 values[GPIOHANDLES_MAX];
};

#define GPIOHANDLE_GET_LINE_VALUES_IOCTL _IOWR(0xB4, 0x08, struct gpiohandle_data)
#define GPIOHANDLE_SET_LINE_VALUES_IOCTL _IOWR(0xB4, 0x09, struct gpiohandle_data)

/* Event request flags for struct gpioevent_request. */
#define GPIOEVENT_REQUEST_RISING_EDGE  (1UL << 0)
#define GPIOEVENT_REQUEST_FALLING_EDGE (1UL << 1)
#define GPIOEVENT_REQUEST_BOTH_EDGES   ((1UL << 0) | (1UL << 1))

struct gpioevent_request {
    __u32 lineoffset;
    __u32 handleflags;
    __u32 eventflags;
    char consumer_label[32];
    int fd;
};

/* GPIO event types delivered through the event file descriptor. */
#define GPIOEVENT_EVENT_RISING_EDGE  0x01
#define GPIOEVENT_EVENT_FALLING_EDGE 0x02

struct gpioevent_data {
    __u64 timestamp;
    __u32 id;
};

#define GPIO_GET_LINEHANDLE_IOCTL _IOWR(0xB4, 0x03, struct gpiohandle_request)
#define GPIO_GET_LINEEVENT_IOCTL  _IOWR(0xB4, 0x04, struct gpioevent_request)

#endif /* _COMPAT_UAPI_GPIO_H_ */
