#ifndef _SYSTEM_H
#define _SYSTEM_H

#include <time.h>
#include <errno.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/ioctl.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/syscall.h>
#define gettid() syscall(SYS_gettid)

#include <semaphore.h>
#include <pthread.h>
#include <math.h>


#ifndef TYPES_DEF
#define TYPES_DEF
typedef unsigned long long u64;
typedef signed long long   s64;
typedef unsigned int	   u32;
typedef signed int	       s32;
typedef unsigned short	   u16;
typedef signed short	   s16;
typedef unsigned char	    u8;
typedef signed char	        s8;
#endif

/**
 * container_of - cast a member of a structure out to the containing structure
 * @ptr:	the pointer to the member.
 * @type:	the type of the container struct this is embedded in.
 * @member:	the name of the member within the struct.
 *
 */
#define container_of(ptr, type, member) ({			\
	const typeof( ((type *)0)->member ) *__mptr = (ptr);	\
	(type *)( (char *)__mptr - offsetof(type,member) );})

#define zc 0
#define pr_debug(fmt, arg...) \
	do { \
		if (zc) \
			zlog_debug(zc, fmt, ##arg); \
	} while (0)

#define pr_info(fmt, arg...) \
	do { \
		if (zc) \
			zlog_info(zc, fmt, ##arg); \
	} while (0)

#define pr_warn(fmt, arg...) \
	do { \
		if (zc) \
			zlog_warn(zc, fmt, ##arg); \
	} while (0)

#define pr_err(fmt, arg...) \
	do { \
		if (zc) \
			zlog_error(zc, fmt, ##arg); \
	} while (0)

#define pr_notice(fmt, arg...) \
	do { \
		if (zc) \
			zlog_notice(zc, fmt, ##arg); \
	} while (0)
#define pr_hdebug(msg, buf, buf_len) do { \
	if (zc && param_get_dbg()) { \
		zlog_debug(zc, msg); \
		hzlog_debug(zc, buf, buf_len); \
		zlog_debug(zc, "\n"); \
	} \
} while (0)

#define pr_hinfo(msg, buf, buf_len) do { \
	if (zc && param_get_dbg()) { \
		zlog_info(zc, msg); \
		hzlog_info(zc, buf, buf_len); \
		zlog_info(zc, "\n"); \
	} \
} while (0)

#define pr_hwarn(msg, buf, buf_len) do { \
	if (zc && param_get_dbg()) { \
		zlog_warn(zc, msg); \
		hzlog_warn(zc, buf, buf_len); \
		zlog_warn(zc, "\n"); \
	} \
} while (0)

#define pr_herr(msg, buf, buf_len) do { \
	if (zc && param_get_dbg()) { \
		zlog_error(zc, msg); \
		hzlog_error(zc, buf, buf_len); \
		zlog_error(zc, "\n"); \
	} \
} while (0)

#define pr_hnotice(msg, buf, buf_len) do { \
	if (zc) { \
		zlog_notice(zc, msg); \
		hzlog_notice(zc, buf, buf_len); \
		zlog_notice(zc, "\n"); \
	} \
} while (0)

#define min(x, y)	((x) < (y) ? (x) : (y))
#define max(x, y)	((x) > (y) ? (x) : (y))
#define getbit(data, x) (((data) & (1<<(x)))?1:0)
#define BIT(x) (1 << x)

#endif
