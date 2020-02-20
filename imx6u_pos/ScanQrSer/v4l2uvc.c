#include "system.h"
#include <sys/mman.h>
#include "v4l2uvc.h"
#include "a.h"

//初始化摄像头组件
int v4l2_init(video_t *v)
{
	int i;
	int ret;

    //打开摄像头设备
	if ((v->fd = open(v->dev_name, O_RDWR)) < 0) {
		myPtf("ERROR open V4L interface %s\n", v->dev_name);
		perror("open ");
		return -1;
	}
	memset(&v->cap, 0, sizeof(struct v4l2_capability));
	//读取当前设备信息
	ret = ioctl(v->fd, VIDIOC_QUERYCAP, &v->cap);
	if (ret < 0) {
		myPtf("Error opening device %s: unable to query device.\n",
				v->dev_name);
		perror("VIDIOC_QUERYCAP ");
		goto fatal;
	}
    //检查该设备是否能够捕获图片
	if ((v->cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) == 0) {
		myPtf("Error opening device %s: video capture not supported.\n",
				v->dev_name);
		goto fatal;;
	}
    //检测该设备支持的是流模式还是读写模式或者其他模式
    //目前只有流模式可以继续，其它模式都会报错
	if (v->cap.capabilities & V4L2_CAP_STREAMING) {
		myPtf("%s support streaming\n", v->dev_name);
	} else if (v->cap.capabilities & V4L2_CAP_READWRITE) {
		myPtf("%s support read write\n", v->dev_name);
		/* not support readwrite only */
		goto fatal;
	} else {
		myPtf("%s support unknown %x\n", v->dev_name, v->cap.capabilities);
		goto fatal;
	}

	// Enumerate the supported formats to check whether the requested one
	// is available. If not, we try to fall back to YUYV.
	unsigned int device_formats[16] = { 0 };	// Assume no device supports more than 16 formats
	int requested_format_found = 0, fallback_format = -1;
	//枚举设备支持的模式？
	if(enum_frame_formats(v->fd, device_formats, ARRAY_SIZE(device_formats))) {
		myPtf("Unable to enumerate frame formats");
		goto fatal;
	}
	//找到对应支持的模式
	for(i = 0; i < ARRAY_SIZE(device_formats) && device_formats[i]; i++) {
		if(device_formats[i] == v->pixelformat) {
			requested_format_found = 1;
			myPtf("find format[ver:1.02] %d!\n",v->pixelformat);
			break;
		}
//		if(device_formats[i] == V4L2_PIX_FMT_MJPEG || device_formats[i] == V4L2_PIX_FMT_YUYV || device_formats[i] == V4L2_PIX_FMT_RGB24)
//			fallback_format = i;
	}
	//没找到支持模式，退出
	if(!requested_format_found) {
		// The requested format is not supported
		myPtf("  Frame format: "FOURCC_FORMAT" not supported\n", FOURCC_ARGS(v->pixelformat));
		goto fatal;
	}

	//Set pixel format and frame size
	//设置图片格式，填充方式
	memset(&v->fmt, 0, sizeof(struct v4l2_format));
	v->fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	v->fmt.fmt.pix.width = v->width;
	v->fmt.fmt.pix.height = v->height;
	v->fmt.fmt.pix.pixelformat = v->pixelformat;
	v->fmt.fmt.pix.field = V4L2_FIELD_ANY;
	//v->fmt.fmt.pix.field = V4L2_FIELD_TOP;
	myPtf("set v->fmt.fmt: width=%d,height=%d,pixelformat:=%c%c%c%c,%d,field=%d\n",v->fmt.fmt.pix.width,v->fmt.fmt.pix.height,
        v->fmt.fmt.pix.pixelformat & 0xFF,(v->fmt.fmt.pix.pixelformat >> 8) & 0xFF,
        (v->fmt.fmt.pix.pixelformat >> 16) & 0xFF,(v->fmt.fmt.pix.pixelformat >> 24) & 0xFF,
        v->fmt.fmt.pix.pixelformat,
        v->fmt.fmt.pix.field);
	ret = ioctl(v->fd, VIDIOC_S_FMT, &v->fmt);
	if (ret < 0) {
		perror("Unable to set format");
		goto fatal;
	}
	ret = ioctl(v->fd, VIDIOC_G_FMT, &v->fmt);
	if (ret < 0) {
		perror("Unable to get format");
		goto fatal;
	}
	myPtf("get v->fmt.fmt: width=%d,height=%d,pixelformat:=%c%c%c%c,%d,field=%d\n",v->fmt.fmt.pix.width,v->fmt.fmt.pix.height,
        v->fmt.fmt.pix.pixelformat & 0xFF,(v->fmt.fmt.pix.pixelformat >> 8) & 0xFF,
        (v->fmt.fmt.pix.pixelformat >> 16) & 0xFF,(v->fmt.fmt.pix.pixelformat >> 24) & 0xFF,
        v->fmt.fmt.pix.pixelformat,
        v->fmt.fmt.pix.field);
	//回读检查是否设置正确，是否支持该格式的设置
	if ((v->fmt.fmt.pix.width != v->width) ||
		(v->fmt.fmt.pix.height != v->height)) {
		myPtf("  Frame size:   %ux%u (requested size %ux%u is not supported by device)\n",
			v->fmt.fmt.pix.width, v->fmt.fmt.pix.height, v->width, v->height);
		goto fatal;
		//vd->width = vd->fmt.fmt.pix.width;
		//vd->height = vd->fmt.fmt.pix.height;
		/* look the format is not part of the deal ??? */
		//vd->formatIn = vd->fmt.fmt.pix.pixelformat;
	}
	else{
	    myPtf("set v->fmt.fmt ok!\n");
	}

#if 0
	//手动曝光
	struct v4l2_control control_s;
	control_s.id = V4L2_CID_EXPOSURE_AUTO;
	control_s.value = V4L2_EXPOSURE_MANUAL;
	ioctl(vd->fd, VIDIOC_S_CTRL, &control_s);

	//设置曝光值
	struct v4l2_control control_s1;
	control_s1.id = V4L2_CID_EXPOSURE_ABSOLUTE;
	control_s1.value = 200;
	ioctl(vd->fd, VIDIOC_S_CTRL, &control_s1);
#endif

	//struct v4l2_queryctrl ctrl;
	//ctrl.type =  v4l2_ctrl_type_integer;
	//ctrl.id = v4l2_cid_exposure;
	//ret = ioctl(vd->fd, vidioc_queryctrl, &ctrl);
	//myPtf("%d\t%d\t%d\t%d\t%d\n", ret, ctrl.minimum, ctrl.maximum, ctrl.step, ctrl.default_value);

#if 0
	/* set framerate */
	struct v4l2_streamparm* setfps;
	setfps=(struct v4l2_streamparm *) calloc(1, sizeof(struct v4l2_streamparm));
	memset(setfps, 0, sizeof(struct v4l2_streamparm));
	setfps->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	setfps->parm.capture.timeperframe.numerator=1;
	setfps->parm.capture.timeperframe.denominator=vd->fps;
	ret = ioctl(vd->fd, VIDIOC_S_PARM, setfps);
	if(ret == -1) {
		perror("Unable to set frame rate");
		goto fatal;
	}
	ret = ioctl(vd->fd, VIDIOC_G_PARM, setfps);
	if(ret == 0) {
		if (setfps->parm.capture.timeperframe.numerator != 1 ||
			setfps->parm.capture.timeperframe.denominator != vd->fps) {
			//myPtf("  Frame rate:   %u/%u fps (requested frame rate %u fps is "
			//	"not supported by device)\n",
			//	setfps->parm.capture.timeperframe.denominator,
			//	setfps->parm.capture.timeperframe.numerator,
			//	vd->fps);
		}
		else {
			//myPtf("  Frame rate:   %d fps\n", vd->fps);
		}
	}
	else {
		perror("Unable to read out current frame rate");
		goto fatal;
	}
#endif
	return 0;
fatal:
	return -1;

}
//打开v4l2设备
int v4l2_on(video_t *v)
{
    int ret;
	enum v4l2_buf_type type;
    //已经打开了，直接返回
	if (v->isstreaming) {
		return 0;
	}

	int i;
	//释放原来的文件映射
	for (i = 0; i < NB_BUFFER; i++) {
		if (v->mem[i]) {
			if (munmap(v->mem[i], v->length[i]) < 0) {
				perror("munmap ");
			}
		}
	}
	/* request buffers */
	//重新设置请求缓存区
	memset(&v->rb, 0, sizeof(struct v4l2_requestbuffers));
	v->rb.count = NB_BUFFER;//缓存数量，即可保存的图片数量
	v->rb.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;//数据流类型，永远都是
	v->rb.memory = V4L2_MEMORY_MMAP;   //存储类型：目前仅支持 V4L2_MEMORY_MMAP

	ret = ioctl(v->fd, VIDIOC_REQBUFS, &v->rb);//使设置生效，（申请缓冲区）
	if (ret < 0) {
		perror("Unable to allocate buffers");
		goto fatal;
	}

	/* map the buffers */
	//把申请到的缓冲帧信息保存到mem数组中，便于以后直接读取
	for (i = 0; i < NB_BUFFER; i++) {
		memset(&v->buf, 0, sizeof(struct v4l2_buffer));
		v->buf.index = i;
		v->buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		v->buf.memory = V4L2_MEMORY_MMAP;
		ret = ioctl(v->fd, VIDIOC_QUERYBUF, &v->buf);//查询序号为i的缓冲区，得到其起始物理地址和大小
		if (ret < 0) {
			perror("Unable to query buffer");
			goto fatal;
		}
		myPtf("length: %u offset: %u\n", v->buf.length,
				v->buf.m.offset);
		v->mem[i] = mmap(0 /* start anywhere */ ,
				v->buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, v->fd,
				v->buf.m.offset);//映射内存
		if (v->mem[i] == MAP_FAILED) {
			perror("Unable to map buffer");
			goto fatal;
		}
		v->offset[i] = v->buf.m.offset;
		v->length[i] = v->buf.length;
		myPtf("Buffer mapped at address %p.\n", v->mem[i]);
	}

//#if 0
	/* Queue the buffers. */
	//反取，查询结果
	for (i = 0; i < NB_BUFFER; ++i) {//VIDIOC_QBUF:把帧放入队列VIDIOC_DQBUF:从队列中取出帧
		memset(&v->buf, 0, sizeof(struct v4l2_buffer));
		v->buf.index = i;
		v->buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		v->buf.memory = V4L2_MEMORY_MMAP;
		v->buf.m.offset = v->offset[i];
		ret = ioctl(v->fd, VIDIOC_QBUF, &v->buf);
		if (ret < 0) {
			perror("Unable to queue buffer");
			goto fatal;;
		}
        else
        {
            myPtf("v4l2 qbuf ret=%d,i=%d\n",ret,i);
        }
	}
//#endif
	myPtf("v4l2 stream on...\n");

    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    //设置流类型，开始捕捉流
    ret = ioctl(v->fd, VIDIOC_STREAMON, &type);
    if (ret < 0) {
		perror("Unable to start capture");
		return ret;
    }
    v->isstreaming = 1;
    return 0;
fatal:
	return -1;

}

//关闭v4l2设备
int v4l2_off(video_t *v)
{
    int ret;
	enum v4l2_buf_type type;

	if (v->isstreaming == 0) {
	    myPtf("v->isstreaming == 0, need not off, return\n");
		return 0;
	}

	myPtf("v4l2 stream off...\n");
    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    ret = ioctl(v->fd, VIDIOC_STREAMOFF, &type);
    if (ret < 0) {
		perror("Unable to stop capture");
		return ret;
    }
    v->isstreaming = 0;
    return 0;
}

//把图像数据（*p）写入.jpg文件中
void write_file(void *p, size_t n)
{
	static int i = 0;
	char name[32] = "";
	sprintf(name, "cap%d.jpg", i);
	i++;
	FILE *f = fopen(name, "w");
	if (f == NULL) {
		return;
	}
	fwrite(p, n, 1, f);
	fclose(f);
}

/*
 * 采集数据得到buf, 取index & bytesused
 */
int v4l2_get_mem(video_t *v)
{
	int ret;

	memset(&v->buf, 0, sizeof(v->buf));
	v->buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	v->buf.memory = V4L2_MEMORY_MMAP;
	//myPtf("get mem:VIDIOC_DQBUF %d start!\n",v->buf_index);
	ret = ioctl(v->fd, VIDIOC_DQBUF, &v->buf);
	if (ret < 0) {
		perror("Unable to capture: ");
		return ret;
	}
	myPtf("get mem:VIDIOC_DQBUF %d over!\n",v->buf_index);
	return 0;
}

/*
 * 将buf加入驱动队列
 */
int v4l2_put_mem(video_t *v)
{
	int ret;
    /*
	if (v->buf_index >= NB_BUFFER) {
		v->buf_index -= NB_BUFFER;
	}
	memset(&v->buf, 0, sizeof(struct v4l2_buffer));
	v->buf.index = v->buf_index;
	v->buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	v->buf.memory = V4L2_MEMORY_MMAP;
	v->buf.m.offset = v->offset[v->buf_index];
	*/
    //myPtf("put mem:VIDIOC_QBUF %d start!\n",v->buf_index);
	ret = ioctl(v->fd, VIDIOC_QBUF, &v->buf);
	if (ret < 0) {
		myPtf("Unable to queue buffer %d\n", v->buf.index);
		perror("");
		return -1;
	}
    myPtf("put mem:VIDIOC_QBUF %d over!\n",v->buf_index);
	return 0;
}

/*
 * 采集数据并保存
 */
 //先用v4l2_get_mem取得缓存图片，再用write_file写成jpg文件
int v4l2_cap_save(video_t *v)
{
	int ret;

	ret = v4l2_get_mem(v);
	if (ret < 0) {
		return ret;
	}

	myPtf("cap %d: %p %d\n", v->buf.index, v->mem[v->buf.index], v->buf.bytesused);
	write_file(v->mem[v->buf.index], v->buf.bytesused);
	return v4l2_put_mem(v);
}

int enum_frame_intervals(int dev, __u32 pixfmt, __u32 width, __u32 height)
{
	int ret;
	struct v4l2_frmivalenum fival;

	memset(&fival, 0, sizeof(fival));
	fival.index = 0;
	fival.pixel_format = pixfmt;
	fival.width = width;
	fival.height = height;
	myPtf("\tTime interval between frame: ");
	while ((ret = ioctl(dev, VIDIOC_ENUM_FRAMEINTERVALS, &fival)) == 0) {
		if (fival.type == V4L2_FRMIVAL_TYPE_DISCRETE) {
				myPtf("%u/%u, ",
						fival.discrete.numerator, fival.discrete.denominator);
		} else if (fival.type == V4L2_FRMIVAL_TYPE_CONTINUOUS) {
				myPtf("{min { %u/%u } .. max { %u/%u } }, ",
						fival.stepwise.min.numerator, fival.stepwise.min.numerator,
						fival.stepwise.max.denominator, fival.stepwise.max.denominator);
				break;
		} else if (fival.type == V4L2_FRMIVAL_TYPE_STEPWISE) {
				myPtf("{min { %u/%u } .. max { %u/%u } / "
						"stepsize { %u/%u } }, ",
						fival.stepwise.min.numerator, fival.stepwise.min.denominator,
						fival.stepwise.max.numerator, fival.stepwise.max.denominator,
						fival.stepwise.step.numerator, fival.stepwise.step.denominator);
				break;
		}
		fival.index++;
	}
	myPtf("\n");
	if (ret != 0 && errno != EINVAL) {
		perror("ERROR enumerating frame intervals");
		return errno;
	}

	return 0;
}

int enum_frame_sizes(int dev, __u32 pixfmt)
{
	int ret;
	struct v4l2_frmsizeenum fsize;

	memset(&fsize, 0, sizeof(fsize));
	fsize.index = 0;
	fsize.pixel_format = pixfmt;
	while ((ret = ioctl(dev, VIDIOC_ENUM_FRAMESIZES, &fsize)) == 0) {
		if (fsize.type == V4L2_FRMSIZE_TYPE_DISCRETE) {
			myPtf("{ discrete:index=%d, width = %u, height = %u }\n",
					fsize.index,fsize.discrete.width, fsize.discrete.height);
			ret = enum_frame_intervals(dev, pixfmt,
					fsize.discrete.width, fsize.discrete.height);
			if (ret != 0)
				myPtf("  Unable to enumerate frame sizes.\n");
		} else if (fsize.type == V4L2_FRMSIZE_TYPE_CONTINUOUS) {
			myPtf("{ continuous: min { width = %u, height = %u } .. "
					"max { width = %u, height = %u } }\n",
					fsize.stepwise.min_width, fsize.stepwise.min_height,
					fsize.stepwise.max_width, fsize.stepwise.max_height);
			myPtf("  Refusing to enumerate frame intervals.\n");
			break;
		} else if (fsize.type == V4L2_FRMSIZE_TYPE_STEPWISE) {
			myPtf("{ stepwise: min { width = %u, height = %u } .. "
					"max { width = %u, height = %u } / "
					"stepsize { width = %u, height = %u } }\n",
					fsize.stepwise.min_width, fsize.stepwise.min_height,
					fsize.stepwise.max_width, fsize.stepwise.max_height,
					fsize.stepwise.step_width, fsize.stepwise.step_height);
			myPtf("  Refusing to enumerate frame intervals.\n");
			break;
		}
		fsize.index++;
	}
	if (ret != 0 && errno != EINVAL) {
		perror("ERROR enumerating frame sizes");
		return errno;
	}

	return 0;
}

int enum_frame_formats(int dev, unsigned int *supported_formats, unsigned int max_formats)
{
	int ret;
	struct v4l2_fmtdesc fmt;

	memset(&fmt, 0, sizeof(fmt));
	fmt.index = 0;
	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	while ((ret = ioctl(dev, VIDIOC_ENUM_FMT, &fmt)) == 0) {
		myPtf("{ pixelformat = '%c%c%c%c', description = '%s' }\n",
				fmt.pixelformat & 0xFF, (fmt.pixelformat >> 8) & 0xFF,
				(fmt.pixelformat >> 16) & 0xFF, (fmt.pixelformat >> 24) & 0xFF,
				fmt.description);
		ret = enum_frame_sizes(dev, fmt.pixelformat);
		if(ret != 0)
			myPtf("  Unable to enumerate frame sizes.\n");
		if(fmt.index < max_formats) {
			supported_formats[fmt.index] = fmt.pixelformat;
		}

		fmt.index++;
	}
	if (errno != EINVAL) {
		perror("ERROR enumerating frame formats");
		return errno;
	}

	return 0;
}


