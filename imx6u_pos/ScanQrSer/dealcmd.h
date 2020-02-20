#ifndef DEALCMD_H
#define DEALCMD_H

#include <stddef.h>

#define v4l_dev "/dev/video1"

enum pix_fmt {
	PIX_FMT_YUYV = 0x30,	// YUYV格式
	PIX_FMT_MJPEG,			// MJPEG格式
};

typedef struct _my_video {
	enum pix_fmt fmt;		// 像素格式
	int width;
	int height;
	int bufindex;
	size_t len;
	char * buf;
} my_video;

my_video my_v;


//处理函数 开始捕获
char * fn_dealCmd_BeginCap(char * recv_buf,int recv_len,int * ret_len);
//处理函数 扫码
char * fn_dealCmd_GetQRCode(char * recv_buf,int recv_len,int * ret_len);
//取消处理解码
char * fn_dealCmd_StopQRCode(char * recv_buf,int recv_len,int * ret_len);
//处理函数 停止捕获
char * fn_dealCmd_EndCap(char * recv_buf,int recv_len,int * ret_len);
//处理函数 获取版本号
char * fn_dealCmd_GetVersion(char * recv_buf,int recv_len,int * ret_len);

void fn_v4l2_init();
//初始化程序
int fn_dealCmd_init();
int fn_dealCmd_destroy();
#endif // DEALCMD_H
