/**
 * @file tal_network.c
 * @brief Network utilities implementation for Tuya SDK.
 *
 * This source file implements the network utilities for the Tuya SDK, providing
 * a layer of abstraction over different network operations such as socket
 * creation, configuration, and communication. It supports various operating
 * systems and networking libraries, including POSIX-compliant systems and
 * systems that use the LwIP networking stack.
 *
 * The file includes conditional compilation sections to include different
 * header files and define macros based on the operating system or the
 * networking library in use. For POSIX-compliant systems, standard networking
 * headers like <sys/socket.h>, <netinet/in.h>, and <arpa/inet.h> are included.
 * For systems using the LwIP networking stack, relevant LwIP headers like
 * "lwip/netdb.h" and "lwip/dns.h" are included.
 *
 * Additionally, the file defines macros to facilitate network operations across
 * different platforms, such as `ENABLE_BIND_INTERFACE` for binding operations
 * and `NET_USING_POSIX` to indicate the use of POSIX networking APIs. It aims
 * to provide a consistent and efficient networking interface for Tuya-based
 * applications across different hardware and software environments.
 *
 * @note This file is part of the Tuya IoT Development Platform and is intended
 * for use in Tuya-based applications. It requires configuration through
 * "tuya_iot_config.h" and interfaces with other parts of the Tuya SDK through
 * "tal_api.h".
 *
 * @copyright Copyright (c) 2021-2024 Tuya Inc. All Rights Reserved.
 *
 */
#include "tuya_iot_config.h"
#include "tal_api.h"

#if 100 == OPERATING_SYSTEM
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <errno.h>
#include <fcntl.h>

#define ENABLE_BIND_INTERFACE 1

#elif defined(ENABLE_LIBLWIP) && (ENABLE_LIBLWIP == 1)
#include "lwip/netdb.h"
#include "lwip/dns.h"
#include "lwip/errno.h"
#else
#include "tkl_network.h"
#endif

#if (defined(ENABLE_LIBLWIP) && (ENABLE_LIBLWIP == 1)) || 100 == OPERATING_SYSTEM
#define NET_USING_POSIX        1
#define TAL_TO_SYS_FD_SET(fds) ((fd_set *)fds)
#else
#define NET_USING_TKL 1
#endif

#if NET_USING_POSIX
typedef struct NETWORK_ERRNO_TRANS {
    int sys_err;
    int priv_err;
} NETWORK_ERRNO_TRANS_S;

const NETWORK_ERRNO_TRANS_S unw_errno_trans[] = {{EINTR, UNW_EINTR},
                                                 {EBADF, UNW_EBADF},
                                                 {EAGAIN, UNW_EAGAIN},
                                                 {EFAULT, UNW_EFAULT},
                                                 {EBUSY, UNW_EBUSY},
                                                 {EINVAL, UNW_EINVAL},
                                                 {ENFILE, UNW_ENFILE},
                                                 {EMFILE, UNW_EMFILE},
                                                 {ENOSPC, UNW_ENOSPC},
                                                 {EPIPE, UNW_EPIPE},
                                                 {EWOULDBLOCK, UNW_EWOULDBLOCK},
                                                 {ENOTSOCK, UNW_ENOTSOCK},
                                                 {ENOPROTOOPT, UNW_ENOPROTOOPT},
                                                 {EADDRINUSE, UNW_EADDRINUSE},
                                                 {EADDRNOTAVAIL, UNW_EADDRNOTAVAIL},
                                                 {ENETDOWN, UNW_ENETDOWN},
                                                 {ENETUNREACH, UNW_ENETUNREACH},
                                                 {ENETRESET, UNW_ENETRESET},
                                                 {ECONNRESET, UNW_ECONNRESET},
                                                 {ENOBUFS, UNW_ENOBUFS},
                                                 {EISCONN, UNW_EISCONN},
                                                 {ENOTCONN, UNW_ENOTCONN},
                                                 {ETIMEDOUT, UNW_ETIMEDOUT},
                                                 {ECONNREFUSED, UNW_ECONNREFUSED},
                                                 {EHOSTDOWN, UNW_EHOSTDOWN},
                                                 {EHOSTUNREACH, UNW_EHOSTUNREACH},
                                                 {ENOMEM, UNW_ENOMEM},
                                                 {EMSGSIZE, UNW_EMSGSIZE}};
#endif

/**
 * @brief Get error code of network
 *
 * @param void
 *
 * @note This API is used for getting error code of network.
 *
 * @return 0 on success. Others on error, please refer to the error no of the
 * target system
 */
TUYA_ERRNO tal_net_get_errno(void)
{
    int sys_err;

#if NET_USING_POSIX
    int i = 0;
    sys_err = errno;

    for (i = 0; i < sizeof(unw_errno_trans) / sizeof(unw_errno_trans[0]); i++) {
        if (unw_errno_trans[i].sys_err == sys_err) {
            return unw_errno_trans[i].priv_err;
        }
    }

#else
    //! TODO:
    sys_err = tkl_net_get_errno();
#endif

    return -100 - sys_err;
}

/**
 * @brief Add file descriptor to set
 *
 * @param[in] fd: file descriptor
 * @param[in] fds: set of file descriptor
 *
 * @note This API is used to add file descriptor to set.
 *
 * @return OPRT_OK on success. Others on error, please refer to
 * tuya_error_code.h
 */
OPERATE_RET tal_net_fd_set(int fd, TUYA_FD_SET_T *fds)
{
    int ret = OPRT_OK;

    if ((fd < 0) || (fds == NULL)) {
        return -3000 + fd;
    }

#if NET_USING_POSIX
    FD_SET(fd, TAL_TO_SYS_FD_SET(fds));
#else
    ret = tkl_net_fd_set(fd, fds);
#endif

    return ret;
}

/**
 * @brief Clear file descriptor from set
 *
 * @param[in] fd: file descriptor
 * @param[in] fds: set of file descriptor
 *
 * @note This API is used to clear file descriptor from set.
 *
 * @return OPRT_OK on success. Others on error, please refer to
 * tuya_error_code.h
 */
OPERATE_RET tal_net_fd_clear(int fd, TUYA_FD_SET_T *fds)
{
    int ret = OPRT_OK;

    if ((fd < 0) || (fds == NULL)) {
        return -3000 + fd;
    }

#if NET_USING_POSIX
    FD_CLR(fd, TAL_TO_SYS_FD_SET(fds));
#else
    ret = tkl_net_fd_clear(fd, fds);
#endif

    return ret;
}

/**
 * @brief Check file descriptor is in set
 *
 * @param[in] fd: file descriptor
 * @param[in] fds: set of file descriptor
 *
 * @note This API is used to check the file descriptor is in set.
 *
 * @return TRUE or FALSE
 */
OPERATE_RET tal_net_fd_isset(int fd, TUYA_FD_SET_T *fds)
{
    int ret = FALSE;

#if NET_USING_POSIX
    ret = FD_ISSET(fd, TAL_TO_SYS_FD_SET(fds));
#else
    ret = tkl_net_fd_isset(fd, fds);
#endif
    return ret;
}

/**
 * @brief Clear all file descriptor in set
 *
 * @param[in] fds: set of file descriptor
 *
 * @note This API is used to clear all file descriptor in set.
 *
 * @return OPRT_OK on success. Others on error, please refer to
 * tuya_error_code.h
 */
OPERATE_RET tal_net_fd_zero(TUYA_FD_SET_T *fds)
{
    int ret = OPRT_OK;

    if (fds == NULL) {
        return 0xFFFFFFFF;
    }

#if NET_USING_POSIX
    FD_ZERO(TAL_TO_SYS_FD_SET(fds));
#else
    ret = tkl_net_fd_zero(fds);
#endif

    return ret;
}

/**
 * @brief Get available file descriptors
 *
 * @param[in] maxfd: max count of file descriptor
 * @param[out] readfds: a set of readalbe file descriptor
 * @param[out] writefds: a set of writable file descriptor
 * @param[out] errorfds: a set of except file descriptor
 * @param[in] ms_timeout: time out
 *
 * @note This API is used to get available file descriptors.
 *
 * @return the count of available file descriptors.
 */
int tal_net_select(const int maxfd, TUYA_FD_SET_T *readfds, TUYA_FD_SET_T *writefds, TUYA_FD_SET_T *errorfds,
                   const uint32_t ms_timeout)
{
    int ret = -1;

    if (maxfd <= 0) {
        return maxfd;
    }

#if NET_USING_POSIX
    struct timeval *tmp = NULL;
    struct timeval timeout = {ms_timeout / 1000, (ms_timeout % 1000) * 1000};
    tmp = ms_timeout ? &timeout : NULL;
    ret = select(maxfd, TAL_TO_SYS_FD_SET(readfds), TAL_TO_SYS_FD_SET(writefds), TAL_TO_SYS_FD_SET(errorfds), tmp);
#else
    ret = tkl_net_select(maxfd, readfds, writefds, errorfds, ms_timeout);
#endif

    return ret;
}

/**
 * @brief Get no block file descriptors
 *
 * @param[in] fd: file descriptor
 *
 * @note This API is used to get no block file descriptors.
 *
 * @return the count of no block file descriptors.
 */
int tal_net_get_nonblock(const int fd)
{
    int ret = 0;

    if (fd < 0) {
        return -3000 + fd;
    }

#if NET_USING_POSIX
    if ((fcntl(fd, F_GETFL, 0) & O_NONBLOCK) == O_NONBLOCK) {
        ret = 1;
    }
#else
    ret = tkl_net_get_nonblock(fd);
#endif

    return ret;
}

/**
 * @brief Set block flag for file descriptors
 *
 * @param[in] fd: file descriptor
 * @param[in] block: block flag
 *
 * @note This API is used to set block flag for file descriptors.
 *
 * @return OPRT_OK on success. Others on error, please refer to
 * tuya_error_code.h
 */
OPERATE_RET tal_net_set_block(const int fd, const BOOL_T block)
{
    int ret = 0;

    if (fd < 0) {
        return -3000 + fd;
    }

#if NET_USING_POSIX
    int flags = fcntl(fd, F_GETFL, 0);
    if (block) {
        flags &= (~O_NONBLOCK);
    } else {
        flags |= O_NONBLOCK;
    }

    if (fcntl(fd, F_SETFL, flags) < 0) {
        ret = -1;
    }

    return 0;
#else
    ret = tkl_net_set_block(fd, block);
#endif

    return ret;
}

/**
 * @brief Close file descriptors
 *
 * @param[in] fd: file descriptor
 *
 * @note This API is used to close file descriptors.
 *
 * @return 0 on success. Others on error, please refer to the error no of the
 * target system
 */
TUYA_ERRNO tal_net_close(const int fd)
{
    int ret = 0;

    if (fd < 0) {
        return -3000 + fd;
    }

#if NET_USING_POSIX
    ret = close(fd);
#else
    ret = tkl_net_close(fd);
#endif

    return ret;
}

/**
 * @brief Create a tcp/udp socket
 *
 * @param[in] type: protocol type, tcp or udp
 *
 * @note This API is used for creating a tcp/udp socket.
 *
 * @return file descriptor
 */
int tal_net_socket_create(const TUYA_PROTOCOL_TYPE_E type)
{
    int fd = -1;

#if NET_USING_POSIX
    if (PROTOCOL_TCP == type) {
        fd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    } else if (PROTOCOL_RAW == type) {
        fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    } else {
        fd = socket(AF_INET, SOCK_DGRAM, 0);
    }
#else
    fd = tkl_net_socket_create(type);
#endif

    return fd;
}

/**
 * @brief Connect to network
 *
 * @param[in] fd: file descriptor
 * @param[in] addr: address information of server
 * @param[in] port: port information of server
 *
 * @note This API is used for connecting to network.
 *
 * @return 0 on success. Others on error, please refer to the error no of the
 * target system
 */
TUYA_ERRNO tal_net_connect(const int fd, const TUYA_IP_ADDR_T addr, const uint16_t port)
{
    int ret = -1;

    if (fd < 0) {
        return -3000 + fd;
    }

#if NET_USING_POSIX
    struct sockaddr_in sock_addr;
    uint16_t tmp_port = port;
    TUYA_IP_ADDR_T tmp_addr = addr;

    sock_addr.sin_family = AF_INET;
    sock_addr.sin_port = htons(tmp_port);
    sock_addr.sin_addr.s_addr = htonl(tmp_addr);

    ret = connect(fd, (struct sockaddr *)&sock_addr, sizeof(struct sockaddr_in));
#else
    ret = tkl_net_connect(fd, addr, port);
#endif

    return ret;
}

/**
 * @brief Connect to network with raw data
 *
 * @param[in] fd: file descriptor
 * @param[in] p_socket: raw socket data
 * @param[in] len: data lenth
 *
 * @note This API is used for connecting to network with raw data.
 *
 * @return 0 on success. Others on error, please refer to the error no of the
 * target system
 */
TUYA_ERRNO tal_net_connect_raw(const int fd, void *p_socket_addr, const int len)
{
    int ret = -1;

    if (fd < 0) {
        return -3000 + fd;
    }

#if NET_USING_POSIX
    ret = connect(fd, (struct sockaddr *)p_socket_addr, len);
#else
    ret = tkl_net_connect_raw(fd, p_socket_addr, len);
#endif

    return ret;
}

#if defined(ENABLE_BIND_INTERFACE) && 1 == ENABLE_BIND_INTERFACE
static TUYA_ERRNO __bind_interface(const int fd, const TUYA_IP_ADDR_T addr)
{
    int ret = 0;
    int i = 0;
    int sock_fd;
    struct sockaddr_in *sin;
    struct ifreq ifr;

    struct if_nameindex *name_list = if_nameindex();
    if (NULL == name_list) {
        printf("name_list NULL\n");
        return -1;
    }

    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        printf("socket create fail\n");
        return -2;
    }

    for (i = 0; name_list[i].if_index != 0; i++) {
        if (strcmp(name_list[i].if_name, "lo") == 0) {
            continue;
        }

        memset(&ifr, 0, sizeof(ifr));
        strncpy(ifr.ifr_name, name_list[i].if_name, sizeof(ifr.ifr_name) - 1);
        if ((ret = ioctl(sock_fd, SIOCGIFADDR, &ifr)) < 0) {
            continue;
        }

        sin = (struct sockaddr_in *)&ifr.ifr_addr;
        if (sin->sin_addr.s_addr == htonl(addr)) {
            ret = setsockopt(fd, SOL_SOCKET, SO_BINDTODEVICE, name_list[i].if_name, sizeof(ifr.ifr_name));
            break;
        }
    }
    close(sock_fd);
    if_freenameindex(name_list);

    return ret;
}
#endif

/**
 * @brief Bind to network
 *
 * @param[in] fd: file descriptor
 * @param[in] addr: address information of server
 * @param[in] port: port information of server
 *
 * @note This API is used for binding to network.
 *
 * @return 0 on success. Others on error, please refer to the error no of the
 * target system
 */
TUYA_ERRNO tal_net_bind(const int fd, const TUYA_IP_ADDR_T addr, const uint16_t port)
{
    int ret = -1;

    if (fd < 0) {
        return -3000 + fd;
    }

#if NET_USING_POSIX
    uint16_t tmp_port = port;
    TUYA_IP_ADDR_T tmp_addr = addr;

    struct sockaddr_in sock_addr;
    sock_addr.sin_family = AF_INET;
    sock_addr.sin_port = htons(tmp_port);
    sock_addr.sin_addr.s_addr = htonl(tmp_addr);

    ret = bind(fd, (struct sockaddr *)&sock_addr, sizeof(struct sockaddr_in));
#if ENABLE_BIND_INTERFACE
    if ((0 == ret) && (addr != INADDR_ANY)) {
        if (0 != __bind_interface(fd, addr)) {
            printf("name_list NULL\n");
        }
    }
#endif
#else
    ret = tkl_net_bind(fd, addr, port);
#endif

    return ret;
}

/**
 * @brief Listen to network
 *
 * @param[in] fd: file descriptor
 * @param[in] backlog: max count of backlog connection
 *
 * @note This API is used for listening to network.
 *
 * @return 0 on success. Others on error, please refer to the error no of the
 * target system
 */
TUYA_ERRNO tal_net_listen(const int fd, const int backlog)
{
    int ret = -1;

    if (fd < 0) {
        return -3000 + fd;
    }

#if NET_USING_POSIX
    ret = listen(fd, backlog);
#else
    ret = tkl_net_listen(fd, backlog);
#endif

    return ret;
}

/**
 * @brief Listen to network
 *
 * @param[in] fd: file descriptor
 * @param[out] addr: the accept ip addr
 * @param[out] port: the accept port number
 *
 * @note This API is used for listening to network.
 *
 * @return 0 on success. Others on error, please refer to the error no of the
 * target system
 */
int tal_net_accept(const int fd, TUYA_IP_ADDR_T *addr, uint16_t *port)
{
    int ret = -1;

    if (fd < 0) {
        return -3000 + fd;
    }

#if NET_USING_POSIX
    struct sockaddr_in sock_addr;
    socklen_t len = sizeof(struct sockaddr_in);
    ret = accept(fd, (struct sockaddr *)&sock_addr, &len);
    if (ret < 0) {
        return -1;
    }
    if (addr) {
        *addr = ntohl((sock_addr.sin_addr.s_addr));
    }
    if (port) {
        *port = ntohs((sock_addr.sin_port));
    }

    return ret;
#else
    ret = tkl_net_accept(fd, addr, port);
#endif

    return ret;
}

/**
 * @brief Send data to network
 *
 * @param[in] fd: file descriptor
 * @param[in] buf: send data buffer
 * @param[in] nbytes: buffer lenth
 *
 * @note This API is used for sending data to network
 *
 * @return 0 on success. Others on error, please refer to the error no of the
 * target system
 */
TUYA_ERRNO tal_net_send(const int fd, const void *buf, const uint32_t nbytes)
{
    int ret = -1;

    if ((fd < 0) || (buf == NULL) || (nbytes == 0)) {
        return -3000 + fd;
    }

#if NET_USING_POSIX
    ret = send(fd, buf, nbytes, 0);
#else
    ret = tkl_net_send(fd, buf, nbytes);
#endif

    return ret;
}

/**
 * @brief Send data to specified server
 *
 * @param[in] fd: file descriptor
 * @param[in] buf: send data buffer
 * @param[in] nbytes: buffer lenth
 * @param[in] addr: address information of server
 * @param[in] port: port information of server
 *
 * @note This API is used for sending data to network
 *
 * @return 0 on success. Others on error, please refer to the error no of the
 * target system
 */
TUYA_ERRNO tal_net_send_to(const int fd, const void *buf, const uint32_t nbytes, const TUYA_IP_ADDR_T addr,
                           const uint16_t port)
{
    int ret = -1;

    if ((fd < 0) || (buf == NULL) || (nbytes == 0)) {
        return -3000 + fd;
    }

#if NET_USING_POSIX
    uint16_t tmp_port = port;
    TUYA_IP_ADDR_T tmp_addr = addr;

    struct sockaddr_in sock_addr;
    sock_addr.sin_family = AF_INET;
    sock_addr.sin_port = htons(tmp_port);
    sock_addr.sin_addr.s_addr = htonl(tmp_addr);

    ret = sendto(fd, buf, nbytes, 0, (struct sockaddr *)&sock_addr, sizeof(sock_addr));
#else
    ret = tkl_net_send_to(fd, buf, nbytes, addr, port);
#endif

    return ret;
}

/**
 * @brief Receive data from network
 *
 * @param[in] fd: file descriptor
 * @param[in] buf: receive data buffer
 * @param[in] nbytes: buffer lenth
 *
 * @note This API is used for receiving data from network
 *
 * @return 0 on success. Others on error, please refer to the error no of the
 * target system
 */
TUYA_ERRNO tal_net_recv(const int fd, void *buf, const uint32_t nbytes)
{
    int ret = -1;

    if ((fd < 0) || (buf == NULL) || (nbytes == 0)) {
        return -3000 + fd;
    }

#if NET_USING_POSIX
    ret = recv(fd, buf, nbytes, 0);
    if (ret <= 0) {
        if ((UNW_EINTR == tal_net_get_errno()) || (UNW_EAGAIN == tal_net_get_errno())) {
            tal_system_sleep(10);
            ret = recv(fd, buf, nbytes, 0);
        }
    }
#else
    ret = tkl_net_recv(fd, buf, nbytes);
#endif
    return ret;
}

/**
 * @brief Receive data from network with need size
 *
 * @param[in] fd: file descriptor
 * @param[in] buf: receive data buffer
 * @param[in] nbytes: buffer lenth
 * @param[in] nd_size: the need size
 *
 * @note This API is used for receiving data from network with need size
 *
 * @return >0 on success. Others on error
 */
int tal_net_recv_nd_size(const int fd, void *buf, const uint32_t buf_size, const uint32_t nd_size)
{
    int ret = -1;

    if ((fd < 0) || (NULL == buf) || (buf_size == 0) || (nd_size == 0) || (buf_size < nd_size)) {
        return -3000 + fd;
    }

#if NET_USING_POSIX
    uint32_t rd_size = 0;

    while (rd_size < nd_size) {
        ret = recv(fd, ((uint8_t *)buf + rd_size), nd_size - rd_size, 0);
        if (ret <= 0) {
            if (UNW_EWOULDBLOCK == tal_net_get_errno() || UNW_EINTR == tal_net_get_errno() ||
                UNW_EAGAIN == tal_net_get_errno()) {
                tal_system_sleep(10);
                continue;
            }
            break;
        }
        rd_size += ret;
    }
    if (rd_size < nd_size) {
        ret = -2;
    } else {
        ret = rd_size;
    }
#else
    ret = tkl_net_recv_nd_size(fd, buf, buf_size, nd_size);
#endif

    return ret;
}

/**
 * @brief Receive data from specified server
 *
 * @param[in] fd: file descriptor
 * @param[in] buf: receive data buffer
 * @param[in] nbytes: buffer lenth
 * @param[in] addr: address information of server
 * @param[in] port: port information of server
 *
 * @note This API is used for receiving data from specified server
 *
 * @return 0 on success. Others on error, please refer to the error no of the
 * target system
 */
TUYA_ERRNO tal_net_recvfrom(const int fd, void *buf, const uint32_t nbytes, TUYA_IP_ADDR_T *addr, uint16_t *port)
{
    int ret = -1;

    if ((fd < 0) || (buf == NULL) || (nbytes == 0)) {
        return -3000 + fd;
    }

#if NET_USING_POSIX
    struct sockaddr_in sock_addr;
    socklen_t addr_len = sizeof(sock_addr);
    ret = recvfrom(fd, buf, nbytes, 0, (struct sockaddr *)&sock_addr, &addr_len);
    if (ret <= 0) {
        return ret;
    }
    if (addr) {
        *addr = ntohl(sock_addr.sin_addr.s_addr);
    }
    if (port) {
        *port = ntohs(sock_addr.sin_port);
    }
#else
    ret = tkl_net_recvfrom(fd, buf, nbytes, addr, port);
#endif

    return ret;
}

/**
 * @brief Set socket options
 *
 * @param[in] fd: file descriptor
 * @param[in] level: setting level
 * @param[in] optname: the name of the option
 * @param[in] optval: the value of option
 * @param[in] optlen: the length of the option value
 *
 * @note This API is used for setting socket options.
 *
 * @return OPRT_OK on success. Others on error, please refer to
 * tuya_error_code.h
 */
OPERATE_RET tal_net_setsockopt(const int fd, const TUYA_OPT_LEVEL level, const TUYA_OPT_NAME optname,
                               const void *optval, const int optlen)
{
    int ret = -1;

#if NET_USING_POSIX
    ret = setsockopt(fd, level, optname, optval, optlen);
#else
    ret = tkl_net_setsockopt(fd, level, optname, optval, optlen);
#endif

    return ret;
}

/**
 * @brief Get socket options
 *
 * @param[in] fd: file descriptor
 * @param[in] level: getting level
 * @param[in] optname: the name of the option
 * @param[out] optval: the value of option
 * @param[out] optlen: the length of the option value
 *
 * @note This API is used for getting socket options.
 *
 * @return OPRT_OK on success. Others on error, please refer to
 * tuya_error_code.h
 */
OPERATE_RET tal_net_getsockopt(const int fd, const TUYA_OPT_LEVEL level, const TUYA_OPT_NAME optname, void *optval,
                               int *optlen)
{
    int ret = -1;

#if NET_USING_POSIX
    ret = getsockopt(fd, level, optname, optval, (socklen_t *)optlen);
#else
    ret = tkl_net_getsockopt(fd, level, optname, optval, optlen);
#endif

    return ret;
}

/**
 * @brief Set timeout option of socket fd
 *
 * @param[in] fd: file descriptor
 * @param[in] ms_timeout: timeout in ms
 * @param[in] type: transfer type, receive or send
 *
 * @note This API is used for setting timeout option of socket fd.
 *
 * @return OPRT_OK on success. Others on error, please refer to
 * tuya_error_code.h
 */
OPERATE_RET tal_net_set_timeout(const int fd, const int ms_timeout, const TUYA_TRANS_TYPE_E type)
{
    int ret = -1;

    if (fd < 0) {
        return -3000 + fd;
    }

#if NET_USING_POSIX
    // NOTE： use TUYA_timeval avoid conflict with sys/time.h
    struct timeval timeout = {ms_timeout / 1000, (ms_timeout % 1000) * 1000};
    int optname = ((type == TRANS_RECV) ? SO_RCVTIMEO : SO_SNDTIMEO);

    ret = tal_net_setsockopt(fd, SOL_SOCKET, optname, (const char *)&timeout, sizeof(timeout));
#else
    ret = tkl_net_set_timeout(fd, ms_timeout, type);
#endif

    return ret;
}

/**
 * @brief Set buffer_size option of socket fd
 *
 * @param[in] fd: file descriptor
 * @param[in] buf_size: buffer size in byte
 * @param[in] type: transfer type, receive or send
 *
 * @note This API is used for setting buffer_size option of socket fd.
 *
 * @return OPRT_OK on success. Others on error, please refer to
 * tuya_error_code.h
 */
OPERATE_RET tal_net_set_bufsize(const int fd, const int buf_size, const TUYA_TRANS_TYPE_E type)
{
    int ret = -1;

    if (fd < 0) {
        return -3000 + fd;
    }

#if NET_USING_POSIX
    int size = buf_size;
    int optname = ((type == TRANS_RECV) ? SO_RCVBUF : SO_SNDBUF);

    ret = tal_net_setsockopt(fd, SOL_SOCKET, optname, (const char *)&size, sizeof(size));
#else
    ret = tkl_net_set_bufsize(fd, buf_size, type);
#endif

    return ret;
}

/**
 * @brief Enable reuse option of socket fd
 *
 * @param[in] fd: file descriptor
 *
 * @note This API is used to enable reuse option of socket fd.
 *
 * @return OPRT_OK on success. Others on error, please refer to
 * tuya_error_code.h
 */
OPERATE_RET tal_net_set_reuse(const int fd)
{
    int ret = -1;

    if (fd < 0) {
        return -3000 + fd;
    }

#if NET_USING_POSIX
    int flag = 1;

    ret = tal_net_setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (const char *)&flag, sizeof(int));
#else
    ret = tkl_net_set_reuse(fd);
#endif

    return ret;
}

/**
 * @brief Disable nagle option of socket fd
 *
 * @param[in] fd: file descriptor
 *
 * @note This API is used to disable nagle option of socket fd.
 *
 * @return OPRT_OK on success. Others on error, please refer to
 * tuya_error_code.h
 */
OPERATE_RET tal_net_disable_nagle(const int fd)
{
    int ret = -1;

    if (fd < 0) {
        return -3000 + fd;
    }

#if NET_USING_POSIX
    int flag = 1;

    ret = tal_net_setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (const char *)&flag, sizeof(int));
#else
    ret = tkl_net_disable_nagle(fd);
#endif

    return ret;
}

/**
 * @brief Enable broadcast option of socket fd
 *
 * @param[in] fd: file descriptor
 *
 * @note This API is used to enable broadcast option of socket fd.
 *
 * @return OPRT_OK on success. Others on error, please refer to
 * tuya_error_code.h
 */
OPERATE_RET tal_net_set_broadcast(const int fd)
{
    int ret = -1;

    if (fd < 0) {
        return -3000 + fd;
    }
#if NET_USING_POSIX
    int flag = 1;
    ret = tal_net_setsockopt(fd, SOL_SOCKET, SO_BROADCAST, (const char *)&flag, sizeof(int));
#else
    ret = tkl_net_set_broadcast(fd);
#endif

    return ret;
}

/**
 * @brief Get address information by domain
 *
 * @param[in] domain: domain information
 * @param[in] addr: address information
 *
 * @note This API is used for getting address information by domain.
 *
 * @return OPRT_OK on success. Others on error, please refer to
 * tuya_error_code.h
 */
OPERATE_RET tal_net_gethostbyname(const char *domain, TUYA_IP_ADDR_T *addr)
{
    int ret = -1;

    if ((domain == NULL) || (addr == NULL)) {
        return -2;
    }

#if NET_USING_POSIX
    struct hostent *h = NULL;
    h = gethostbyname(domain);
    if (h) {
        *addr = ntohl(((struct in_addr *)(h->h_addr_list[0]))->s_addr);
        ret = OPRT_OK;
    }
#else
    ret = tkl_net_gethostbyname(domain, addr);
#endif

    return ret;
}

/**
 * @brief Set keepalive option of socket fd to monitor the connection
 *
 * @param[in] fd: file descriptor
 * @param[in] alive: keepalive option, enable or disable option
 * @param[in] idle: keep idle option, if the connection has no data exchange
 * with the idle time(in seconds), start probe.
 * @param[in] intr: keep interval option, the probe time interval.
 * @param[in] cnt: keep count option, probe count.
 *
 * @note This API is used to set keepalive option of socket fd to monitor the
 * connection.
 *
 * @return OPRT_OK on success. Others on error, please refer to
 * tuya_error_code.h
 */
OPERATE_RET tal_net_set_keepalive(int fd, const BOOL_T alive, const uint32_t idle, const uint32_t intr,
                                  const uint32_t cnt)
{
    int ret = -1;

    if (fd < 0) {
        return -3000 + fd;
    }

#if NET_USING_POSIX
    int keepalive = alive;
    int keepidle = idle;
    int keepinterval = intr;
    int keepcount = cnt;

    ret |= tal_net_setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, (void *)&keepalive, sizeof(keepalive));
    ret |= tal_net_setsockopt(fd, IPPROTO_TCP, TCP_KEEPIDLE, (void *)&keepidle, sizeof(keepidle));
    ret |= tal_net_setsockopt(fd, IPPROTO_TCP, TCP_KEEPINTVL, (void *)&keepinterval, sizeof(keepinterval));
    ret |= tal_net_setsockopt(fd, IPPROTO_TCP, TCP_KEEPCNT, (void *)&keepcount, sizeof(keepcount));
#else
    ret = tkl_net_set_keepalive(fd, alive, idle, intr, cnt);
#endif

    return ret;
}

/**
 * @brief Get ip address by socket fd
 *
 * @param[in] fd: file descriptor
 * @param[out] addr: ip address
 *
 * @note This API is used for getting ip address by socket fd.
 *
 * @return OPRT_OK on success. Others on error, please refer to
 * tuya_error_code.h
 */
OPERATE_RET tal_net_get_socket_ip(int fd, TUYA_IP_ADDR_T *addr)
{
    int ret = -1;

#if NET_USING_POSIX
    struct sockaddr_in sock_addr;
    memset(&sock_addr, 0, sizeof(sock_addr));
    socklen_t len = sizeof(sock_addr);

    if (0 == getsockname(fd, (struct sockaddr *)&sock_addr, &len)) {
        *addr = ntohl(sock_addr.sin_addr.s_addr);
        ret = OPRT_OK;
    }
#else
    ret = tkl_net_get_socket_ip(fd, addr);
#endif

    return ret;
}

/**
 * @brief Change ip string to address
 *
 * @param[in] ip_str: ip string
 *
 * @note This API is used to change ip string to address.
 *
 * @return ip address
 */
TUYA_IP_ADDR_T tal_net_str2addr(const char *ip_str)
{

#if NET_USING_POSIX
    if (ip_str == NULL) {
        return 0xFFFFFFFF;
    }

    TUYA_IP_ADDR_T addr1 = inet_addr(ip_str);
    TUYA_IP_ADDR_T addr2 = ntohl(addr1);

    return addr2;
#else
    return tkl_net_str2addr(ip_str);
#endif
}

/**
 * @brief Change ip address to string
 *
 * @param[in] ipaddr: ip address
 *
 * @note This API is used to change ip address(in host byte order) to string(in
 * IPv4 numbers-and-dots(xx.xx.xx.xx) notion).
 *
 * @return ip string
 */
char *tal_net_addr2str(TUYA_IP_ADDR_T ipaddr)
{
#if NET_USING_POSIX
    struct in_addr hostaddr;
    hostaddr.s_addr = htonl(ipaddr);
    return inet_ntoa(hostaddr);
#else
    return tkl_net_addr2str(ipaddr);
#endif
}
