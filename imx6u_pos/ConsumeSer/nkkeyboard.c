#include "a.h"
#include "nkkeyboard.h"
/////////////////
//定义键盘锁
pthread_mutex_t keyboardmutex;
//键盘缓存
static char keybuffer[MAX_KEYBUFFER_LEN];
static int  keybufferlen;
static char ukeybuffer[MAX_UKEYBUFFER_LEN];
static int  ukeybufferlen;
//键盘线程ID
static pthread_t keyboardthreadid;

#define DEV_PATH "/dev/input/event0"   //difference is possible

//查找该键值是那边的
int fn_KeySide(int orgkey);
char fn_ConvertKey(int orgkey,int isupper);


//读取键盘，在主线程调用
unsigned char fn_ReadKey()
{
    int i;
    unsigned char ret;
    //如果缓冲区没数据，返回NO_KEY
    if(keybufferlen<=0)
    {
        //myPtf("lenth  is Zero\n");
        return NO_KEY;
    }
    //加锁
    pthread_mutex_lock(&keyboardmutex);
    //取出队首字符
    ret=keybuffer[0];
    //队列前移
    for(i=1;i<keybufferlen;i++)
    {
       keybuffer[i-1]=keybuffer[i];
    }
    //队尾填充NO_KEY
    keybuffer[keybufferlen--]=NO_KEY;
    //解锁
    pthread_mutex_unlock(&keyboardmutex);
    return ret;
}
//清除键盘缓冲区，在主线程调用，
//一般在开始键盘输入以前，或者读到回车或ESC完毕以后调用
int fn_ClearKeyBuff()
{
    //加锁
    pthread_mutex_lock(&keyboardmutex);
    //队列前移
    for(int i=0;i<MAX_KEYBUFFER_LEN;i++)
    {
       keybuffer[i]=NO_KEY;
    }
    keybufferlen=0;
    //解锁
    pthread_mutex_unlock(&keyboardmutex);
    return 0;
}
//清除用户键盘缓冲区，在主线程调用，
//一般在开始键盘输入以前，或者读到回车或ESC完毕以后调用
int fn_ClearUKeyBuff()
{
    //加锁
    pthread_mutex_lock(&keyboardmutex);
    //队列前移
    for(int i=0;i<MAX_UKEYBUFFER_LEN;i++)
    {
       ukeybuffer[i]=NO_KEY;
    }
    ukeybufferlen=0;
    //解锁
    pthread_mutex_unlock(&keyboardmutex);
    return 0;
}
//读取用户面键盘，在主线程调用
unsigned char fn_ReadUKey()
{
    int i;
    unsigned char ret;
    //如果缓冲区没数据，返回NO_KEY
    if(ukeybufferlen<=0)
        return NO_KEY;
    //加锁
    pthread_mutex_lock(&keyboardmutex);
    //取出队首字符
    ret=ukeybuffer[0];
    //队列前移
    for(i=1;i<ukeybufferlen;i++)
    {
       ukeybuffer[i-1]=ukeybuffer[i];
    }
    //队尾填充NO_KEY
    ukeybuffer[ukeybufferlen--]=NO_KEY;
    //解锁
    pthread_mutex_unlock(&keyboardmutex);
    return ret;
}

//从硬件键盘读到一个输入字符
unsigned char fn_divReadKey(int fd)
{
	struct input_event t;
	while(1)
	{
		if(read(fd, &t, sizeof(t)) == sizeof(t))
		{
			if(t.type==EV_KEY)
            {
                if (t.value==0)
                {
                    myPtf("key code=%02X;value=%d;type=%d\n", t.code, t.value,t.type);
                    //myPtf("key %d %s\n", t.code, "Released");
                    return (unsigned char)(t.code);
                }
            }
		}
	}
}

//显示线程内处理的真实业务逻辑
void *fn_ReadKeyInThread(void *args)
{
    myPtf("thread start!\n");
    int i=0;
    unsigned char ckey;

    int keys_fd;
	keys_fd=open(DEV_PATH, O_RDONLY);
	if(keys_fd <= 0)
	{
		myPtf("open %s device error!\n",DEV_PATH);
		return NULL;
	}
    int isupper=0;
    int lcdside=OPER_SIDE;  //用户面还是操作员面，USER_SIDE,OPER_SIDE
    //初始化锁
    pthread_mutex_init(&keyboardmutex, NULL);   //默认属性初始化锁
    //一直循环判断读取Key，每50ms判断一次显示
    while(1)
    {
        //myPtf("real read key\n");
        ckey=fn_divReadKey(keys_fd);
        //是操作员户面按键
        lcdside=fn_KeySide(ckey);
        //暂时不考虑按键溢出的问题，如果按多了就丢弃
        ckey=(char)fn_ConvertKey(ckey,isupper);
        if (ckey==KEY_MYUPPER)
        {
            isupper=1;
            continue;
        }
        else
        {
            isupper=0;
        }

        if ((lcdside==OPER_SIDE)&&(keybufferlen>=MAX_KEYBUFFER_LEN))
        {
            continue;
        }
        //否则，是用户面按键,且溢出
        else if ((lcdside==USER_SIDE)&&(ukeybufferlen>=MAX_UKEYBUFFER_LEN))
        {
            continue;
        }

        //myPtf("ready to lock\n");
        //加锁
        pthread_mutex_lock(&keyboardmutex);
        //把她加到队尾
        //如果键值大，是用户面按键
        if (lcdside==USER_SIDE)
        {
            ukeybuffer[ukeybufferlen++]=ckey;
        }
        //否则，是操作员面按键
        else
        {
            keybuffer[keybufferlen++]=ckey;
        }
        //解锁
        pthread_mutex_unlock(&keyboardmutex);
        usleep(100000);
    }
    close(keys_fd);
    //回收锁
    pthread_mutex_destroy(&keyboardmutex);
}

//启动键盘线程，应在主程序初始化时调用
int fn_StartKeyboardThread()
{
    //启动显示线程
    myPtf("thread is ready to start!\n");
    int ret=pthread_create(&keyboardthreadid,NULL,fn_ReadKeyInThread,NULL);
    myPtf("thread is started!\n");
    return ret;
}
//结束键盘线程，可以不要
int fn_EndKeyboardThread()
{
    pthread_join(keyboardthreadid,NULL);
    return 0;
}
//查找该键值是那边的
int fn_KeySide(int orgkey)
{
    //用户面键盘
    if ((orgkey<15)||(orgkey==UENTER_KEY))
        return USER_SIDE;
    else
        return OPER_SIDE;
}
/////////////////
//键值转换
char fn_ConvertKey(int orgkey,int isupper)
{
    char retkey=-1;

    switch(orgkey)
    {
        case UNUM1_KEY:
        case NUM1_KEY:
            retkey='1';
            break;
        case UNUM2_KEY:
        case NUM2_KEY:
            retkey='2';
            break;
        case UNUM3_KEY:
        case NUM3_KEY:
            retkey='3';
            break;
        case UNUM4_KEY:
        case NUM4_KEY:
            retkey='4';
            break;
        case UNUM5_KEY:
        case NUM5_KEY:
            retkey='5';
            break;
        case UNUM6_KEY:
        case NUM6_KEY:
            retkey='6';
            break;
        case UNUM7_KEY:
        case NUM7_KEY:
            retkey='7';
            break;
        case UNUM8_KEY:
        case NUM8_KEY:
            retkey='8';
            break;
        case UNUM9_KEY:
        case NUM9_KEY:
            retkey='9';
            break;
        case UNUM0_KEY:
        case NUM0_KEY:
            retkey='0';
            break;
        case A_KEY:
            if (isupper==1)
                retkey='A';
            else
                retkey='a';
            break;
        case B_KEY:
            if (isupper==1)
                retkey='B';
            else
                retkey='b';
            break;
        case C_KEY:
            if (isupper==1)
                retkey='C';
            else
                retkey='c';
            break;
        case D_KEY:
            if (isupper==1)
                retkey='D';
            else
                retkey='d';
            break;
        case E_KEY:
            if (isupper==1)
                retkey='E';
            else
                retkey='e';
            break;
        case RETURN_KEY://回车符，表示输入结束，进入等待刷卡或扫码消费
            retkey='!';
            break;
        case CLEAR_KEY://退格键，表示删除原输入
            retkey='@';
            break;
        case FIXED_KEY://固定键
            retkey='#';
            break;
        case CASH_KEY://现金键与+键键复用
            retkey='$';
            break;
        case UCLEAR_KEY://用户面退格键
            retkey='%';
            break;
        case UENTER_KEY://用户面回车键
            retkey=EXIT_KEY;
            break;

        default:
            retkey=orgkey;
            break;
    }
    //return retkey;
    return orgkey;
}
////////////////////////////////////////////////////////////////////

