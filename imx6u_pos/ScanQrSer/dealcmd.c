#include "dealcmd.h"
#include "v4l2uvc.h"
#include "zbar_cut.h"
#include "a.h"


video_t v = {
	.dev_name = v4l_dev,
	.width = 800,
	.height = 600,
	.pixelformat = V4L2_PIX_FMT_YUYV,//V4L2_PIX_FMT_MJPEG,
};

static int video_on=0;          //0表示未打开，1表示已打开，不需要再次打开
static int decodethread_on=SCANTHREAD_STATE_NOWORK;   //扫码解码线程是否正在运行,线程内部赋值，外部查看
static int decodethread_stop=0; //停止扫码线程，线程外部赋值，线程内部查看。
static char qrcode[128];        //识别到的二维码

void fn_v4l2_init()
{
    v4l2_init(&v);
}

//通过v4l2_put_mem和v4l2_get_mem，获取一帧图像
static int _cams_cap_image()
{
	video_t *vp = &v;
	int ret;
	ret = v4l2_get_mem(&v);
	if (ret < 0) {
		return ret;
	}
	////////////////////////////////////////////////////////
    //填写my_v并返回 modified by yangtao 20108-08-16
    my_v.width=vp->width;
    my_v.height=vp->height;
	if (vp->pixelformat == V4L2_PIX_FMT_MJPEG) {
		my_v.fmt = PIX_FMT_MJPEG;
	} else if (vp->pixelformat == V4L2_PIX_FMT_YUYV) {
		my_v.fmt = PIX_FMT_YUYV;
	}
	my_v.len=vp->buf.bytesused;
	my_v.bufindex=vp->buf.index;
	my_v.buf=malloc(my_v.len);
	memcpy(my_v.buf,vp->mem[vp->buf.index],vp->buf.bytesused);
    ////////////////////////////////////////////////////////
    //myPtf("get mem:width=%d,heigth=%d,fmt=%d\n",my_v.width,my_v.height,my_v.fmt);
	if (ret < 0)
	{
		return ret;
	}
  	ret = v4l2_put_mem(&v);
	if (ret < 0) {
		return ret;
	}
	return 0;
}

int _cap_image(void **p)
{
	int ret;
	video_t *vp = &v;

	int fd = vp->fd;
	int sizeimage=vp->width*vp->height*3;
	void *buf = malloc(sizeimage);

	ret = read(fd, buf, sizeimage);
	if (ret != sizeimage) {
		myPtf("cap image read return %d\n", ret);
	}
	//save_image(i, buf, sizeimage);
	*p = buf;
	myPtf("alloc %p %d\n", *p, sizeimage);
	return ret;
}
//抓取单帧图片并解码
char * cams_qr_oneframe_decode(int * codelen)
{
	video_t *vp = &v;
	char * ret_buf = NULL;
	int len = 0;
	void * framepic=NULL;
	int ret;
	//首先抓取一张图
	myPtf("decode:get frame image start\n");
    ret = _cap_image(&framepic);
	myPtf("decode:get frame image ret = %d\n",ret);
	////////////////////////////////////////////////////////////////////////////
	void *buf = malloc(sizeof(qr_data_t));
    memset(buf,0,sizeof(qr_data_t));
	if(zbar_decode(vp->width, vp->height, (u8 *)framepic, ret, 0, (qr_data_t *)buf)==1)
	{
	    len = strlen(((qr_data_t *)buf)->data)+1;
	    ret_buf = (char *)malloc(len);
	    memcpy(ret_buf,((qr_data_t *)buf)->data,len);
	    myPtf("qr:%s\n",((qr_data_t *)buf)->data);
	}
	////////////////////////////////////////////////////////////////////////////
	myPtf("cams_qr_oneframe_decode OK!\n");
	free(buf);
	free(framepic);
	*codelen = len;
	return ret_buf;
}
//抓图并解码
char * cams_qr_decode(int * codelen)
{
	video_t *vp = &v;
	char * ret_buf = NULL;
	int len = 0;
	*codelen = 0;
	//首先抓取一张图
	if (_cams_cap_image() < 0) {
		return NULL;
	}
	myPtf("decode:get image\n");
	////////////////////////////////////////////////////////////////////////////
	void *buf = malloc(vp->buf.bytesused + sizeof(qr_data_t));
    memset(buf,0,vp->buf.bytesused + sizeof(qr_data_t));
	if(zbar_decode(my_v.width, my_v.height, (u8 *)my_v.buf, my_v.len, 0, (qr_data_t *)buf)==1)
	{
	    len = strlen(((qr_data_t *)buf)->data)+1;
	    ret_buf = (char *)malloc(len);
	    memcpy(ret_buf,((qr_data_t *)buf)->data,len);
	    myPtf("qr:%s\n",((qr_data_t *)buf)->data);
	}
	////////////////////////////////////////////////////////////////////////////
	myPtf("cams_qr_decode OK!\n");
	free(my_v.buf);
	free(buf);
	*codelen = len;
	return ret_buf;
}

//处理函数 开始捕获
char * fn_dealCmd_BeginCap(char * recv_buf,int recv_len,int * ret_len)
{
    myPtf("fn_dealCmd_BeginCap \n");
    //开始
    if (video_on==0)
    {
        myPtf("fn_dealCmd_BeginCap video_on == 0 run v4l2_on\n");
        v4l2_on(&v);
        video_on = 1;
        myPtf("fn_dealCmd_BeginCap video_on = 1\n");
    }
    //返回
    *ret_len = 0;
    return NULL;
}

//获取版本号
char * fn_dealCmd_GetVersion(char *recv_buf,int recv_len,int * ret_len)
{
    char strTemp[128];
    sprintf(strTemp,"{\"scanqrser_version\":\"%s\"}",SCANQRSERVERSION);
    //json串错误
    char * strRet = malloc(strlen(strTemp)+1);
    memset(strRet,0,strlen(strTemp)+1);
    memcpy(strRet,strTemp,strlen(strTemp));
    *ret_len = strlen(strTemp)+1;
    //返回
    myPtf("ret_len=%d,strRet=%s\n",*ret_len,strRet);
    return strRet;
}

//二维码识别线程
static void * DecodeThread(void *arg)
{
    myPtf("start DecodeThread \n");
    //置线程打开标志
    while(decodethread_stop == 0)
    {
        if (decodethread_on != SCANTHREAD_STATE_WORKING)    //当前没有工作，线程sleep
        {
            usleep(100000);//等待100ms。
            //myPtf("not working, DecodeThread is sleep\n");
            continue;
        }
        //如果未打开，先打开摄像头
        if (video_on==0)
        {
            myPtf("video_on == 0,run v4l2_on\n");
            v4l2_on(&v);
            video_on = 1;
        }else{
            myPtf("video_on == 1,run v4l2_off and v4l2_on\n");
            v4l2_off(&v);
            usleep(1000);
            v4l2_on(&v);
        }
        myPtf("video_on = 1\n");
        char * ret_code = NULL;
        int ret_len = 0;
        //线程未被外界关闭，且未读到二维码，继续读
        while(decodethread_on == SCANTHREAD_STATE_WORKING)  //当前有任务，则一直循环抓图解码
        {
            //读二维码
            ret_code = cams_qr_decode(&ret_len);
            myPtf("in thread read qr = %s\n",ret_code);
            //如果读到
            if ((ret_len > 0)&&(ret_code!=NULL))
            {
                //将其拷贝到qrcode，并释放内存，退出线程
                memcpy(qrcode,ret_code,ret_len);
                free(ret_code);
                decodethread_on = SCANTHREAD_STATE_WORKOVER;    //任务完成，退出抓图扫码循环
                break;
            }
            usleep(66000);//每秒30帧，每帧33ms。
        }
    }
    myPtf("end DecodeThread \n");
    return (void *)NULL;
}
char * fn_dealCmd_StopQRCode(char * recv_buf,int recv_len,int * ret_len)
{
    //如果处理线程是打开状态，则关闭它
    decodethread_on = SCANTHREAD_STATE_NOWORK;  //将处理线程改为停止状态，则停止扫码
    //返回
    *ret_len = 0;
    return NULL;
}
//处理函数 扫码
char * fn_dealCmd_GetQRCode(char * recv_buf,int recv_len,int * ret_len)
{
    //读取解码数据并返回
    char * ret_buf = NULL;
    *ret_len = 0;
    //如果本次扫码未开始，则启动扫码线程工作
    if (decodethread_on == SCANTHREAD_STATE_NOWORK) //处理线程停止状态，此时有扫码请求，开始下一轮扫码
    {
        memset(qrcode,0,sizeof(qrcode));
        myPtf("qrcode thread start working!!!!!!!!!!!!!!!!\n");
        decodethread_on = SCANTHREAD_STATE_WORKING; //开始下一轮扫码
    }
    //如果已经启动，则查看是否读到码
    else if (strlen(qrcode)>0)
    {
        //已经读到码，置标志让扫码线程继续sleep，等待下一次唤醒
        myPtf("get qrcode!!!!!!!!!!!!!!!!\n");
        *ret_len = strlen(qrcode)+1;
        ret_buf = malloc(*ret_len);
        strcpy(ret_buf,qrcode);
        memset(qrcode,0,sizeof(qrcode));
        decodethread_on = SCANTHREAD_STATE_NOWORK;  //已经读到结果，且结果被取走，本次扫码彻底结束，从新回到等待下一次扫码状态
        myPtf("exit fn_dealCmd_GetQRCode\n");
    }
    else
    {
        //未读到吗，返回，扫码线程会继续读
        myPtf("DecodeThread is already run before!\n");
    }
     return ret_buf;
}

//处理函数 停止捕获
char * fn_dealCmd_EndCap(char * recv_buf,int recv_len,int * ret_len)
{
    //结束
    myPtf("fn_dealCmd_EndCap\n");
    if (video_on == 1)
    {
        myPtf("fn_dealCmd_EndCap video_on==1 run v4l2_off\n");
        v4l2_off(&v);
        video_on = 0;
        myPtf("fn_dealCmd_EndCap video_on=0 \n");
    }
    //返回
    *ret_len = 0;
    return NULL;
}

//初始化消费服务，包括初始化网络、初始化两个AF_UNIX链接客户端、同步时间、获取各程序版本号等
//
int fn_dealCmd_init()
{
    //初始化 公共资源 启动线程
    memset(qrcode,0,sizeof(qrcode));
    pthread_t thread;
    decodethread_stop = 0;
    decodethread_on = SCANTHREAD_STATE_NOWORK;  //初始化，无任务
    myPtf("create DecodeThread\n");
    int ret = pthread_create(&thread, NULL, DecodeThread, NULL);
    if (ret < 0) {
        myPtf("create consume thread error! ret = %d\n",ret);
    }
    else
    {
        myPtf("create DecodeThread OK!\n");
    }
    return 0;
}

int fn_dealCmd_destroy()
{
    //关闭串口
    decodethread_stop = 1;
    decodethread_on = SCANTHREAD_STATE_NOWORK;  //释放资源，置无任务
    if (video_on == 1)
    {
        myPtf("fn_dealCmd_EndCap video_on==1 run v4l2_off\n");
        v4l2_off(&v);
        video_on = 0;
        myPtf("fn_dealCmd_EndCap video_on=0 \n");
    }
    return 0;
}
