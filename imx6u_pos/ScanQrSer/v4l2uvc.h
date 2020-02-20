#ifndef V4L2UVC_H
#define V4L2UVC_H

#include <linux/videodev2.h>
#include "system.h"

#define NB_BUFFER 3
#define ARRAY_SIZE(a)		(sizeof(a) / sizeof((a)[0]))
#define FOURCC_FORMAT		"%c%c%c%c"
#define FOURCC_ARGS(c)		(c) & 0xFF, ((c) >> 8) & 0xFF, ((c) >> 16) & 0xFF, ((c) >> 24) & 0xFF

typedef struct _video {
	int fd;
	const char *dev_name;
	u32 pixelformat;
	u32 width;
	u32 height;
	struct v4l2_capability cap;		/* device capability */
	struct v4l2_format fmt;			/* stream data format */
	struct v4l2_buffer buf;			/* video buffer info */
	struct v4l2_requestbuffers rb;	/* memory mapping buffers */
	/* buffers */
	void *mem[NB_BUFFER];
	size_t offset[NB_BUFFER];
	size_t length[NB_BUFFER];
	int buf_index;
	int isstreaming;
} video_t;

int v4l2_init(video_t *v);
int enum_frame_formats(int dev, unsigned int *supported_formats, unsigned int max_formats);
int v4l2_on(video_t *v);
int v4l2_off(video_t *v);
int v4l2_cap_save(video_t *v);

int v4l2_get_mem(video_t *v);
int v4l2_put_mem(video_t *v);
#endif

