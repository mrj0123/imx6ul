#include <sys/mman.h>
#include "a.h"
#include "exqrscan.h"
#include "common.h"

//键盘缓存
static int  qrbufferlen;

//扫码线程ID
static pthread_t qrthreadid;

const char *sub_buff1="SM SM-2D PRODUCT HID KBW";
const char *sub_buff2="sysrq kbd";
static int exscanqrevent=-1;

static int errorkeynum=0;
static int lastscanTime=0;

int find_event()
{
    int number = -1;
    char * pkblist = NULL;
    int kbnum = 0;

    pkblist = GetKBList(&kbnum);
    myPtf("kbnum=%d\n",kbnum);
    char * pTemp;
    char * buff;
    char deviceInfo[DEV_ITEMLEN][DEV_INFOLEN];
	for(int i=0;i<kbnum;i++)
	{
	    //拷贝一个设备信息
        myPtf("pkblist=%d\n",(int)pkblist);
	    pTemp = (char *)(((int)pkblist)+i*DEV_ITEMLEN*DEV_INFOLEN);
        myPtf("pTemp=%d\n",(int)pTemp);
	    memcpy(deviceInfo,pTemp,DEV_ITEMLEN*DEV_INFOLEN);
        myPtf("deviceInfo=%d\n",(int)deviceInfo);
	    //显示该设备信息
        myPtf("i=%d,DEV_BUS=%d,info=%s",i,DEV_BUS,deviceInfo[DEV_BUS]);
        myPtf("i=%d,DEV_VENDOR=%d,info=%s",i,DEV_VENDOR,deviceInfo[DEV_VENDOR]);
        myPtf("i=%d,DEV_PORDUCT=%d,info=%s",i,DEV_PORDUCT,deviceInfo[DEV_PORDUCT]);
        myPtf("i=%d,DEV_VERSION=%d,info=%s",i,DEV_VERSION,deviceInfo[DEV_VERSION]);
        myPtf("i=%d,DEV_NAME=%d,info=%s",i,DEV_NAME,deviceInfo[DEV_NAME]);
        myPtf("i=%d,DEV_PHYS=%d,info=%s",i,DEV_PHYS,deviceInfo[DEV_PHYS]);
        myPtf("i=%d,DEV_SYSFS=%d,info=%s",i,DEV_SYSFS,deviceInfo[DEV_SYSFS]);
        myPtf("i=%d,DEV_HANDLERS=%d,info=%s\n",i,DEV_HANDLERS,deviceInfo[DEV_HANDLERS]);

        //查找SM SM-2D PRODUCT HID KBW
        buff = strstr(deviceInfo[DEV_NAME],sub_buff1);// in the mem file_buff  find sub_buff name
        if(NULL!=buff)
        {
            //查找sysrq kbd
            buff = strstr(deviceInfo[DEV_HANDLERS],sub_buff2);
            if(NULL!=buff)
            {
                myPtf("dev buf=%s\n",buff);
                number = *(buff+strlen(sub_buff2)+6)-0x30;// 6== event 再转换为ASCII对应的数字，只适用于1位数字。
            }
        }
        else
        {
            myPtf("i=%d not find dev\n",i);
        }
	}
    myPtf("free pkblist!\n");
    free(pkblist);
    myPtf("free pkblist OK!\n");
    return number;
}

int fn_reOpen()
{
    char devpath[128];
    memset(devpath,0,sizeof(devpath));
    sprintf(devpath,"%s%d",QR_DEV_PATH,find_event());
	return open(devpath, O_RDONLY);
}

//从硬件键盘读到一个输入字符
unsigned char fn_divScanQR(int * pfd)
{
	struct input_event t;
	int fd = *pfd;
	while(1)
	{
		if(read(fd, &t, sizeof(t)) == sizeof(t))
		{
		    errorkeynum = 0;
            //myPtf("All key code=%02X;value=%d;type=%d\n", t.code, t.value,t.type);
			if(t.type == EV_KEY)
            {
                //myPtf("?key %d %s\n", t.code, (t.value) ? "Pressed" : "Released");
                //myPtf("EV_KEY code=%02X;value=%d;type=%d\n", t.code, t.value,t.type);
                if (t.value == 0)
                {
                    //myPtf("Released code=%02X;value=%d;type=%d\n", t.code, t.value,t.type);
                    //myPtf("%02x", t.code);
                    lastscanTime = getNowTime();
                    return (unsigned char)(t.code);
                }
            }
		}
		else
		{
		    errorkeynum++;
		    if (errorkeynum%10 == 0)
		    {
		        usleep(100000);
                myPtf("errorkeynum = %d ,sleep\n",errorkeynum);
		    }
		    if (errorkeynum > 100)//(100/10*100000=1000000um=1m+执行时间)
		    {
		        int newfd = fn_reOpen();
                myPtf("open newfd = %d\n",newfd);
		        if (newfd > 0)
		        {
		            close(fd);
		            *pfd = newfd;
		            fd = newfd;
		        }
		        errorkeynum = 0;
		        continue;
		    }
            myPtf("read error key!\n");
		}
	}
}

//显示线程内处理的真实业务逻辑
void *fn_QRScanInThread(void *args)
{
    exscanqrevent = find_event();
    while (exscanqrevent==-1)
    {
        myPtf("read dev qr error!\n");
        sleep(5);
        exscanqrevent = find_event();
    }
    myPtf("devpath:%s%d\n",QR_DEV_PATH,exscanqrevent);
    ////////////////////////////////////////
    myPtf("thread start!\n");
    if(qrbufferlen>1024)
        return NULL;
    int keys_fd;
    char devpath[128];
    memset(devpath,0,sizeof(devpath));
    sprintf(devpath,"%s%d",QR_DEV_PATH,find_event());
	keys_fd=open(devpath, O_RDONLY);
	if(keys_fd <= 0)
	{
        myPtf("open %s device error!\n",QR_DEV_PATH);
		return NULL;
	}
    //初始化锁
    //一直循环判断读取Key，每50ms判断一次显示
    while(1)
    {
        myPtf("real read1 key keys_fd = %d\n",keys_fd);
        fn_divScanQR(&keys_fd);
        myPtf("real read2 key keys_fd = %d\n",keys_fd);
        usleep(100);
        //myPtf("sleep over\n");
    }
    close(keys_fd);
    return NULL;
}

//启动键盘线程，应在主程序初始化时调用
int fn_StartQRScanThread()
{
    //启动显示线程
    myPtf("thread is ready to start!\n");
    int ret=pthread_create(&qrthreadid,NULL,fn_QRScanInThread,NULL);
    myPtf("thread is started!\n");
    return ret;
}
//结束键盘线程，可以不要
int fn_EndQRScanThread()
{

    pthread_join(qrthreadid,NULL);
    return 0;
}

int fn_getLastTime()
{
    return lastscanTime;
}




//键值转换
char fn_ConvertBase64Key(int orgkey,int isupper)
{
    char retkey=-1;

    switch(orgkey)
    {
        case KEY_1:
            if (isupper==1)
                retkey='!';
            else
                retkey='1';
            break;
        case KEY_2:
            if (isupper==1)
                retkey='@';
            else
                retkey='2';
            break;
        case KEY_3:
            if (isupper==1)
                retkey='#';
            else
                retkey='3';
            break;
        case KEY_4:
            if (isupper==1)
                retkey='$';
            else
                retkey='4';
            break;
        case KEY_5:
            if (isupper==1)
                retkey='%';
            else
                retkey='5';
            break;
        case KEY_6:
            if (isupper==1)
                retkey='^';
            else
                retkey='6';
            break;
        case KEY_7:
            if (isupper==1)
                retkey='&';
            else
                retkey='7';
            break;
        case KEY_8:
            if (isupper==1)
                retkey='*';
            else
                retkey='8';
            break;
        case KEY_9:
            if (isupper==1)
                retkey='(';
            else
                retkey='9';
            break;
        case KEY_0:
            if (isupper==1)
                retkey=')';
            else
                retkey='0';
            break;
        case KEY_Q:
            if (isupper==1)
                retkey='Q';
            else
                retkey='q';
            break;
        case KEY_W:
            if (isupper==1)
                retkey='W';
            else
                retkey='w';
            break;
        case KEY_E:
            if (isupper==1)
                retkey='E';
            else
                retkey='e';
            break;
        case KEY_R:
            if (isupper==1)
                retkey='R';
            else
                retkey='r';
            break;
        case KEY_T:
            if (isupper==1)
                retkey='T';
            else
                retkey='t';
            break;
        case KEY_Y:
            if (isupper==1)
                retkey='Y';
            else
                retkey='y';
            break;
        case KEY_U:
            if (isupper==1)
                retkey='U';
            else
                retkey='u';
            break;
        case KEY_I:
            if (isupper==1)
                retkey='I';
            else
                retkey='i';
            break;
        case KEY_O:
            if (isupper==1)
                retkey='O';
            else
                retkey='o';
            break;
        case KEY_P:
            if (isupper==1)
                retkey='P';
            else
                retkey='p';
            break;
        case KEY_ENTER:
            retkey='\r';
            break;
        case KEY_A:
            if (isupper==1)
                retkey='A';
            else
                retkey='a';
            break;
        case KEY_S:
            if (isupper==1)
                retkey='S';
            else
                retkey='s';
            break;
        case KEY_D:
            if (isupper==1)
                retkey='D';
            else
                retkey='d';
            break;
        case KEY_F:
            if (isupper==1)
                retkey='F';
            else
                retkey='f';
            break;
        case KEY_G:
            if (isupper==1)
                retkey='G';
            else
                retkey='g';
            break;
        case KEY_H:
            if (isupper==1)
                retkey='H';
            else
                retkey='h';
            break;
        case KEY_J:
            if (isupper==1)
                retkey='J';
            else
                retkey='j';
            break;
        case KEY_K:
            if (isupper==1)
                retkey='K';
            else
                retkey='k';
            break;
        case KEY_L:
            if (isupper==1)
                retkey='L';
            else
                retkey='l';
            break;
        case KEY_Z:
            if (isupper==1)
                retkey='Z';
            else
                retkey='z';
            break;
        case KEY_X:
            if (isupper==1)
                retkey='X';
            else
                retkey='x';
            break;
        case KEY_C:
            if (isupper==1)
                retkey='C';
            else
                retkey='c';
            break;
        case KEY_V:
            if (isupper==1)
                retkey='V';
            else
                retkey='v';
            break;
        case KEY_B:
            if (isupper==1)
                retkey='B';
            else
                retkey='b';
            break;
        case KEY_N:
            if (isupper==1)
                retkey='N';
            else
                retkey='n';
            break;
        case KEY_M:
            if (isupper==1)
                retkey='M';
            else
                retkey='m';
            break;
        case KEY_LEFTSHIFT:
            retkey=KEY_MYUPPER;
            break;
        case KEY_DOWN:
            retkey=-2;
            break;
        case KEY_KPPLUS:
            retkey='+';
            break;
        case KEY_SLASH:
            if (isupper==1)
                retkey='?';
            else
                retkey='/';
            break;
        case KEY_SEMICOLON:
            if (isupper==1)
                retkey=':';
            else
                retkey=';';
            break;
        case KEY_DOT:
            retkey='.';
            break;
        case KEY_EQUAL:
            retkey='=';
            break;
        case KEY_KPMINUS:
            retkey='-';
            break;
        default:
            retkey=orgkey;
            break;
    }
    return retkey;
}
