#include "a.h"
#include "cJSON.h"
#include "common.h"
#include "dealcmd.h"
#include "sendcmd.h"
#include "serial.h"
#include "sendcmd_cloud.h"
#include "nkqrscan.h"
#include <time.h>
#include <pthread.h>

#define MAX_PATH 260
#define CONF_FILE_PATH	"Config.ini"
#define NEEDRETRY 6     //10-6=4,0~4次之内连接不上，可以重连；4次以上连接不上，不用重连，直接返回错误，前端也提示错误
#define PAGESIZE  100   //100人一包名单下载


const char SERIAL_READCARD_CMD[]={0x02,0xE0,0x01,0xA6,0x89,0xCC};   //串口读卡命令字

static int MaxOfflinePackNum = 100;         //单笔脱机上传包
static char FCODE[]="12457801";
static char QRdeskey[24] = "YXP,sEY@.X&yrt@lZnk.2@O0";//秘钥
static DES_cblock QRivsetup = "R*AK@Z&w";
//static char QRdeskey[24] = "woshikeykkkk111333222111";//秘钥
//static DES_cblock QRivsetup = "nihaobua";
//static char QRdeskey[24] = "nkty83712208tjnkbhxq1234";//秘钥
//static DES_cblock QRivsetup = "bjkjgxm!";

static char ScanQrSerVersion[50]="";

static char ExQrCode[MAX_QRCODE_LEN]="";

static int UseTermType = 0; //终端使用标识
                        //USERTYPE_BOC_HQ(1001)中国银行总行
                        //USERTYPE_BOC_GZ(1002)中国银行广东分行
                        //USERTYPE_EDU_NKBH(2001)滨海学院
                        //USERTYPE_BJHS(2002) 北京航食只能刷M1卡4字节SN

static int UseTPP = USETTP_ENABLE;    //是否允许第三方支付

static char CallBackRetString[512];    //回调参数
static int PageNum=0;                  //下载到第几包

static config_t ParaConfig;

typedef struct Consume_Struct{
    int cardID;             //卡号
    int accountID;          //账号
    int contypeID;          //操作类型（0消费 1存款 2取款 3洗衣）
    int fType;              //消费方式（0刷卡，1刷脸，2账号二维码，3支付二维码）
    int flowID;             //终端流水号（消费时间秒值,时间戳）
    int flowMoney;          //消费金额（洗衣模式为消费洗衣劵数量）
    int status;             //当前消费状态.0表示未消费,
                            //1表示已收到消费请求，但尚未刷卡或扫码，可以取消
                            //2表示用户已刷卡或扫码，不能取消，只能等待返回
                            //3(0?)表示数据已返回，等待用户取走最后结果
    int could_Cmd;          //向云端发送的命令字，消费出纳等是3001（CLD_CONSUME_CMD），卡自捡是3010（CLD_GETACCOINFO_CMD）
    int termID;
    int areaID;
    int merchantID;         //商户编号 wang mao tan 2019-07-25 added
    int transactionType;    //交易类型：1-读卡，2-扫码，3-读卡或扫码
    int cmdType;            //命令类型，现在包括
                            //3：UI_REQ_CONSUME 请求消费
                            //10:UI_REQ_ACCOUNTINFO 获取账户信息
                            //11:UI_REQ_SCANQRCODE 申请扫描二维码
                            //13:UI_REQ_OFFLINECONSUME 请求脱机消费
    char pwd[8];            //卡片消费密码
    char consumeNum[128];   //二维码（支付宝或微信）
    char retstring[512];    //消费结果，当status=3时有效。服务端返回的字符串
    char termCode[52];      //终端唯一识别序号
}consume_struct_t;

typedef struct HeartBeat_Struct{
    int beattime;
    int ret;
    char termCode[52];           //终端唯一识别序号
}heartbeat_struct_t;

static char termCode[52];      //终端唯一识别序号

static heartbeat_struct_t curHeartbeat;
static consume_struct_t curConsume;
static int stopbeat = 0;
static int threadFlowID = 0;
static int scanFlowID = 0;
static int threadUploadFlow = 0;        //上传流水启动状态，1表示正在上传，0表示停止上传
static int threadDownloadUserList = 0;  //下载名单启动状态，1表示正在下载，0表示停止下载
//static char ConsumeSerVersion[50];    //版本号

static client_unsocket_t cli_scanqr;        //扫码使用结构
//static client_unsocket_t cli_secscreen;     //副屏显示使用结构
static client_unsocket_t cli_uicallback;    //UI层回调结构

static int serialfd;                        //串口使用结构

static char * pUserList = NULL;
static int userNum = 0;


extern int DES_dbs64_3dcbc_decrypt(const unsigned char *input,   //输入待解密数据（密文）
                                   unsigned char *output,       //输出解密后数据（明文）
                                   long length,                 //输出数据长度  填8，表示按8字节数据进行加密
                                   char *key1,                  //8字节秘钥1
                                   char *key2,                  //8字节秘钥2
                                   char *key3,                  //8字节秘钥3
                                   DES_cblock  ivec,            //偏移量
                                   int pack);                    //填充方式

//终端UI传来的JSON串，添加CMD命令字，以便发给云端服务
char * addCmd(int cmd,char * inputStr, int inputlen, int * outLen)
{
    myPtf("addCmd cmd=%d,inputstr=%s,inputlen=%d,outLen=%d,inputdata=",cmd,inputStr,inputlen,*outLen);
    for(int i=0;i<inputlen;i++)
    {
        myPtf("%02X",*(inputStr+i));
    }
    myPtf("\n");

    char strCmd[32];
    memset(strCmd,0,32);
    sprintf(strCmd,"{\"cmd\":%d,",cmd);

    char * strNew;
    int newLen = inputlen + strlen(strCmd); //OK
    strNew = malloc(newLen);
    if (strNew==NULL)
    {
        *outLen = 0;
        return NULL;
    }
    memset(strNew,0,newLen);
    memcpy(strNew,strCmd,strlen(strCmd));
    memcpy(strNew+strlen(strCmd),inputStr+1,inputlen-1);
    myPtf("addstring: %s,len=%d\n",strNew,newLen-1);
    *outLen = newLen-1;
    return strNew;
}
//YYYY-MM-DD HH:NN:SS 转 time_t格式
time_t FormatTime(char * szTime)
{
    struct tm tm1;
    time_t time1;
    sscanf(szTime, "%4d-%2d-%2d %2d:%2d:%2d",
          &tm1.tm_year,
          &tm1.tm_mon,
          &tm1.tm_mday,
          &tm1.tm_hour,
          &tm1.tm_min,
          &tm1.tm_sec);
    tm1.tm_year -= 1900;
    tm1.tm_mon --;
    tm1.tm_isdst=-1;
    time1 = mktime(&tm1);
    return time1;
}

//中银解码程序
//ciphertext    密文
//accountId     账号
//ifCheckTime   是否检查时间，0表示不检查，1表示检查。脱机时不检查。
//返回0表示成功，此时返回的accountId有效，否则accountId=0
//返回-1表示二维码解码失败
//返回-2表示超时失败
//返回-3表示厂商号不一致
//wang mao tan 2019-07-24 广分新规则，8位商户号+24位卡号+14位时间+00
int fn_DecryptForBOC(char * ciphertext,int * accountId,int cmdType)
{

    myPtf("start fn_DecryptForBOC\n");
    char deskey1[9];
    char deskey2[9];
    char deskey3[9];
    memset(deskey1,0,9);
    memset(deskey2,0,9);
    memset(deskey3,0,9);

    memcpy(deskey1,&QRdeskey[0],8);
    memcpy(deskey2,&QRdeskey[8],8);
    memcpy(deskey3,&QRdeskey[16],8);

    int recv_len = strlen(ciphertext);
    char * outbuff;
    outbuff = malloc(recv_len);
    memset(outbuff,0,recv_len);
    int ret = DES_dbs64_3dcbc_decrypt((unsigned char *)ciphertext,(unsigned char *)outbuff,recv_len,deskey1,deskey2,deskey3,QRivsetup,NOPADDING);
    if(ret>0)
    {
        myPtf("解码结果:%s\n",outbuff);
    }
    else
    {
        free(outbuff);
        myPtf("error 1!\n");
        return QR_DECODE_ERROR;
    }
    /*
    if (strlen(outbuff)!=40)
    {
        free(outbuff);
        myPtf("error 2!\n");
        return QR_DECODE_ERROR;
    }
    */
    //初始化参数
    char factoryCode[9];
    memset(factoryCode,0,9);
    //wang mao tan 商户编号变量
    int merchantID=0;
    char merchantNumber[9];
    memset(merchantNumber,0,9);

    int factoryID=0;
    char accId[25];
    memset(accId,0,25);
    char consumetime[17];
    memset(consumetime,0,17);

    //广分
    switch(UseTermType)
    {
        case USERTYPE_BOC_GZ:
        case USERTYPE_BOC_QD:
        //wang mao tan 2019-07-24 广东分行新规则新增商户号判断解析
            if (strlen(outbuff)!=40&&strlen(outbuff)!=48)
            {
                free(outbuff);
                myPtf("error 2!\n");
                return -1;
            }
            //wang mao tan 2019-07-24 商户号判断，兼容旧规则
            else if(strlen(outbuff)==48)
            {
                //取出参数
                //商户号还没做处理
                memcpy(merchantNumber,outbuff,8);
                merchantID = atoi(merchantNumber);
                memcpy(accId,outbuff+8,24);
                memcpy(consumetime,outbuff+32,14);
                break;
            }
            else
            {
                //取出参数
                memcpy(accId,outbuff,24);
                memcpy(consumetime,outbuff+24,14);
                break;
            }
        case USERTYPE_BOC_HQ:
        case USERTYPE_BJKJG:
        case USERTYPE_BJHS:
            if (strlen(outbuff)!=48)
            {
                free(outbuff);
                myPtf("error 3!\n");
                return -1;
            }
            //取出参数
            memcpy(factoryCode,outbuff,8);
            factoryID = atol(factoryCode);
            memcpy(accId,outbuff+8,24);
            memcpy(consumetime,outbuff+32,14);
            break;
        default:
            break;
    }
    /*
    if ((UseTermType == USERTYPE_BOC_GZ)||(UseTermType == USERTYPE_BOC_QD)||(UseTermType == USERTYPE_BJHS))
    {
        if (strlen(outbuff)!=40)
        {
            free(outbuff);
            myPtf("error 2!\n");
            return -1;
        }
        //取出参数
        memcpy(accId,outbuff,24);
        memcpy(consumetime,outbuff+24,14);
    }
    //总行
    else if ((UseTermType == USERTYPE_BOC_HQ)||(UseTermType == USERTYPE_BJKJG))
    {
        if (strlen(outbuff)!=48)
        {
            free(outbuff);
            myPtf("error 2!\n");
            return -1;
        }
        //取出参数
        memcpy(factoryCode,outbuff,8);
        memcpy(accId,outbuff+8,24);
        memcpy(consumetime,outbuff+32,14);
    }
    */
    free(outbuff);
    //判断时间格式有效性
    for(int i=0;i<14;i++)
    {
        if((consumetime[i]<0x30)||(consumetime[i]>0x39))
        {
            myPtf("time format error i=%d,consumetime[i]=%d!\n",i,consumetime[i]);
            return QR_DECODE_ERROR;
        }
    }
#if 0 
    //比较厂商编号
    if (strcmp(factoryCode,FCODE)!=0)
    {
        return QR_DECODE_FACTORYERR;
    }
#endif   
    //wang mao tan 2019-07-25
    //比较和终端传回来的商户号，一致则为本行人员，不一致则为外行人员
    myPtf("终端商户号有没有值:%d\n",curConsume.merchantID);
    if(merchantID!=curConsume.merchantID){
        return QR_DECODE_MERCHANTID; //商户编号错误
    }
    if ((factoryID>0)&&(factoryID!=UseTermType))
    {
        return QR_DECODE_FACTORYERR;
    }
    //比较时间
    struct tm tm1;
    myPtf("Qrtime %s\n",consumetime);
    sscanf(consumetime, "%4d%2d%2d%2d%2d%2d",
          &tm1.tm_year,
          &tm1.tm_mon,
          &tm1.tm_mday,
          &tm1.tm_hour,
          &tm1.tm_min,
          &tm1.tm_sec);
    tm1.tm_year -= 1900;
    tm1.tm_mon --;
    tm1.tm_isdst=-1;

    time_t QrTime = mktime(&tm1);
    time_t nowTime;
    time(&nowTime);
    double diff = difftime(nowTime,QrTime);
    myPtf("nowTime=%d,QrTime=%d",(int)nowTime,(int)QrTime);
    //如果脱机且超时，才返回超时
    if(((diff>300)||(diff<-300))&&(cmdType!=UI_REQ_OFFLINECONSUME))
    {
        return QR_DECODE_TIMEOUT;
    }
    //返回账号
    *accountId = atoi(accId);
    return 0;
}
//清除时间戳以外的数据
void fn_cleanConsumeStructButflowID()
{
    curConsume.cardID=0;             //卡号
    curConsume.accountID=0;          //账号
    curConsume.contypeID=0;          //操作类型（0消费 1存款 2取款 3洗衣）
    curConsume.fType=0;              //消费方式（0刷卡，1刷脸，2账号二维码，3支付二维码）
    //curConsume.flowID=0;             //终端流水号（消费时间秒值,时间戳）
    curConsume.flowMoney=0;          //消费金额（洗衣模式为消费洗衣劵数量）
    curConsume.status=0;             //当前消费状态.0表示未消费,
    curConsume.could_Cmd=0;          //向云端发送的命令字，消费出纳等是3001（CLD_CONSUME_CMD），卡自捡是3010（CLD_GETACCOINFO_CMD）
    curConsume.termID=0;
    curConsume.areaID=0;
    curConsume.transactionType=0;
    curConsume.merchantID=12457801;         //商户号 wang mao tan 2019-07-25 added
    memset(curConsume.pwd,0,sizeof(curConsume.pwd));            //卡片消费密码
    memset(curConsume.consumeNum,0,sizeof(curConsume.consumeNum));   //二维码（支付宝或微信）
    memset(curConsume.retstring,0,sizeof(curConsume.retstring));    //消费结果，当status=3时有效。服务端返回的字符串
    memset(curConsume.termCode,0,sizeof(curConsume.termCode));           //终端唯一识别序号
}

//从内存中取出一批流水并上传，成功返回>0，发送失败返回0
int fn_SendOfflineFlow(char * data,int number)
{
    char * sendBuf = (char *)malloc(number*OFFLINEFLOW_JSONSIZE+24);
    char FlowStr[OFFLINEFLOW_JSONSIZE+10];
    int i;
    offline_flow_t offline_flow;
    sprintf(sendBuf,"{\"cmd\":%d,\"flows\":[",CLD_UPLOADFLOW_CMD);

    char strTimeStamp[24];

    //循环取出数据
    for(i=0;i<number-1;i++)
    {
        memset(FlowStr,0,sizeof(FlowStr));
        memcpy(&offline_flow,(char *)((int)data+sizeof(offline_flow)*i),sizeof(offline_flow));

        sprintf(FlowStr,"{\"flowID\":%lld,\"cardID\":%d,\"accountId\":%d,\"flowMoney\":%d,\"termCode\":\"%s\"},",
                    (long long)(offline_flow.timeStamp)*10000000000+offline_flow.curTime,offline_flow.cardNo,
                    offline_flow.accountId,offline_flow.flowMoney,curHeartbeat.termCode);
        strcat(sendBuf,FlowStr);
    }
    memset(FlowStr,0,sizeof(FlowStr));
    memcpy(&offline_flow,(char *)((int)data+sizeof(offline_flow)*i),sizeof(offline_flow));
    //添加末尾
    sprintf(FlowStr,"{\"flowID\":%lld,\"cardID\":%d,\"accountId\":%d,\"flowMoney\":%d,\"termCode\":\"%s\"}]}",
                (long long)(offline_flow.timeStamp)*10000000000+offline_flow.curTime,offline_flow.cardNo,
                offline_flow.accountId,offline_flow.flowMoney,curHeartbeat.termCode);
    strcat(sendBuf,FlowStr);
    //通讯上传
    int strCloudCmd = CLD_UPLOADFLOW_CMD;
    int cloudCmdLen = strlen(sendBuf);
    int cloudRevLen = 0;
    char * strCloudRecv = sendCmd_socket(&strCloudCmd, sendBuf, cloudCmdLen, &cloudRevLen);
    myPtf("ret len=%d ,str=%s|||||\n",cloudRevLen,strCloudRecv);
    //释放内存
    free(sendBuf);

    //析出ret值
	cJSON * jRoot;
    //解析Json串
    jRoot=cJSON_Parse((char *)strCloudRecv);
    if(!jRoot)
    {
        myPtf("get not json root\n");
        free(strCloudRecv);
        //json串错误
        return 0;
    }
    cJSON * jRet = cJSON_GetObjectItem(jRoot,"ret");
    int ret = jRet->valueint;
    cJSON_Delete(jRoot);
    free(strCloudRecv);

    //判断返回值
    if (ret == 10)
    {
        myPtf("fn_DelRecord\n");
        //删除本批次
        fn_DelRecord(number);
    }
    return cloudRevLen;
}


char * GetHardwareSerialNo(int *reclen)
{
	FILE *ptr = NULL;
	char cmd[128] = "cat /sys/fsl_otp/HW_OCOTP_CFG1";
	int status = 0;
	char buf[150];
	int count;
	char *newbuf=NULL;

	newbuf=(char *)malloc(256);
	memset(newbuf,0,256);

	if((ptr = popen(cmd, "r"))==NULL)
	{
		return 0;
	}
	*reclen=0;
	memset(buf, 0, sizeof(buf));

	if((fgets(buf, sizeof(buf),ptr))!= NULL)//获取进程和子进程的总数
	{
		sprintf(newbuf,"%s",&buf[2]);
		newbuf[strlen(newbuf)-1]='\0';
		*reclen=strlen(newbuf);
		for(int i=0;i<strlen(newbuf);i++)
		{

			if(newbuf[i]>96&&newbuf[i]<123)
            {
			    newbuf[i]=newbuf[i]-32;
			    continue;
			}
		}
	}
	fclose(ptr);
	return newbuf;
}

//下载名单流水线程
static void * DownLoadUserThread(void *arg)
{
    //获取账户列表
    char strGetUserList[100]="";
    int iRetlen=0;
    PageNum = 1;
    cJSON * jRoot;  //JSON根目录
    cJSON * jUsers; //用户数组JSON串
    cJSON * jUser;  //用户
    int arrySize=PAGESIZE;
    int accountid=0;
    int cardid=0;
    userlist_t array_User[PAGESIZE];
    char * pUsers = (char *)array_User;
    //通讯上传
    int send_cmd = CLD_GETACCOLIST_CMD;
    int i=0;
    int lastParkageUserNum=0;
    //while(PageNum<1000)
    while(1)
    {
        sprintf(strGetUserList,"{\"cmd\":%d,\"termCode\":\"%s\",\"pageNum\":\"%d\",\"pageSize\":\"%d\"}",CLD_GETACCOLIST_CMD,termCode,PageNum,PAGESIZE);
        myPtf("CLD_GETACCOLIST_CMD %s\n",strGetUserList);
        char * strRecvBuf = sendCmd_socket(&send_cmd, strGetUserList, strlen(strGetUserList), &iRetlen);
        myPtf("ret len=%d ,str=%s|||||\n",iRetlen,strRecvBuf);
        if (strRecvBuf==NULL)
        {
            myPtf("userlist error|||||\n");
            PageNum = -1;
            break;
        }
        memset(pUsers,0,sizeof(userlist_t)*PAGESIZE);
        i = 0;
        //////////////////////////////////////////////
        //解析Json串
        jRoot=cJSON_Parse((char *)strRecvBuf);
        if(!jRoot)
        {
            myPtf("get not json root\n");
            free(strRecvBuf);
            //json串错误
            break;
        }
        //首先析出users用户数组
        jUsers = cJSON_GetObjectItem(jRoot,"users");
        jUser = jUsers->child;
        //如果已经为空，释放退出
        if (jUser == NULL)
        {
            cJSON_Delete(jRoot);
            myPtf("userlist over all|||||\n");
            break;
        }
        while(jUser != NULL)
        {
            array_User[i].accountId = cJSON_GetObjectItem(jUser,"accountId")->valueint;
            array_User[i].cardNo = cJSON_GetObjectItem(jUser,"cardid")->valueint;
            //myPtf("AccountId=%d,cardid=%d\n",accountid,cardid);
            jUser = jUser->next;
            i++;
            lastParkageUserNum = i;
        }
        accountid = array_User[lastParkageUserNum-1].accountId;
        cardid = array_User[lastParkageUserNum-1].cardNo;

        cJSON_Delete(jRoot);
        myPtf("userlist oneparkage over!");
        free(strRecvBuf);
        //第一包新写文件
        //以后包追加写文件
        if (i>0)
        {
            fn_WriteAccoPack(pUsers,PageNum,sizeof(userlist_t)*i);
        }
        else
        {
            myPtf("userlist over all|||||\n");
            break;
        }
        PageNum++;
    }
    //回读一遍全账户
    if (pUserList != NULL)
    {
        free(pUserList);
        userNum = 0;
    }
    pUserList = fn_ReadAccoPack(&userNum);
    myPtf("fn_ReadAccoPack|||||userNum=%d,PageNum=%d,lastParkageUserNum=%d,pUserList=%d\n",userNum,PageNum,lastParkageUserNum,(int)pUserList);
    if (userNum != (PageNum-2)*PAGESIZE + lastParkageUserNum)
    {
        myPtf("userNum error|||||userNum=%d,PageNum=%d,lastParkageUserNum=%d\n",userNum,PageNum,lastParkageUserNum);
        PageNum = -1;
    }
    else
    {

        //查找账户
        myPtf("fn_FindKeyByAccountId|||||pUserList=%d,userNum=%d,accountid=%d\n",(int)pUserList,userNum,accountid);
        if(fn_FindKeyByAccountId(pUserList,userNum,accountid) == 1)
        {
            PageNum = 0;
            myPtf("get list OK!");
        }
        else
        {
            PageNum = -1;
            myPtf("get list error!");
        }
    }
    threadDownloadUserList = 0;
    return (void *)NULL;
}

//上传流水线程
static void * UploadFlowThread(void *arg)
{
    char * pData = (char *)malloc(sizeof(offline_flow_t)*MaxOfflinePackNum);
    offline_flow_t offline_flow;
    int flowCount=0;
    //获取部分流水
    //还有流水未上传？
    while(fn_GetOffline_FlowNum()>0)
    {
        //如果正在刷卡消费，则直接退出不上传
        if (threadFlowID != 0)
        {
            myPtf("UploadFlowThread:threadFlowID!=0,Exit!\n");
            break;
        }
        //上传部分流水
        flowCount = fn_GetRecord(pData,MaxOfflinePackNum);
        if (flowCount>0)
        {
            myPtf("fn_SendOfflineFlow:flowCount=%d!\n",flowCount);
            //上传成功，继续循环上传
            if(fn_SendOfflineFlow(pData,flowCount)>0)
            {
                myPtf("fn_SendOfflineFlow OK!\n");
                if(fn_GetOffline_FlowNum() == 0)
                {
                    myPtf("fn_SendOfflineFlow OK! Offline_FlowNum=0!\n");
                    //删除文件
                    break;
                }
            }
            else
            //上传失败，将剩余流水重写，并退出
            {
                myPtf("fn_SendOfflineFlow error!  fn_WriteRecords!\n");
                break;
            }
        }
        else
        {
            myPtf("fn_SendOfflineFlow:flowCount=%d!\n",flowCount);
            //返回
            //删除文件
            break;
        }
    }
    //回写文件
    fn_WriteRecords();
    free(pData);
    threadUploadFlow = 0;
    return (void *)NULL;
}

struct timeval timeget(void)
{
    struct timeval now;
    unsigned char  timestr[60] = {0};
    unsigned char  uptimestr[30] = {0};
    unsigned char * dotaddr;
    unsigned long second;
    char error = 0;
    FILE * timefile = NULL;

    timefile = fopen("/proc/uptime", "r");
    if(!timefile)
    {
        myPtf("[%s:line:%d] error opening '/proc/uptime'",__FILE__,__LINE__);
        error = 1;
        goto out;
    }

    if( (fread(timestr, sizeof(char), 60, timefile)) == 0 )
    {
        myPtf("[%s:line:%d] read '/proc/uptime' error",__FILE__,__LINE__);
        error = 1;
        goto out;
    }

    dotaddr = (unsigned char *)strchr((char *)timestr, '.');
    if((dotaddr - timestr + 2) < 30)
        memcpy(uptimestr, timestr, dotaddr - timestr + 2);
    else
    {
        myPtf("[%s:line:%d] uptime string is too long",__FILE__,__LINE__);
        error = 1;
        goto out;
    }
    uptimestr[dotaddr - timestr + 2] = '\0';

out:
    if(error)
    {
        now.tv_sec  = 0;
        now.tv_usec = 0;
    }
    else
    {
        now.tv_sec  = atol((char *)uptimestr);
        now.tv_usec = 0;
    }

    fclose(timefile);
    return now;
}


static void * HeartBeatThread(void *arg)
{
    memset(&curHeartbeat,0,sizeof(curHeartbeat));
    int cloudCmdLen ;
    char strCloudCmd[128];
    int cloudRevLen;
    int cloudret;
    int send_cmd;
    struct timeval tv;
    time_t timep;
    char syscmd[128];
    memset(syscmd,0,sizeof(syscmd));
    sprintf(syscmd,"sh /usr/local/nkty/autosingle.sh Pos320Main");
	//char *szbufferOut=NULL;
	int ilen=0;
	int iLoopTime = 0;
    struct timeval uptime;
    int iFirstTime = 1;
    while(stopbeat == 0)
    {
        //调用system
        myPtf("system:%s\n",syscmd);
        system(syscmd);

        //szbufferOut=(char *)GetHardwareSerialNo(&ilen);
        //myPtf("==================返回结果2:%s,%d,哈哈哈===============\n", szbufferOut,ilen);
        //free(szbufferOut);

        //准备命令
        memset(strCloudCmd,0,sizeof(strCloudCmd));
        sprintf(strCloudCmd,"{\"cmd\":%d,\"termCode\":\"%s\"}",CLD_HEARTBEAT_CMD,curHeartbeat.termCode);
        cloudCmdLen = strlen(strCloudCmd);
        cloudRevLen = 0;
        cloudret = 0;
        //获取心跳包
        char * strCloudRecv = sendCmd_socket(&send_cmd, strCloudCmd, cloudCmdLen, &cloudRevLen);
        myPtf("ret len=%d ,str=%s|||||\n",cloudRevLen,strCloudRecv);
        if ((strCloudRecv==NULL)||(cloudRevLen==0))
        {
            //连接失败
            time (&timep);
            myPtf("not get heartbeat,tps=%s\n",ctime(&timep));
            if (curHeartbeat.ret > 0)
                curHeartbeat.ret--;
            sleep(2);
        }
        else
        {
            //解析Json串
            cJSON * jRoot=cJSON_Parse((char *)strCloudRecv);
            if(!jRoot)
            {
                if (curHeartbeat.ret > 0)
                    curHeartbeat.ret--;
                myPtf("get not json root\n");
            }
            else
            {
                cJSON * jRet=cJSON_GetObjectItem(jRoot,"ret");
                if(!jRet)
                {
                    if (curHeartbeat.ret > 0)
                        curHeartbeat.ret--;
                    myPtf("get not json Ret\n");
                }
                else
                {
                    if (jRet->valueint == 0)
                        curHeartbeat.ret--;
                    else
                        curHeartbeat.ret = jRet->valueint;
                    //记录时间
                    gettimeofday(&tv,NULL);
                    curHeartbeat.beattime = tv.tv_sec;
                }
                cJSON * jTermType=cJSON_GetObjectItem(jRoot,"UseTermType");
                if(jTermType)
                {
                    UseTermType = jTermType->valueint;
                    if (iFirstTime == 1)
                    {
                        ParaConfig.UseTermType = UseTermType;
                        //写入参数成功
                        if (fn_WriteConfig((char *)(&ParaConfig),sizeof(config_t))==0)
                        {
                            iFirstTime = 0;
                        }
                    }
                }
                cJSON * jTPPType=cJSON_GetObjectItem(jRoot,"UseTPP");
                if(jTPPType)
                {
                    UseTPP = jTPPType->valueint;
                }
            }
            cJSON_Delete(jRoot);
            free(strCloudRecv);
            time (&timep);
            myPtf("Heartbeat is sleep tsp=%s,curConsume.flowID=%d,threadFlowID=%d,iLoopTime=%d\n",ctime(&timep),curConsume.flowID,threadFlowID,iLoopTime);
            //每5秒心跳一次
            sleep(5);
            iLoopTime++;
            //10分钟更新一次
            if(iLoopTime>120)
            {
                uptime = timeget();
                time (&timep);
                myPtf("try to fn_GetServerTime iLoopTime = %d!timestamp=%lu,tsp=%s\n",iLoopTime,uptime.tv_sec,ctime(&timep));
                //更新时间
                if (fn_GetServerTime(&send_cmd)==0)
                {
                    //如果更新成功，则归零
                    iLoopTime = 0;
                    myPtf("fn_GetServerTime ok!\n");
                }
                else
                {
                    //如果失败，则40秒后重试
                    iLoopTime = 100;
                    myPtf("fn_GetServerTime error!\n");
                }
                uptime = timeget();
                time (&timep);
                myPtf("fn_GetServerTime over iLoopTime = %d!timestamp=%lu,tsp=%s\n",iLoopTime,uptime.tv_sec,ctime(&timep));
            }
        }
    }
    return (void *)NULL;
}

//识别二维码线程
static void * ScanQrThread(void *arg)
{
    //int flowID = *(int *)arg;
    int flowID = curConsume.flowID;
    scanFlowID = flowID;
    myPtf("start ScanQrThread scanFlowID=%d\n",scanFlowID);
    int Cmd;
    int ret;
    char * strQr = NULL;
    char * strRecvConsume;

    time_t timep;
    //当流水被取消，或者已刷卡，返回
    while(curConsume.flowID == flowID)
    {
        time (&timep);
        myPtf("%s:ScanQrThread curConsume.flowid=%d,flowID=%d,scanFlowID=%d",ctime(&timep),curConsume.flowID,flowID,scanFlowID);
        int ret = 0;
        if (curConsume.status == CONSUME_FINISH)
        {
            break;
        }
        else
        {
            usleep(200000);
            time (&timep);
            myPtf("%s:ScanQrThread curConsume.status==CONSUME_WAITFORCARD,start read qr or card\n",ctime(&timep));
            //读二维码
            Cmd = SCANQR_GET_QRCODE_CMD;
            time (&timep);
            myPtf("%s:ScanQrThread get qr\n",ctime(&timep));
            strQr = sendCmd_afunix(cli_scanqr.clisockfd,cli_scanqr.clipath, cli_scanqr.serpath,&Cmd, NULL, 0);
            //strQr = NULL;
            time (&timep);
            myPtf("%s:ScanQrThread get qr over\n",ctime(&timep));
            if (strQr!=NULL)
            {
                //读到二维码
                myPtf("ScanQrThread get qr:%s\n",strQr);
                /////////////////////////////////////////////
                sprintf(curConsume.retstring,"{\"flowID\":\"%d\",\"ret\":10,\"qrstr\":\"%s\"}",flowID,strQr);
                ////////////////////////////////////////////////////////////////////
                //回调通知UI界面
                memset(CallBackRetString,0,sizeof(CallBackRetString));
                sprintf(CallBackRetString,"{\"timeStamp\":\"%d\",\"cmdType\":%d,\"state\":1,",flowID,curConsume.cmdType);
                strcpy(CallBackRetString+strlen(CallBackRetString),curConsume.retstring+1);
                Cmd = CONSUME_CALLBACK_QUERET;
                sendCmd_afunix(cli_uicallback.clisockfd,cli_uicallback.clipath, cli_uicallback.serpath,&Cmd, CallBackRetString,strlen(CallBackRetString)+1);
                ////////////////////////////////////////////////////////////////////
                curConsume.status = CONSUME_FINISH;
                /////////////////////////////////////////////
                //释放资源
                free(strQr);
                strQr=NULL;
            }
            else
            {
                memset(ExQrCode,0,sizeof(ExQrCode));
                myPtf("ScanQrThread get qr  fn_GetQRCode ??????\n");
                if(fn_GetQRCode(ExQrCode)==0)
                {
                    time (&timep);
                    myPtf("%s:ScanQrThread read qr fault usleep200000\n",ctime(&timep));
                }
                else
                {
                    //读到二维码
                    myPtf("ScanQrThread get exqr:%s\n",ExQrCode);
                    /////////////////////////////////////////////
                    sprintf(curConsume.retstring,"{\"flowID\":\"%d\",\"ret\":10,\"qrstr\":\"%s\"}",flowID,ExQrCode);
                    ////////////////////////////////////////////////////////////////////
                    //回调通知UI界面
                    memset(CallBackRetString,0,sizeof(CallBackRetString));
                    sprintf(CallBackRetString,"{\"timeStamp\":\"%d\",\"cmdType\":%d,\"state\":1,",flowID,curConsume.cmdType);
                    strcpy(CallBackRetString+strlen(CallBackRetString),curConsume.retstring+1);
                    Cmd = CONSUME_CALLBACK_QUERET;
                    sendCmd_afunix(cli_uicallback.clisockfd,cli_uicallback.clipath, cli_uicallback.serpath,&Cmd, CallBackRetString,strlen(CallBackRetString)+1);
                    ////////////////////////////////////////////////////////////////////
                    curConsume.status = CONSUME_FINISH;
                    /////////////////////////////////////////////
                    //通知内置摄像头停止扫码
                    Cmd = SCANQR_STOP_QRCODE_CMD;
                    sendCmd_afunix(cli_scanqr.clisockfd,cli_scanqr.clipath, cli_scanqr.serpath,&Cmd, NULL, 0);
                }
            }
        }
    }
    scanFlowID = 0;
    return (void *)NULL;
}
//消费服务线程
static void * ConsumeThread(void *arg)
{
    int flowID;
    int Cmd;
    char * strQr = NULL;
    char recvbuf[64]; //用于读卡命令返回
    int recvlen;//需要接收19个字符
    int recvleft;
    int recvedlen;
    int LoopTimes;
    int i=0;
    char strCloudConsume[512];//用于向云端发送数据，因为存在反复发送，所以固定内存数组。考虑以后二维码长度最大128，总共设计512字节
    int istrlen;
    int send_cmd;
    char * strRecvConsume;
    int ret;
    time_t timep;
    char upTime[24];
    time_t rawtime;
    struct tm *ptminfo;
    int iloop=0;
    char waitString[]="\"ret\":100";
    offline_flow_t offline_flow;
    memset(&offline_flow,0,sizeof(offline_flow));
    //int flowID = curConsume.flowID;
    while (true)
    {
        if (threadFlowID == 0)
        {
            //myPtf("threadFlowID is zeor,sleep and continue for wait!%d\n",threadFlowID);
            usleep(50000);
            continue;
        }
        myPtf("new threadFlowId=%d,iloop=%d\n",threadFlowID,iloop++);
        flowID = threadFlowID;
        myPtf("start ConsumeThread threadFlowID=%d\n",threadFlowID);
        myPtf("start consumeThread001 time = %d\n",getCurrentTime());
        fn_ClearQRBuff();
        myPtf("fn_ClearQRBuff consumeThread0011 time = %d\n",getCurrentTime());

        //如果是脱机，则允许读卡或扫码
        if (curConsume.cmdType == UI_REQ_OFFLINECONSUME)
        {
            curConsume.transactionType = TRSACTTYPE_READANDSCAN;
        }

        //当流水被取消，或者已刷卡，返回
        while(curConsume.flowID == flowID)
        {
            time (&timep);
            myPtf("%s:0curConsume.flowid=%d,flowID=%d,threadFlowID=%d,curConsume.status=%d",ctime(&timep),curConsume.flowID,flowID,threadFlowID,curConsume.status);
            myPtf("start loop consumeThread002 time = %d\n",getCurrentTime());
            ret = 0;
            if (curConsume.status == CONSUME_FINISH)
            {
                break;
            }
            //等待刷卡或扫码
            if(curConsume.status == CONSUME_WAITFORCARD)
            {
                myPtf("curConsume.transactionType = %d\n",curConsume.transactionType);
                //如果是读二维吗内容，但不交易
                if (curConsume.transactionType == TRSACTTYPE_SCANQRANDRET)
                {
                    time (&timep);
                    myPtf("%s:curConsume.status == CONSUME_WAITFORCARD,start read qr or card\n",ctime(&timep));
                    //读二维码
                    Cmd = SCANQR_GET_QRCODE_CMD;
                    time (&timep);
                    myPtf("%s:get qr\n",ctime(&timep));
                    strQr = sendCmd_afunix(cli_scanqr.clisockfd,cli_scanqr.clipath, cli_scanqr.serpath,&Cmd, NULL, 0);
                    //strQr = NULL;
                    time (&timep);
                    myPtf("%s:get qr over\n",ctime(&timep));
                    if (strQr!=NULL)
                    {
                        //读到二维码
                        myPtf("get qr:%s\n",strQr);
                        sprintf(curConsume.retstring,"{\"flowID\":\"%d\",\"ret\":10,\"qrstr\":\"%s\"}",flowID,strQr);
                        ////////////////////////////////////////////////////////////////////
                        //回调通知UI界面
                        memset(CallBackRetString,0,sizeof(CallBackRetString));
                        sprintf(CallBackRetString,"{\"timeStamp\":\"%d\",\"cmdType\":%d,\"state\":1,",flowID,curConsume.cmdType);
                        strcpy(CallBackRetString+strlen(CallBackRetString),curConsume.retstring+1);
                        Cmd = CONSUME_CALLBACK_QUERET;
                        sendCmd_afunix(cli_uicallback.clisockfd,cli_uicallback.clipath, cli_uicallback.serpath,&Cmd, CallBackRetString,strlen(CallBackRetString)+1);
                        ////////////////////////////////////////////////////////////////////
                        curConsume.status = CONSUME_FINISH;

                        free(strQr);
                        strQr=NULL;
                    }
                    else
                    {
                        memset(ExQrCode,0,sizeof(ExQrCode));
                        myPtf("get qr 2fn_GetQRCode !!!!\n");
                        if(fn_GetQRCode(ExQrCode) == 1)
                        {
                            //读到二维码
                            myPtf("get qr OK from fn_GetQRCode:%s\n",ExQrCode);
                            //关闭内置扫码
                            Cmd = SCANQR_STOP_QRCODE_CMD;
                            sendCmd_afunix(cli_scanqr.clisockfd,cli_scanqr.clipath, cli_scanqr.serpath,&Cmd, NULL, 0);
                            /////////////////////////////////////////////
                            //读到二维码
                            myPtf("get qr:%s\n",ExQrCode);
                            sprintf(curConsume.retstring,"{\"flowID\":\"%d\",\"ret\":10,\"qrstr\":\"%s\"}",flowID,ExQrCode);
                            ////////////////////////////////////////////////////////////////////
                            //回调通知UI界面
                            memset(CallBackRetString,0,sizeof(CallBackRetString));
                            sprintf(CallBackRetString,"{\"timeStamp\":\"%d\",\"cmdType\":%d,\"state\":1,",flowID,curConsume.cmdType);
                            strcpy(CallBackRetString+strlen(CallBackRetString),curConsume.retstring+1);
                            Cmd = CONSUME_CALLBACK_QUERET;
                            sendCmd_afunix(cli_uicallback.clisockfd,cli_uicallback.clipath, cli_uicallback.serpath,&Cmd, CallBackRetString,strlen(CallBackRetString)+1);
                            ////////////////////////////////////////////////////////////////////
                            curConsume.status = CONSUME_FINISH;
                        }
                        else
                        {
                            myPtf("get qr error from fn_GetQRCode\n");
                        }

                    }
                    /*else
                    {
                        time (&timep);
                        myPtf("%s:read qr fault usleep200000\n",ctime(&timep));
                    }*/
                    continue;
                }

                //交易类型为（扫码）或者（扫码或读卡）
                if((curConsume.transactionType == TRSACTTYPE_SCANQRCODE)||(curConsume.transactionType == TRSACTTYPE_READANDSCAN))
                {
                    //读二维码
                    time (&timep);
                    myPtf("%s:curConsume.status==CONSUME_WAITFORCARD,start read qr or card\n",ctime(&timep));
                    //读二维码
                    Cmd = SCANQR_GET_QRCODE_CMD;
                    time (&timep);
                    myPtf("%s:get qr\n",ctime(&timep));
                    strQr = sendCmd_afunix(cli_scanqr.clisockfd,cli_scanqr.clipath, cli_scanqr.serpath,&Cmd, NULL, 0);
                    //strQr = NULL;
                    time (&timep);
                    myPtf("%s:get qr over\n",ctime(&timep));
                    if (strQr!=NULL)
                    {
                        //读到二维码
                        myPtf("get qr:%s\n",strQr);
                        //判断终端类型
                        switch (UseTermType)
                        {
                        case USERTYPE_BOC_HQ:
                        case USERTYPE_BJKJG:
                        case USERTYPE_BJHS:
                            curConsume.fType = 2;
                            //解码，中银总行特殊要求，解码后为账号
                            if (strlen(strQr)!=64)
                            {
                                //第三方支付码支付（微信支付宝等第三方支付码支付）
                                if ((UseTPP==USETTP_ENABLE)&&(strlen(strQr)==18))
                                {
                                    strcpy(curConsume.consumeNum,strQr);
                                    curConsume.fType = 3;
                                    ret = 0;
                                }
                                else
                                {
                                    ret = QR_DECODE_ERROR;  //错误二维码，长度不足
                                }
                            }
                            else
                            {
                                ret = fn_DecryptForBOC(strQr,&curConsume.accountID,curConsume.cmdType);
                            }
                            break;
                        //广分、青分、科技馆、航食均支持扫码模式（密码可能不对）
                        case USERTYPE_BOC_GZ:
                        case USERTYPE_BOC_QD:
                            curConsume.fType = 2;
                            myPtf("strlen(strQr) = %d\n",strlen(strQr));
                            //解码，中银广分特殊要求，解码后为账号
                            //wang mao tan 2019-07-24 广东分行新规则新加8位商户号，长度变为64，兼容旧规则
                            if (strlen(strQr)!=56&&strlen(strQr)!=64)
                            {
                                //第三方支付码支付（微信支付宝等第三方支付码支付）
                                if ((UseTPP==USETTP_ENABLE)&&(strlen(strQr)==18))
                                {
                                    strcpy(curConsume.consumeNum,strQr);
                                    curConsume.fType = 3;
                                    ret = 0;
                                }
                                else
                                {
                                    ret = QR_DECODE_ERROR;  //错误二维码，长度不足
                                }
                            }
                            else
                            {
                                myPtf("ready to fn_DecryptForBOC\n");
                                ret = fn_DecryptForBOC(strQr,&curConsume.accountID,curConsume.cmdType);
                            }
                            break;
                        case USERTYPE_EDU_NKBH:
                            curConsume.fType = 3;
                            //第三方支付码支付（微信支付宝等第三方支付码支付）
                            if ((UseTPP==USETTP_ENABLE)&&(strlen(strQr)==18))
                            {
                                strcpy(curConsume.consumeNum,strQr);
                                ret = 0;
                            }
                            else
                            {
                                ret = QR_DECODE_ERROR;  //错误二维码，长度不足
                            }
                            break;
                        default:
                            break;
                        }
                        myPtf("decrypt accountid = %d\n",curConsume.accountID);
                        //解码成功，进入下一阶段
                        if ((ret == 0)&&((curConsume.accountID>0)||(strlen(curConsume.consumeNum)>0)))
                        //if ((ret == 0)&&(curConsume.accountID>0))
                        {
                            myPtf("scanqr1 OK consumeThread003 time = %d\n",getCurrentTime());
                            ////////////////////////////////////////////////////////////////////
                            //回调通知UI界面
                            memset(CallBackRetString,0,sizeof(CallBackRetString));
                            //state=4表示结算中，state=1表示成功，state=2表示失败
                            sprintf(CallBackRetString,"{\"timeStamp\":\"%d\",\"cmdType\":%d,\"state\":4}",flowID,curConsume.cmdType);
                            Cmd = CONSUME_CALLBACK_QUERET;
                            //sendCmd_afunix(cli_uicallback.clisockfd,cli_uicallback.clipath,cli_uicallback.serpath,&Cmd,CallBackRetString,strlen(CallBackRetString)+1);
                            ////////////////////////////////////////////////////////////////////
                            curConsume.status = CONSUME_HAVECARDNO;
                        }
                        else
                        {
                            myPtf("scanqr2 OK consumeThread003 time = %d\n",getCurrentTime());
                            sprintf(curConsume.retstring,"{\"flowID\":\"%d\",\"ret\":%d}",flowID,ret);
                            ////////////////////////////////////////////////////////////////////
                            //回调通知UI界面
                            memset(CallBackRetString,0,sizeof(CallBackRetString));
                            sprintf(CallBackRetString,"{\"timeStamp\":\"%d\",\"cmdType\":%d,\"state\":2,",flowID,curConsume.cmdType);
                            strcpy(CallBackRetString+strlen(CallBackRetString),curConsume.retstring+1);
                            Cmd = CONSUME_CALLBACK_QUERET;
                            sendCmd_afunix(cli_uicallback.clisockfd,cli_uicallback.clipath, cli_uicallback.serpath,&Cmd, CallBackRetString,strlen(CallBackRetString)+1);
                            ////////////////////////////////////////////////////////////////////
                            curConsume.status = CONSUME_FINISH;
                            free(strQr);
                            strQr=NULL;
                            //退出本线程
                            continue;
                        }
                        /////////////////////////////////////////////
                        //释放资源
                        free(strQr);
                        strQr=NULL;
                    }
                    else
                    {
                        memset(ExQrCode,0,sizeof(ExQrCode));
                        myPtf("get qr 1fn_GetQRCode !!!!\n");
                        if(fn_GetQRCode(ExQrCode) == 1)
                        {
                            //读到二维码
                            myPtf("get qr OK from fn_GetQRCode:%s\n",ExQrCode);
                            //////////////////////////////
                            //test QRCode
                            //myPtf("test fn_DecryptForBOC:%s\n",ExQrCode);
                            //fn_DecryptForBOC(ExQrCode,&curConsume.accountID,curConsume.cmdType);
                            //关闭内置扫码
                            Cmd = SCANQR_STOP_QRCODE_CMD;
                            sendCmd_afunix(cli_scanqr.clisockfd,cli_scanqr.clipath, cli_scanqr.serpath,&Cmd, NULL, 0);
                            myPtf("close scanqr SCANQR_STOP_QRCODE_CMD 11111 UseTermType=%d\n",UseTermType);
                           //判断终端类型
                            switch (UseTermType)
                            {
                            //科技馆、航食、总行
                            case USERTYPE_BOC_HQ:
                            case USERTYPE_BJKJG:
                            case USERTYPE_BJHS:
                                myPtf("USERTYPE_BOC_HQ 1\n");
                                curConsume.fType = 2;
                                //解码，中银总行特殊要求，解码后为账号
                                if (strlen(ExQrCode)!=64)
                                {
                                    //第三方支付码支付（微信支付宝等第三方支付码支付）
                                    if ((UseTPP==USETTP_ENABLE)&&(strlen(ExQrCode)==18))
                                    {
                                        strcpy(curConsume.consumeNum,ExQrCode);
                                        curConsume.fType = 3;
                                        ret = 0;
                                    }
                                    else
                                    {
                                        ret = QR_DECODE_ERROR;  //错误二维码，长度不足
                                    }
                                }
                                else
                                    ret = fn_DecryptForBOC(ExQrCode,&curConsume.accountID,curConsume.cmdType);
                                break;
                            //广分、青分均支持扫码模式（密码可能不对）
                            case USERTYPE_BOC_GZ:
                            case USERTYPE_BOC_QD:
                                myPtf("USERTYPE_BOC_GZ is here strlen(ExQrCode)=%d\n",strlen(ExQrCode));
                                curConsume.fType = 2;
                                //解码，中银广分特殊要求，解码后为账号
                                //wang mao tan 2019-07-24 广东分行新规则，长度位64，兼容旧规则
                                if (strlen(ExQrCode)!=56&&strlen(ExQrCode)!=64)
                                {
                                    myPtf("USERTYPE_BOC_GZ 1 UseTPP=%d,strlen(ExQrCode)=%d\n",UseTPP,strlen(ExQrCode));
                                    //第三方支付码支付（微信支付宝等第三方支付码支付）
                                    if ((UseTPP==USETTP_ENABLE)&&(strlen(ExQrCode)==18))
                                    {
                                        myPtf("USERTYPE_BOC_GZ 2\n");
                                        strcpy(curConsume.consumeNum,ExQrCode);
                                        curConsume.fType = 3;
                                        ret = 0;
                                    }
                                    else
                                    {
                                        myPtf("USERTYPE_BOC_GZ 3\n");
                                        ret = QR_DECODE_ERROR;  //错误二维码，长度不足
                                    }
                                }
                                else
                                {
                                    myPtf("USERTYPE_BOC_GZ 4\n");
                                    ret = fn_DecryptForBOC(ExQrCode,&curConsume.accountID,curConsume.cmdType);
                                }
                                break;
                            case USERTYPE_EDU_NKBH:
                                myPtf("USERTYPE_EDU_NKBH 1\n");
                                curConsume.fType = 3;
                                //第三方支付码支付（微信支付宝等第三方支付码支付）
                                if ((UseTPP==USETTP_ENABLE)&&(strlen(ExQrCode)==18))
                                {
                                    strcpy(curConsume.consumeNum,ExQrCode);
                                    ret = 0;
                                }
                                else
                                {
                                    ret = QR_DECODE_ERROR;  //错误二维码，长度不足
                                }
                                break;
                            default:
                                myPtf("other user 1\n");
                                break;
                            }
                            myPtf("decrypt accountid = %d\n",curConsume.accountID);
                            //解码成功，进入下一阶段
                            if ((ret == 0)&&((curConsume.accountID>0)||(strlen(curConsume.consumeNum)>0)))
                            {
                                myPtf("scanqr OK consumeThread003 time = %d\n",getCurrentTime());
                                ////////////////////////////////////////////////////////////////////
                                //回调通知UI界面
                                memset(CallBackRetString,0,sizeof(CallBackRetString));
                                //state=4表示结算中，state=1表示成功，state=2表示失败
                                sprintf(CallBackRetString,"{\"timeStamp\":\"%d\",\"cmdType\":%d,\"state\":4}",flowID,curConsume.cmdType);
                                Cmd = CONSUME_CALLBACK_QUERET;
                                //sendCmd_afunix(cli_uicallback.clisockfd,cli_uicallback.clipath,cli_uicallback.serpath,&Cmd,CallBackRetString,strlen(CallBackRetString)+1);
                                ////////////////////////////////////////////////////////////////////
                                curConsume.status = CONSUME_HAVECARDNO;
                            }
                            else
                            {
                                myPtf("scanqr error consumeThread003 time = %d\n",getCurrentTime());
                                sprintf(curConsume.retstring,"{\"flowID\":\"%d\",\"ret\":%d}",flowID,ret);
                                ////////////////////////////////////////////////////////////////////
                                //回调通知UI界面
                                memset(CallBackRetString,0,sizeof(CallBackRetString));
                                sprintf(CallBackRetString,"{\"timeStamp\":\"%d\",\"cmdType\":%d,\"state\":2,",flowID,curConsume.cmdType);
                                strcpy(CallBackRetString+strlen(CallBackRetString),curConsume.retstring+1);
                                Cmd = CONSUME_CALLBACK_QUERET;
                                sendCmd_afunix(cli_uicallback.clisockfd,cli_uicallback.clipath, cli_uicallback.serpath,&Cmd, CallBackRetString,strlen(CallBackRetString)+1);
                                ////////////////////////////////////////////////////////////////////
                                curConsume.status = CONSUME_FINISH;
                                //退出本线程
                                continue;
                            }
                        }
                        else
                        {
                            myPtf("get qr error from fn_GetQRCode\n");
                        }

                    }
                    time (&timep);
                    myPtf("%s:read qr fault,curConsume.fType=%d\n",ctime(&timep),curConsume.fType);
                }
                //扫码未成功，且交易类型为只读卡或读卡加扫码，此时开始读卡
                if((curConsume.status == CONSUME_WAITFORCARD) &&
                   ((curConsume.transactionType == TRSACTTYPE_READCARD)||
                    (curConsume.transactionType == TRSACTTYPE_READANDSCAN)
                   )
                  )
                {
                    myPtf("start readcard consumeThread004 time = %d\n",getCurrentTime());
                    //检查是否读到卡
                    ret=serial_send(serialfd,SERIAL_READCARD_CMD,6);
                    myPtf("readcard send consumeThread0041 time = %d\n",getCurrentTime());
                    myPtf("serial_send:%d \n",ret);
                    //接收数据
                    memset(recvbuf,0,sizeof(recvbuf));
                    recvlen=19;//需要接收19个字符
                    recvleft=recvlen;
                    recvedlen=0;
                    LoopTimes=0;
                    while(recvleft>0)
                    {
                        //如果读到数据，但是不够19个字节，应该读完
                        ret=serial_recv(serialfd,recvbuf+(recvedlen),recvleft,500);
                        myPtf("readcard recv consumeThread0042 time = %d\n",getCurrentTime());
                        recvedlen+=ret;
                        recvleft-=ret;
                        if ((LoopTimes++)>=0)
                            break;
                    }
                    myPtf("serial_revlen:%d \nrecvbuf:",recvedlen);
                    for(i=0;i<recvedlen;i++)
                    {
                        myPtf("%02X",recvbuf[i]);
                    }
                    myPtf("\n");
                    //解析卡号
                    if((recvbuf[0]==0x05)&&(recvbuf[1]==0xE0)&&(recvbuf[2]==0x07)&&(recvbuf[8]==0x90)&&(recvbuf[9]==0x00))
                    {
                        myPtf("readcard OK consumeThread005 time = %d||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||\n\n\n\n",getCurrentTime());
                        myPtf("readcard OK consumeThread005 time = %d||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||\n\n\n\n",getCurrentTime());

                        //if(recvbuf[3]==0x51)
                        if(UseTermType == USERTYPE_BJKJG)
                        {
                            //卡码
                            curConsume.cardID = recvbuf[7]+(recvbuf[6]<<8)+(recvbuf[5]<<16)+(recvbuf[4]<<24);
                        }
                        else
                        {
                            //序列号
                            //中银广分要求，特殊卡片序列号处理，使用3字节卡号
                            curConsume.cardID = recvbuf[4]+(recvbuf[5]<<8)+(recvbuf[6]<<16);
                            //非广分版本应该使用4字节卡号
                            if(UseTermType != USERTYPE_BOC_GZ)
                            {
                               curConsume.cardID += recvbuf[7]<<24;
                            }
                        }
                        curConsume.fType = 0;
                        ////////////////////////////////////////////////////////////////////
                        //回调通知UI界面
                        memset(CallBackRetString,0,sizeof(CallBackRetString));
                        sprintf(CallBackRetString,"{\"timeStamp\":\"%d\",\"cmdType\":%d,\"state\":4}",flowID,curConsume.cmdType);
                        Cmd = CONSUME_CALLBACK_QUERET;
                        //sendCmd_afunix(cli_uicallback.clisockfd,cli_uicallback.clipath,cli_uicallback.serpath,&Cmd,CallBackRetString,strlen(CallBackRetString)+1);
                        ////////////////////////////////////////////////////////////////////
                        curConsume.status = CONSUME_HAVECARDNO;
                        //让扫码服务停止扫码
                        if ((curConsume.transactionType==TRSACTTYPE_SCANQRCODE)||(curConsume.transactionType==TRSACTTYPE_READANDSCAN))
                        {
                            Cmd = SCANQR_STOP_QRCODE_CMD;
                            myPtf("stop get qr\n");
                            sendCmd_afunix(cli_scanqr.clisockfd,cli_scanqr.clipath, cli_scanqr.serpath,&Cmd, NULL, 0);
                        }
                    }
                    else
                    {
                        myPtf("consume thread is sleep,curConsume.flowid=%d,flowID=%d,threadFlowID=%d\n",curConsume.flowID,flowID,threadFlowID);
                        usleep(5000);
                    }
                }
            }

            //读到卡号或二维码
            if(curConsume.status == CONSUME_HAVECARDNO)
            {
                //如果是脱机消费
                if(curConsume.cmdType == UI_REQ_OFFLINECONSUME)
                {
                    //从内存中判断卡号或账号是否正确
                    //////////////////////////////////////////////////
                    //判断账号卡号是否合法
                    //查找账户
                    int iFound=0;
                    if (curConsume.cardID != 0)
                    {
                        if(fn_FindKeyByCardId(pUserList,userNum,curConsume.cardID)==1)
                            iFound=1;
                    }
                    if (curConsume.accountID != 0)
                    {
                        if(fn_FindKeyByAccountId(pUserList,userNum,curConsume.accountID)==1)
                            iFound=1;
                    }
                    if(iFound == 1)
                    {
                        myPtf("User find！\n");
                        //////////////////////////////////////////////////
                        //写入内存和磁盘
                        offline_flow.cardNo = curConsume.cardID;
                        offline_flow.flowMoney = curConsume.flowMoney;
                        offline_flow.accountId = curConsume.accountID;
                        offline_flow.timeStamp = flowID;
                        offline_flow.curTime = getCurrentTime();
                        memset(CallBackRetString,0,sizeof(CallBackRetString));
                        if(fn_AddRecord((char *)(&offline_flow)) == 0)
                        {
                            //成功
                            sprintf(CallBackRetString,"{\"timeStamp\":\"%d\",\"cmdType\":%d,\"state\":1}",flowID,curConsume.cmdType);
                            myPtf("fn_AddRecord OK！\n");
                        }
                        else
                        {
                            //失败
                            sprintf(CallBackRetString,"{\"timeStamp\":\"%d\",\"cmdType\":%d,\"state\":2}",flowID,curConsume.cmdType);
                            myPtf("fn_AddRecord Error！\n");
                        }
                    }
                    else
                    {
                        //失败
                        sprintf(CallBackRetString,"{\"timeStamp\":\"%d\",\"cmdType\":%d,\"state\":2}",flowID,curConsume.cmdType);
                        myPtf("User not find！\n");
                    }
                    //回调通知UI界面
                    strcpy(CallBackRetString+strlen(CallBackRetString),curConsume.retstring+1);
                    Cmd = CONSUME_CALLBACK_QUERET;
                    sendCmd_afunix(cli_uicallback.clisockfd,cli_uicallback.clipath, cli_uicallback.serpath,&Cmd, CallBackRetString,strlen(CallBackRetString)+1);
                    curConsume.status = CONSUME_FINISH;
                    continue;
                }

                /////////////////////////////////////////////
                //如果是断网状态，直接返回
                if (curHeartbeat.ret < NEEDRETRY)
                {
                    myPtf("network is not connected! ret CLOUD_SENDCMD_ERROR\n");
                    sprintf(curConsume.retstring,"{\"flowID\":\"%d\",\"ret\":%d}",flowID,CLOUD_SENDCMD_ERROR);
                    ////////////////////////////////////////////////////////////////////
                    //回调通知UI界面
                    memset(CallBackRetString,0,sizeof(CallBackRetString));
                    sprintf(CallBackRetString,"{\"timeStamp\":\"%d\",\"cmdType\":%d,\"state\":1,",flowID,curConsume.cmdType);
                    strcpy(CallBackRetString+strlen(CallBackRetString),curConsume.retstring+1);
                    Cmd = CONSUME_CALLBACK_QUERET;
                    sendCmd_afunix(cli_uicallback.clisockfd,cli_uicallback.clipath, cli_uicallback.serpath,&Cmd, CallBackRetString,strlen(CallBackRetString)+1);
                    ////////////////////////////////////////////////////////////////////
                    curConsume.status = CONSUME_FINISH;
                    //退出本线程
                    continue;
                }

                /////////////////////////////////////////////
                //置状态标记
                time (&timep);
                myPtf("%s:read ok,curConsume.fType=%d,curConsume.status=CONSUME_HAVECARDNO\n",ctime(&timep),curConsume.fType);
                //记录流水产生时间
                time(&rawtime);
                ptminfo = localtime(&rawtime);
                sprintf(upTime,"%04d-%02d-%02d %02d:%02d:%02d",
                        ptminfo->tm_year + 1900, ptminfo->tm_mon + 1, ptminfo->tm_mday,
                        ptminfo->tm_hour, ptminfo->tm_min, ptminfo->tm_sec);
                //组织JSON串
                istrlen=0;
                memset(strCloudConsume,0,sizeof(strCloudConsume));
                //字符串这里不要换行写，不然会占用更大空间，甚至导致strCloudConsume越界.

                sprintf(strCloudConsume,"{\"areaID\":%d,\"upTime\":\"%s\",\"cardID\":%d,\"cmd\":%d,\"consumeNum\":\"%s\",\"contypeID\":%d,\"fType\":%d,\"flowID\":%d,\"flowMoney\":%d,\"pwd\":\"%s\",\"termCode\":\"%s\",\"termID\":%d,\"accountId\":%d}",
                                          curConsume.areaID,upTime,curConsume.cardID,curConsume.could_Cmd,curConsume.consumeNum,
                                          curConsume.contypeID,curConsume.fType,flowID,curConsume.flowMoney,
                                          curConsume.pwd,curConsume.termCode,curConsume.termID,curConsume.accountID);


                time (&timep);
                myPtf("%s:1consume json:%s\n",ctime(&timep),strCloudConsume);
                //向云端发送请求
                myPtf("send consume command to cloud consumeThread006 time = %d\n",getCurrentTime());
                strRecvConsume = sendCmd_socket(&send_cmd, strCloudConsume, strlen(strCloudConsume), &istrlen);
                myPtf("send consume command to cloud over consumeThread007 time = %d\n",getCurrentTime());
                //myPtf("1recv from cloud ,len=%d,data=%s",strRecvConsume,strRecvConsume);
                time (&timep);
                myPtf("%s:1recv from cloud ,data=%s",ctime(&timep),strRecvConsume);
                //考虑调用失败问题，服务器未收到该流水
                ////////////////////////////////////////////////////////////////////
                //回调通知UI界面
                /*
                memset(CallBackRetString,0,sizeof(CallBackRetString));
                sprintf(CallBackRetString,"{\"timeStamp\":\"%d\",\"cmdType\":%d,\"state\":4}",flowID,curConsume.cmdType);
                Cmd = CONSUME_CALLBACK_QUERET;
                sendCmd_afunix(cli_uicallback.clisockfd,cli_uicallback.clipath,cli_uicallback.serpath,&Cmd,CallBackRetString,strlen(CallBackRetString)+1);
                */
                ////////////////////////////////////////////////////////////////////
                curConsume.status = CONSUME_WAITFORCLOUD;
                //如果未返回结果
                if(strRecvConsume==NULL)
                {
                    //如果是消费且未返回结果，继续调后续查询接口
                    if(curConsume.could_Cmd==CLD_CONSUME_CMD)
                    {
                        myPtf("sendCmd_socket first error!\n");
                        //sprintf(curConsume.retstring,"{\"flowID\":\"%d\",\"ret\":%d}",flowID,CLOUD_SENDCMD_ERROR);
                        continue;
                    }
                    //如果是查询账号信息且未返回结果，则在返回串中写错误
                    else if (curConsume.could_Cmd==CLD_GETACCOINFO_CMD)
                    {
                        sprintf(curConsume.retstring,"{\"flowID\":\"%d\",\"ret\":%d}",flowID,send_cmd);
                        curConsume.status = CONSUME_FINISH;
                    }
                }
                //如果返回结果，填写结果
                else
                {
                    //解析返回值,查找子串ret=100
                    if (strstr(strRecvConsume, waitString)==NULL)
                    {
                        //不是等待支付标识，标识本次操作已完成
                        ////////////////////////////////////////////////////////////////////
                        curConsume.status = CONSUME_FINISH;
                    }
                    else
                    {
                        //有等待支付标识，需要通知UI当前状态，并继续从云端查询结果
                        //else里啥也不做，只是为了写注释
                    }
                    memcpy(curConsume.retstring,strRecvConsume,istrlen);
                    free(strRecvConsume);
                    strRecvConsume = NULL;
                }
                ////////////////////////////////////////////////////////////////////
                if (curConsume.status == CONSUME_FINISH)
                {
                    //回调通知UI界面
                    memset(CallBackRetString,0,sizeof(CallBackRetString));
                    sprintf(CallBackRetString,"{\"timeStamp\":\"%d\",\"cmdType\":%d,\"state\":1,",flowID,curConsume.cmdType);
                    strcpy(CallBackRetString+strlen(CallBackRetString),curConsume.retstring+1);
                    Cmd = CONSUME_CALLBACK_QUERET;
                    myPtf("send callbackret start time = %d\n",getCurrentTime());
                    sendCmd_afunix(cli_uicallback.clisockfd,cli_uicallback.clipath, cli_uicallback.serpath,&Cmd, CallBackRetString,strlen(CallBackRetString)+1);
                    myPtf("send callbackret end time = %d\n",getCurrentTime());
                }
                //退出本线程
                continue;

            }
            else if(curConsume.status==CONSUME_WAITFORCARD)
            //未读到卡号或二维码，此时curConsume.status任然为1，继续续等待刷卡或扫码
            {
                myPtf("not read cardno or qr,curConsume.status:%d\n",curConsume.status);
                continue;
            }
            //发送云端查询请求
            if(curConsume.status==CONSUME_WAITFORCLOUD)
            {
                myPtf("send to cloud,curConsume.status==%d\n",curConsume.status);
                //发送云端查询请求
                istrlen=0;
                memset(strCloudConsume,0,sizeof(strCloudConsume));
                sprintf(strCloudConsume,"{\"cmd\":%d,\"flowID\":%d,\"termCode\":\"%s\",\"termID\":%d,\"fType\":%d}",
                        CLD_GETCONSUMERET_CMD,flowID,curConsume.termCode,curConsume.termID,curConsume.fType);
                myPtf("2consume json:%s\n",strCloudConsume);
                //向云端发送请求
                strRecvConsume = sendCmd_socket(&send_cmd, strCloudConsume, strlen(strCloudConsume), &istrlen);
                myPtf("ret2222222222222\n");
                myPtf("2recv from cloud ,len=%d,data=%s",istrlen,strRecvConsume);
                //如果未返回结果，表示网络断开，返回失败
                if(strRecvConsume == NULL)
                {
                    myPtf("1curConsume.flowid=%d,flowID=%d,threadFlowID=%d",curConsume.flowID,flowID,threadFlowID);
                    myPtf("sendCmd_socket second error!\n");
                    sprintf(curConsume.retstring,"{\"flowID\":\"%d\",\"ret\":%d}",flowID,CLOUD_SENDCMD_ERROR);
                    ////////////////////////////////////////////////////////////////////
                    //回调通知UI界面
                    memset(CallBackRetString,0,sizeof(CallBackRetString));
                    sprintf(CallBackRetString,"{\"timeStamp\":\"%d\",\"cmdType\":%d,\"state\":1,",flowID,curConsume.cmdType);
                    strcpy(CallBackRetString+strlen(CallBackRetString),curConsume.retstring+1);
                    Cmd = CONSUME_CALLBACK_QUERET;
                    sendCmd_afunix(cli_uicallback.clisockfd,cli_uicallback.clipath, cli_uicallback.serpath,&Cmd, CallBackRetString,strlen(CallBackRetString)+1);
                    ////////////////////////////////////////////////////////////////////
                    curConsume.status = CONSUME_FINISH;
                    continue;
                }
                //解析返回值,查找子串ret=100
                if (strstr(strRecvConsume, waitString)==NULL)
                {
                    //不是等待支付标识，表示本次操作已完成
                    ////////////////////////////////////////////////////////////////////
                    memcpy(curConsume.retstring,strRecvConsume,istrlen);
                    //回调通知UI界面
                    memset(CallBackRetString,0,sizeof(CallBackRetString));
                    sprintf(CallBackRetString,"{\"timeStamp\":\"%d\",\"cmdType\":%d,\"state\":1,",flowID,curConsume.cmdType);
                    strcpy(CallBackRetString+strlen(CallBackRetString),curConsume.retstring+1);
                    Cmd = CONSUME_CALLBACK_QUERET;
                    sendCmd_afunix(cli_uicallback.clisockfd,cli_uicallback.clipath, cli_uicallback.serpath,&Cmd, CallBackRetString,strlen(CallBackRetString)+1);
                    ////////////////////////////////////////////////////////////////////
                    curConsume.status = CONSUME_FINISH;
                }
                free(strRecvConsume);
                strRecvConsume = NULL;
            }
        }
        threadFlowID = 0;
        myPtf("end consumeThread008 threadFlowID=%d, time = %d\n",threadFlowID,getCurrentTime());
    }
    return (void *)NULL;
}

char * fn_dealCmd_Echo(char *recv_buf,int recv_len,int * ret_len,int *send_cmd)
{
    char * ret_buf = (char *)malloc(recv_len);
    memcpy(ret_buf,recv_buf,recv_len);
    myPtf("dealcmd:recvbuf=%s\n",ret_buf);
    *ret_len = recv_len;
    *send_cmd = ECHO_CMD;
    //返回
    myPtf("ret_len=%d,ret_buf=%s\n",*ret_len,ret_buf);
    return ret_buf;
}

int fn_GetServerTime(int *send_cmd)
{
    //////////////////////////////////////////////////////
    //获取时间JSON串
    cJSON * jRoot;
    char strGetTime[20]="{\"cmd\":3008}";
    int iTimelen=0;
    myPtf("gettime:%s\n",strGetTime);
    char * strRecvTime = sendCmd_socket(send_cmd, strGetTime, strlen(strGetTime), &iTimelen);
    myPtf("ret len=%d ,str=%s|||||\n",iTimelen,strRecvTime);
    if (strRecvTime==NULL)
    {
        myPtf("not get recvtime\n");
        *send_cmd = CLOUD_SENDCMD_ERROR;
        return -1;
    }
    //解析Json串
    jRoot=cJSON_Parse((char *)strRecvTime);
    if(!jRoot)
    {
        myPtf("get not json root\n");
        *send_cmd = JSON_ERROR;
        free(strRecvTime);
        //json串错误
        return -1;
    }
    cJSON * jTime=cJSON_GetObjectItem(jRoot,"nowTime");
    if(!jTime)
    {
        myPtf("get not json nowTime\n");
        *send_cmd = JSON_NOITEM_ERROR;
        cJSON_Delete(jRoot);
        free(strRecvTime);
        //json串错误
        return -1;
    }
    //解析时间
    char strTime[20] = "";
    //strcpy(strTime,jTime->valuestring);
    switch (jTime->type)
    {
    case cJSON_Number:
        myPtf("jTime type is cJSON_Number\n");
        sprintf(strTime,"%d",jTime->valueint);
        myPtf("strTime=%s,jTime=%d\n",strTime,jTime->valueint);
        break;
    case cJSON_String:
        myPtf("jTime type is cJSON_String\n");
        strcpy(strTime,jTime->valuestring);
        break;
    default:
        myPtf("nowTime format error\n");
        cJSON_Delete(jRoot);
        free(strRecvTime);
        *send_cmd = NORECVBUF_ERROR;
        return -1;
    }


    myPtf("get time %s\n",strTime);
    cJSON_Delete(jRoot);
    free(strRecvTime);
    //修改系统时间
    myPtf("server time is %s\n",strTime);
    time_t sertime = FormatTime(strTime);
    int ret = stime(&sertime);  //修改时间需要root权限
    myPtf("modify time ret = %d\n",ret);
    return 0;
}

//获取终端参数
char * fn_dealCmd_GetTermInfo(char *recv_buf,int recv_len,int * ret_len,int *send_cmd)
{
    /*
    //////////////////////////////////////////////////////
    //获取时间JSON串
    cJSON * jRoot;
    *ret_len = 0;
    char strGetTime[20]="{\"cmd\":3008}";
    int iTimelen=0;
    myPtf("gettime:%s\n",strGetTime);
    char * strRecvTime = sendCmd_socket(send_cmd, strGetTime, strlen(strGetTime), &iTimelen);
    myPtf("ret len=%d ,str=%s|||||\n",iTimelen,strRecvTime);
    if (strRecvTime==NULL)
    {
        myPtf("not get recvtime\n");
        *send_cmd = CLOUD_SENDCMD_ERROR;
        return NULL;
    }
    //解析Json串
    jRoot=cJSON_Parse((char *)strRecvTime);
    if(!jRoot)
    {
        myPtf("get not json root\n");
        *send_cmd = JSON_ERROR;
        free(strRecvTime);
        //json串错误
        return NULL;
    }
    cJSON * jTime=cJSON_GetObjectItem(jRoot,"nowTime");
    if(!jTime)
    {
        myPtf("get not json nowTime\n");
        *send_cmd = JSON_NOITEM_ERROR;
        cJSON_Delete(jRoot);
        free(strRecvTime);
        //json串错误
        return NULL;
    }
    //解析时间
    char strTime[20] = "";
    //strcpy(strTime,jTime->valuestring);
    switch (jTime->type)
    {
    case cJSON_Number:
        myPtf("jTime type is cJSON_Number\n");
        sprintf(strTime,"%d",jTime->valueint);
        myPtf("strTime=%s,jTime=%d\n",strTime,jTime->valueint);
        break;
    case cJSON_String:
        myPtf("jTime type is cJSON_String\n");
        strcpy(strTime,jTime->valuestring);
        break;
    default:
        myPtf("nowTime format error\n");
        cJSON_Delete(jRoot);
        free(strRecvTime);
        *send_cmd = NORECVBUF_ERROR;
        return NULL;
    }


    myPtf("get time %s\n",strTime);
    cJSON_Delete(jRoot);
    free(strRecvTime);
    //修改系统时间
    myPtf("server time is %s\n",strTime);
    time_t sertime = FormatTime(strTime);
    int ret = stime(&sertime);  //修改时间需要root权限
    myPtf("modify time ret = %d\n",ret);
    */
    if(fn_GetServerTime(send_cmd)<0)
    {
        return NULL;
    }
    //////////////////////////////////////////////////////
    *ret_len=0;
    if (recv_buf==NULL)
    {
        myPtf("NORECVBUF_ERROR\n");
        *send_cmd = NORECVBUF_ERROR;
        return NULL;
    }
    //识别当前终端唯一识别号
    //解析Json串
    cJSON * jRoot;
    jRoot=cJSON_Parse((char *)recv_buf);
    if(!jRoot)
    {
        *send_cmd = JSON_ERROR;
        //json串错误
        return NULL;
    }
    cJSON * jTermCode=cJSON_GetObjectItem(jRoot,"termCode");
    if(!jTermCode)
    {
        *send_cmd = JSON_NOITEM_ERROR;
        cJSON_Delete(jRoot);
        //json串错误
        return NULL;
    }
    else
    {
        strcpy(termCode,jTermCode->valuestring);
        strcpy(ParaConfig.termCode,termCode);
        //写入参数成功
        fn_WriteConfig((char *)(&ParaConfig),sizeof(config_t));
    }
    //TermCode为字符串类型
    /*
    memset(TERMCODE,0,sizeof(TERMCODE));
    strcpy(TERMCODE,jTermCode->valuestring);
    myPtf("TERMCODE=%s,jTermCode=%s,len=%d,size=%d\n",TERMCODE,jTermCode->valuestring,strlen(TERMCODE),sizeof(TERMCODE));
    cJSON_Delete(jRoot);
    */
    //组织发送串
    int cloudCmdLen=0;
    /*
    char strtemp[]="{\"cmd\":3003,\"termCode\":\"nktyterm0001\"}";
    cloudCmdLen = strlen(strtemp);
    char * strCloudCmd = malloc(cloudCmdLen+1);
    strcpy(strCloudCmd,strtemp);
    */
    char * strCloudCmd = addCmd(CLD_GETTERMINFO_CMD,recv_buf,recv_len,&cloudCmdLen);

    if(strCloudCmd == NULL)
    {
        *send_cmd = JSON_ADDCMD_ERROR;
        //json串错误
        return NULL;
    }
    //接收字符串
    int cloudRevLen=0;
    myPtf("strlen=%d,cloudCmdlen=%d\n",strlen(strCloudCmd),cloudCmdLen);
    char * strCloudRecv = sendCmd_socket(send_cmd, strCloudCmd, cloudCmdLen, &cloudRevLen);
    myPtf("return ok!11111111111111111\n");
    free (strCloudCmd);
    myPtf("return ok!22222222222222222\n");
    if(strCloudRecv == NULL)
    {
        *send_cmd = CLOUD_SENDCMD_ERROR;
        return NULL;
    }
    //获取终端号\分区号
    //////////////////////////////////////
    //解析Json串
    jRoot=cJSON_Parse((char *)strCloudRecv);
    if(!jRoot)
    {
        *send_cmd = RET_JSON_ERROR;
        free(strCloudRecv);
        //json串错误
        return NULL;
    }
    cJSON * jTerm=cJSON_GetObjectItem(jRoot,"term");
    if(jTerm)
    {
        //termid和areaid都是整形
        cJSON * jTermID=cJSON_GetObjectItem(jTerm,"termID");
        if(jTermID)
        {
            curConsume.termID = jTermID->valueint;
        }
        cJSON * jAreaID=cJSON_GetObjectItem(jTerm,"areaID");
        if(jAreaID)
        {
            curConsume.areaID = jAreaID->valueint;
        }
        //wang mao tan 2019-07-25 获取终端传过来的商户号
        cJSON * jMerchantNumber=cJSON_GetObjectItem(jTerm,"merchantID");
        if(jMerchantNumber){
            curConsume.merchantID = jMerchantNumber->valueint;
        }
    }
    ///////////////////////////////////////////////////////
    /*
    //获取账户列表
    char strGetUserList[100]="";
    int iRetlen=0;
    int pageNum=1;
    int pageSize=50;
    while(pageNum<600)
    {
        sprintf(strGetUserList,"{\"cmd\":%d,\"termCode\":\"%s\",\"pageNum\":\"%d\",\"pageSize\":\"%d\"}",CLD_GETACCOLIST_CMD,termCode,pageNum,pageSize);
        myPtf("CLD_GETACCOLIST_CMD %s\n",strGetUserList);
        char * strRecvTime = sendCmd_socket(send_cmd, strGetUserList, strlen(strGetUserList), &iRetlen);
        myPtf("ret len=%d ,str=%s|||||\n",iRetlen,strRecvTime);
        if(iRetlen<50)
        {
            myPtf("userlist over|||||\n");
            break;
        }
        pageNum++;
    }
    */
    ///////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////
    cJSON_Delete(jRoot);

    *ret_len = strlen(strCloudRecv)+1;  //OK
    *send_cmd=UI_GET_TERMINFO + 10000;
    //返回
    myPtf("ret_len=%d,ret_buf=%s\n",*ret_len,strCloudRecv);
    return strCloudRecv;
}

//取消某次（现在是最后一次，由终端控制）消费流水
char * fn_dealCmd_CancelFlow(char *recv_buf,int recv_len,int * ret_len,int *send_cmd)
{
    if (recv_buf==NULL)
    {
        myPtf("NORECVBUF_ERROR\n");
        *ret_len = 0;
        *send_cmd = NORECVBUF_ERROR;
        return NULL;
    }
    //组织发送串
    int cloudCmdLen=0;
    //myPtf("recvcmdjson:%s,jsonlen=%d\n",recv_buf,recv_len);
    char * strCloudCmd = addCmd(CLD_CANCELFLOW_CMD,recv_buf,recv_len,&cloudCmdLen);
    //myPtf("andcmdjson:%s,jsonlen=%d\n",strCloudCmd,cloudCmdLen);
    if(strCloudCmd == NULL)
    {
        *send_cmd = JSON_ADDCMD_ERROR;
        //json串错误
        return NULL;
    }
    //接收字符串
    int cloudRevLen=0;
    char * strCloudRecv = sendCmd_socket(send_cmd, strCloudCmd, cloudCmdLen, &cloudRevLen);
    free (strCloudCmd);
    if(strCloudRecv == NULL)
    {
        *send_cmd = CLOUD_SENDCMD_ERROR;
        return NULL;
    }
    *ret_len = cloudRevLen;
    *send_cmd=UI_EXEC_CANCELFLOW + 10000;
    //返回
    myPtf("ret_len=%d,strCloudRecv=%s\n",*ret_len,strCloudRecv);
    return strCloudRecv;
}

//获取消费总额
char * fn_dealCmd_GetConsumeInfo(char *recv_buf,int recv_len,int * ret_len,int *send_cmd)
{
    if (recv_buf==NULL)
    {
        myPtf("NORECVBUF_ERROR\n");
        *ret_len = 0;
        *send_cmd = NORECVBUF_ERROR;
        return NULL;
    }
    //组织发送串
    int cloudCmdLen=0;
    //myPtf("recvcmdjson:%s,jsonlen=%d\n",recv_buf,recv_len);
    char * strCloudCmd = addCmd(CLD_GETFLOWT_CMD,recv_buf,recv_len,&cloudCmdLen);
    //myPtf("andcmdjson:%s,jsonlen=%d\n",strCloudCmd,cloudCmdLen);
    if(strCloudCmd == NULL)
    {
        *send_cmd = JSON_ADDCMD_ERROR;
        //json串错误
        return NULL;
    }
    //接收字符串
    int cloudRevLen=0;
    char * strCloudRecv = sendCmd_socket(send_cmd, strCloudCmd, cloudCmdLen, &cloudRevLen);
    free (strCloudCmd);
    if(strCloudRecv == NULL)
    {
        *send_cmd = CLOUD_SENDCMD_ERROR;
        return NULL;
    }
    *ret_len = cloudRevLen;
    *send_cmd=UI_GET_CONSUMEINFO + 10000;
    //返回
    myPtf("ret_len=%d,strCloudRecv=%s\n",*ret_len,strCloudRecv);
    return strCloudRecv;
}

//脱机消费
char * fn_dealCmd_ReqOfflineConsume(char *recv_buf,int recv_len,int * ret_len,int *send_cmd)
{
    //将收到的参数填入结构
    *ret_len=0;
    int Cmd=0;;
    time_t timep;
    if (recv_buf==NULL)
    {
        myPtf("NORECVBUF_ERROR\n");
        *send_cmd = NORECVBUF_ERROR;
        return NULL;
    }
    //解析Json串
    cJSON * jRoot=cJSON_Parse((char *)recv_buf);
    if(!jRoot)
    {
        *send_cmd = JSON_ERROR;
        //json串错误
        return NULL;
    }
    cJSON * jMoney=cJSON_GetObjectItem(jRoot,"money");
    if(!jMoney)
    {
        *send_cmd = JSON_NOITEM_ERROR;
        cJSON_Delete(jRoot);
        //json串错误
        return NULL;
    }
    cJSON * jTimeStamp=cJSON_GetObjectItem(jRoot,"timeStamp");
    if(!jTimeStamp)
    {
        *send_cmd = JSON_NOITEM_ERROR;
        cJSON_Delete(jRoot);
        //json串错误
        return NULL;
    }
    cJSON * jTermCode=cJSON_GetObjectItem(jRoot,"termCode");
    if(!jTermCode)
    {
        *send_cmd = JSON_NOITEM_ERROR;
        cJSON_Delete(jRoot);
        //json串错误
        return NULL;
    }
    time (&timep);
    myPtf("%s:wait for thread over!!!\n",ctime(&timep));
    myPtf("start consume002 time = %d\n",getCurrentTime());
    curConsume.flowID = atoi(jTimeStamp->valuestring);
    //如果是同一个时间戳，直接退出
    if (curConsume.flowID == threadFlowID)
    {
        *send_cmd = TIMESTAMP_ERROR;
        cJSON_Delete(jRoot);
        //json串错误
        return NULL;
    }
    myPtf("let us waitting!\n");
    //等待上一笔线程结束
    while(threadFlowID != 0)
    {
        usleep(50000);
        myPtf("wait for threadFlowID to zero!\n");
    }
    time (&timep);
    myPtf("%s:thread is over!\n",ctime(&timep));
    //检查，如果没有用户数据，重新载入账户信息
    if ((userNum==0)||(pUserList==NULL))
    {
        //回读一遍全账户
        if (pUserList != NULL)
        {
            free(pUserList);
            userNum = 0;
        }
        pUserList = fn_ReadAccoPack(&userNum);
    }
    //如果用户终端类型为空，则读取本机配置信息
    if (UseTermType == 0)
    {
        int ret = fn_ReadConfig((char *)(&ParaConfig),sizeof(config_t));
        myPtf("fn_ReadConfig:ret = %d",ret);
        if (ret==0)
        {
            myPtf("read config UseTermType=%d,ParaConfig.UseTermType=%d!\n",UseTermType,ParaConfig.UseTermType);
            UseTermType=ParaConfig.UseTermType;
        }
        myPtf("read config UseTermType=%d,ParaConfig.UseTermType=%d!\n",UseTermType,ParaConfig.UseTermType);
    }
    //结构清空
    fn_cleanConsumeStructButflowID();
    //TermCode为字符串类型
    curConsume.flowMoney = atoi(jMoney->valuestring);
    curConsume.flowID = atoi(jTimeStamp->valuestring);
    curConsume.cmdType = UI_REQ_OFFLINECONSUME;
    strcpy(curConsume.termCode,jTermCode->valuestring);

    //如果没有得到卡号，则到达读卡状态
    curConsume.fType = -1;
    ////////////////////////////////////////////////////////////////////
    //回调通知UI界面
    memset(CallBackRetString,0,sizeof(CallBackRetString));
    sprintf(CallBackRetString,"{\"timeStamp\":\"%d\",\"cmdType\":%d,\"state\":5}",curConsume.flowID,curConsume.cmdType);
    Cmd = CONSUME_CALLBACK_QUERET;
    sendCmd_afunix(cli_uicallback.clisockfd,cli_uicallback.clipath,cli_uicallback.serpath,&Cmd,CallBackRetString,strlen(CallBackRetString)+1);
    ////////////////////////////////////////////////////////////////////
    curConsume.status = CONSUME_WAITFORCARD;

    myPtf("end callback !!\n");

    //释放
    cJSON_Delete(jRoot);
    //启动消费服务线程,并将结构传入
    threadFlowID = curConsume.flowID;

	myPtf("pthread_create consume thread shourd work!\n");
    //等待本次线程启动
    while(threadFlowID == 0)
    {
        usleep(10000);
        myPtf("consume thread is not start threadFlowID=%d\n",threadFlowID);
    }
    myPtf("consume thread is start threadFlowID=%d\n",threadFlowID);
	//组织返回串
	char strRet[1024];
	sprintf(strRet,"{\"timeStamp\":\"%d\",\"state\":1,\"result\":\"\"}",curConsume.flowID);

    char * ret_buf = (char *)malloc(strlen(strRet)+1);
    memset(ret_buf,0,strlen(strRet)+1);
    memcpy(ret_buf,strRet,strlen(strRet));
    time (&timep);
    myPtf("%s:dealcmd:recvbuf=%s\n",ctime(&timep),ret_buf);
    *ret_len = strlen(strRet)+1;
    *send_cmd = UI_REQ_OFFLINECONSUME + 10000;
    myPtf("start consume004 time = %d\n",getCurrentTime());
    //返回成功
    return ret_buf;
}

//查询离线流水数量
char * fn_dealCmd_GetOfflineFlowNum(char *recv_buf,int recv_len,int * ret_len,int *send_cmd)
{
    int ret = 0;
    char strRet[50];
	sprintf(strRet,"{\"flowCount\":%d,\"state\":1}",fn_GetOffline_FlowNum());
    char * ret_buf = (char *)malloc(strlen(strRet)+1);
    memset(ret_buf,0,strlen(strRet)+1);
    memcpy(ret_buf,strRet,strlen(strRet));
    *ret_len = strlen(strRet)+1;
    *send_cmd = UI_GET_OFFLINEFLOWNUM + 10000;
    return ret_buf;
}

//下载用户名单列表
char * fn_dedlCmd_DownloadUserList(char *recv_buf,int recv_len,int * ret_len,int *send_cmd)
{
    int ret = 1;
    char strRet[50];


    //启动脱机上传线程
    pthread_t downloaduserlist_thread;
    if (threadDownloadUserList == 0)
    {
        threadDownloadUserList = 1;
        ret = pthread_create(&downloaduserlist_thread, NULL, DownLoadUserThread, NULL);
    }
    else
    {
        ret = -1;
    }
    //组织返回串
    if (ret < 0) {
        myPtf("create DownLoadUserThread thread error! ret = %d\n",ret);
    }
    else
    {
        ret = 1;
    }
    PageNum = 1;
    //state=1表示成功
	sprintf(strRet,"{\"state\":1,\"result\":\"\"}");
    char * ret_buf = (char *)malloc(strlen(strRet)+1);
    memset(ret_buf,0,strlen(strRet)+1);
    memcpy(ret_buf,strRet,strlen(strRet));
    *ret_len = strlen(strRet)+1;
    *send_cmd = UI_REQ_DOWNLOADUSERLIST + 10000;
    return ret_buf;
}
//下载名单到第几包
char * fn_dedlCmd_GetUserListPackNum(char *recv_buf,int recv_len,int * ret_len,int *send_cmd)
{
    int ret = 0;
    char strRet[50];
	sprintf(strRet,"{\"UserPackCount\":%d,\"state\":1}",PageNum);
    char * ret_buf = (char *)malloc(strlen(strRet)+1);
    memset(ret_buf,0,strlen(strRet)+1);
    memcpy(ret_buf,strRet,strlen(strRet));
    *ret_len = strlen(strRet)+1;
    *send_cmd = UI_GET_USERLISTNUM + 10000;
    return ret_buf;
}

//脱机流水上传
char * fn_dealCmd_ReqUploadFlow(char *recv_buf,int recv_len,int * ret_len,int *send_cmd)
{
    int ret = 0;
    char strRet[50];
    if (fn_GetOffline_FlowNum() != 0)
    {
        //启动脱机上传线程
        pthread_t uplaodflow_thread;
        if (threadUploadFlow == 0)
        {
            threadUploadFlow = 1;
            ret = pthread_create(&uplaodflow_thread, NULL, UploadFlowThread, NULL);
        }
        else
        {
            ret = -1;
        }
        //组织返回串
        if (ret < 0) {
            myPtf("create UploadFlowThread thread error! ret = %d\n",ret);
        }
        else
        {
            ret = 1;
        }
    }
	sprintf(strRet,"{\"state\":%d,\"result\":\"\"}",ret);
    char * ret_buf = (char *)malloc(strlen(strRet)+1);
    memset(ret_buf,0,strlen(strRet)+1);
    memcpy(ret_buf,strRet,strlen(strRet));
    *ret_len = strlen(strRet)+1;
    *send_cmd = UI_REQ_UPLOADFLOW + 10000;
    return ret_buf;
}

//请求消费
char * fn_dealCmd_ReqConsume(char *recv_buf,int recv_len,int * ret_len,int *send_cmd)
{
    //将收到的参数填入结构
    myPtf("start consume001 time = %d\n",getCurrentTime());
    *ret_len=0;
    int Cmd=0;;
    time_t timep;
    if (recv_buf==NULL)
    {
        myPtf("NORECVBUF_ERROR\n");
        *send_cmd = NORECVBUF_ERROR;
        return NULL;
    }
    if (curHeartbeat.ret < NEEDRETRY)
    {
        myPtf("CLOUD_SENDCMD_ERROR\n");
        *send_cmd = CLOUD_SENDCMD_ERROR;
        return NULL;
    }
    //解析Json串
    cJSON * jRoot=cJSON_Parse((char *)recv_buf);
    if(!jRoot)
    {
        *send_cmd = JSON_ERROR;
        //json串错误
        return NULL;
    }
    cJSON * jMoney=cJSON_GetObjectItem(jRoot,"money");
    if(!jMoney)
    {
        *send_cmd = JSON_NOITEM_ERROR;
        cJSON_Delete(jRoot);
        //json串错误
        return NULL;
    }
    cJSON * jType=cJSON_GetObjectItem(jRoot,"type");
    if(!jType)
    {
        *send_cmd = JSON_NOITEM_ERROR;
        cJSON_Delete(jRoot);
        //json串错误
        return NULL;
    }
    cJSON * jTimeStamp=cJSON_GetObjectItem(jRoot,"timeStamp");
    if(!jTimeStamp)
    {
        *send_cmd = JSON_NOITEM_ERROR;
        cJSON_Delete(jRoot);
        //json串错误
        return NULL;
    }
    cJSON * jPwd=cJSON_GetObjectItem(jRoot,"pwd");
    if(!jPwd)
    {
        *send_cmd = JSON_NOITEM_ERROR;
        cJSON_Delete(jRoot);
        //json串错误
        return NULL;
    }
    cJSON * jTermCode=cJSON_GetObjectItem(jRoot,"termCode");
    if(!jTermCode)
    {
        *send_cmd = JSON_NOITEM_ERROR;
        cJSON_Delete(jRoot);
        //json串错误
        return NULL;
    }
    //termid和areaid都是整形
    cJSON * jTermID=cJSON_GetObjectItem(jRoot,"termID");
    if(!jTermID)
    {
        *send_cmd = JSON_NOITEM_ERROR;
        cJSON_Delete(jRoot);
        //json串错误
        return NULL;
    }
    cJSON * jAreaID=cJSON_GetObjectItem(jRoot,"areaID");
    if(!jAreaID)
    {
        *send_cmd = JSON_NOITEM_ERROR;
        cJSON_Delete(jRoot);
        //json串错误
        return NULL;
    }
    cJSON * jTransactionType=cJSON_GetObjectItem(jRoot,"transactionType");
    if(!jTransactionType)
    {
        *send_cmd = JSON_NOITEM_ERROR;
        cJSON_Delete(jRoot);
        //json串错误
        return NULL;
    }
    cJSON * jCardID=cJSON_GetObjectItem(jRoot,"cardId");
    if(!jCardID)
    {
        *send_cmd = JSON_NOITEM_ERROR;
        cJSON_Delete(jRoot);
        //json串错误
        return NULL;
    }
    time (&timep);
    myPtf("%s:wait for thread over!!!\n",ctime(&timep));
    myPtf("start consume002 time = %d\n",getCurrentTime());
    curConsume.flowID = atoi(jTimeStamp->valuestring);
    //如果是同一个时间戳，直接退出
    if (curConsume.flowID == threadFlowID)
    {
        *send_cmd = TIMESTAMP_ERROR;
        cJSON_Delete(jRoot);
        //json串错误
        return NULL;
    }
    myPtf("let us waitting!\n");
    //等待上一笔线程结束
    while(threadFlowID != 0)
    {
        usleep(50000);
        myPtf("wait for threadFlowID to zero!\n");
    }
    time (&timep);
    myPtf("%s:thread is over!!!\n",ctime(&timep));
    myPtf("start consume003 time = %d\n",getCurrentTime());
    //结构清空
    fn_cleanConsumeStructButflowID();
    //TermCode为字符串类型
    curConsume.flowMoney = atoi(jMoney->valuestring);
    curConsume.flowID = atoi(jTimeStamp->valuestring);
    curConsume.contypeID = atoi(jType->valuestring);
    strcpy(curConsume.pwd,jPwd->valuestring);
    curConsume.could_Cmd = CLD_CONSUME_CMD;
    curConsume.cmdType = UI_REQ_CONSUME;
    curConsume.termID = jTermID->valueint;
    curConsume.areaID = jAreaID->valueint;
    curConsume.cardID = jCardID->valueint;
    curConsume.transactionType = jTransactionType->valueint;
    strcpy(curConsume.termCode,jTermCode->valuestring);

    //写状态,如果已经有卡号，表示通过自检已经获取卡号，则直接跳过读卡流程
    myPtf("start callback !!\n");
    if (curConsume.cardID != 0)
    {
        curConsume.fType = 0;
        ////////////////////////////////////////////////////////////////////
        //回调通知UI界面
        memset(CallBackRetString,0,sizeof(CallBackRetString));
        sprintf(CallBackRetString,"{\"timeStamp\":\"%d\",\"cmdType\":%d,\"state\":4}",curConsume.flowID,curConsume.cmdType);
        Cmd = CONSUME_CALLBACK_QUERET;
        sendCmd_afunix(cli_uicallback.clisockfd,cli_uicallback.clipath,cli_uicallback.serpath,&Cmd,CallBackRetString,strlen(CallBackRetString)+1);
        ////////////////////////////////////////////////////////////////////
        curConsume.status = CONSUME_HAVECARDNO;
    }
    //如果没有得到卡号，则到达读卡状态
    else
    {
        curConsume.fType = -1;
        ////////////////////////////////////////////////////////////////////
        //回调通知UI界面
        memset(CallBackRetString,0,sizeof(CallBackRetString));
        sprintf(CallBackRetString,"{\"timeStamp\":\"%d\",\"cmdType\":%d,\"state\":5}",curConsume.flowID,curConsume.cmdType);
        Cmd = CONSUME_CALLBACK_QUERET;
        sendCmd_afunix(cli_uicallback.clisockfd,cli_uicallback.clipath,cli_uicallback.serpath,&Cmd,CallBackRetString,strlen(CallBackRetString)+1);
        ////////////////////////////////////////////////////////////////////
        curConsume.status = CONSUME_WAITFORCARD;
    }
    myPtf("end callback !!\n");

    //释放
    cJSON_Delete(jRoot);
    //启动消费服务线程,并将结构传入
    pthread_t thread;
    myPtf("pthread_create consume thread is start!\n");
    threadFlowID = curConsume.flowID;
    /*
	int ret = pthread_create(&thread, NULL, ConsumeThread, &curConsume.flowID);
	if (ret < 0) {
		myPtf("create consume thread error! ret = %d\n",ret);
        *send_cmd = CREATE_CONSUMETHREAD_ERROR;
        //json串错误
        return NULL;
	}*/
	myPtf("pthread_create consume thread shourd work!\n");
    //等待本次线程启动
    while(threadFlowID == 0)
    {
        usleep(10000);
        myPtf("consume thread is not start threadFlowID=%d\n",threadFlowID);
    }
    myPtf("consume thread is start threadFlowID=%d\n",threadFlowID);
	//组织返回串
	char strRet[1024];
	sprintf(strRet,"{\"timeStamp\":\"%d\",\"state\":1,\"result\":\"\"}",curConsume.flowID);

    char * ret_buf = (char *)malloc(strlen(strRet)+1);
    memset(ret_buf,0,strlen(strRet)+1);
    memcpy(ret_buf,strRet,strlen(strRet));
    time (&timep);
    myPtf("%s:dealcmd:recvbuf=%s\n",ctime(&timep),ret_buf);
    *ret_len = strlen(strRet)+1;
    *send_cmd=UI_REQ_CONSUME + 10000;
    myPtf("start consume004 time = %d\n",getCurrentTime());
    //返回成功
    return ret_buf;
}
//查询消费结果
char * fn_dealCmd_QueConsumeRet(char *recv_buf,int recv_len,int * ret_len,int *send_cmd)
{
    //解析字符串
    //将收到的参数填入结构
    *ret_len=0;
    int Cmd = 0;
    if (recv_buf==NULL)
    {
        myPtf("NORECVBUF_ERROR\n");
        *send_cmd = NORECVBUF_ERROR;
        return NULL;
    }
    //解析Json串
    cJSON * jRoot=cJSON_Parse((char *)recv_buf);
    if(!jRoot)
    {
        *send_cmd = JSON_ERROR;
        //json串错误
        return NULL;
    }
    cJSON * jTimeStamp=cJSON_GetObjectItem(jRoot,"timeStamp");
    if(!jTimeStamp)
    {
        *send_cmd = JSON_NOITEM_ERROR;
        cJSON_Delete(jRoot);
        //json串错误
        return NULL;
    }

    int timestamp =  atoi(jTimeStamp->valuestring);
    //释放
    cJSON_Delete(jRoot);
    usleep(50000);
    //查询结果
    myPtf("timestamp=%d,curtimestamp=%d,curstatus=%d,threadFlowID=%d,curConsume.retstring=%s\n",
          timestamp,curConsume.flowID,curConsume.status,threadFlowID,curConsume.retstring);
    char strRet[1024];

    if (timestamp != curConsume.flowID)
    {
        //返回错误
        sprintf(strRet,"{\"timeStamp\":\"%d\",\"state\":3}",timestamp);
        //json串错误
    }
    else
    {
        //如果出现线程已休息，但是状态为未结束，应该返回错误？还是返回
        if ((threadFlowID==0)&&(curConsume.status!=CONSUME_FINISH))
        {
            myPtf("QQQQQ~~threadFlowID==0,curConsume.status!=CONSUME_FINISH,curConsume.flowID=%d,curConsume.retstring=%s\n\n",
              curConsume.flowID,curConsume.retstring);
            sprintf(curConsume.retstring,"{\"flowID\":\"%d\",\"ret\":%d}",curConsume.flowID,NO_CONSUME_REQ);
            ////////////////////////////////////////////////////////////////////
            //回调通知UI界面
            memset(CallBackRetString,0,sizeof(CallBackRetString));
            sprintf(CallBackRetString,"{\"timeStamp\":\"%d\",\"cmdType\":%d,\"state\":1,",curConsume.flowID,curConsume.cmdType);
            strcpy(CallBackRetString+strlen(CallBackRetString),curConsume.retstring+1);
            Cmd = CONSUME_CALLBACK_QUERET;
            sendCmd_afunix(cli_uicallback.clisockfd,cli_uicallback.clipath, cli_uicallback.serpath,&Cmd, CallBackRetString,strlen(CallBackRetString)+1);
            ////////////////////////////////////////////////////////////////////
            curConsume.status = CONSUME_FINISH;
            myPtf("QQQQQ~~~~~~~~~~~~~~~~~~\n");
        }
        //等待刷卡，可取消
        if (curConsume.status == CONSUME_WAITFORCARD ) //可以取消
        {
            sprintf(strRet,"{\"timeStamp\":\"%d\",\"state\":5}",timestamp);
        }
        //交易中，已提交云端，等待结果
        else if ((curConsume.status == CONSUME_HAVECARDNO)||(curConsume.status == CONSUME_WAITFORCLOUD))
        {
            //设置返回字符串、
            sprintf(strRet,"{\"timeStamp\":\"%d\",\"state\":4}",timestamp);

        }
        //从云端返回数据串，本次交易结束
        else if (curConsume.status == CONSUME_FINISH)
        {
            //strcpy(strRet,curConsume.retstring);
            sprintf(strRet,"{\"timeStamp\":\"%d\",\"state\":1,",timestamp);
            int len = strlen(strRet);
            strcpy(strRet+len,curConsume.retstring+1);
            myPtf("queconsumeret001 time = %d-----------------------------------------------------------------------------------------------------------------\n\n\n\n\n\n\n",getCurrentTime());
            myPtf("queconsumeret001 time = %d-----------------------------------------------------------------------------------------------------------------\n\n\n\n\n\n\n",getCurrentTime());
        }
        else //其它，失败
        {
            sprintf(strRet,"{\"timeStamp\":\"%d\",\"state\":2}",timestamp);
        }
    }
    //分配内存
    char * ret_buf = (char *)malloc(strlen(strRet)+1);
    memset(ret_buf,0,strlen(strRet)+1);
    memcpy(ret_buf,strRet,strlen(strRet));
    myPtf("dealcmd:recvbuf=%s\n",ret_buf);
    *ret_len = strlen(strRet)+1;
    *send_cmd=UI_QUE_CONSUMERET + 10000;
    //返回
    myPtf("ret_len=%d,ret_buf=%s\n",*ret_len,ret_buf);
    return ret_buf;
}
//取消本次消费
char * fn_dealCmd_CancelConsume(char *recv_buf,int recv_len,int * ret_len,int *send_cmd)
{
    char strRet[1024]="";
    //解析字符串
    //将收到的参数填入结构
    *ret_len=0;
    if (recv_buf==NULL)
    {
        myPtf("NORECVBUF_ERROR\n");
        *send_cmd = NORECVBUF_ERROR;
        return NULL;
    }
    //解析Json串
    cJSON * jRoot=cJSON_Parse((char *)recv_buf);
    if(!jRoot)
    {
        *send_cmd = JSON_ERROR;
        //json串错误
        return NULL;
    }
    cJSON * jTimeStamp=cJSON_GetObjectItem(jRoot,"timeStamp");
    if(!jTimeStamp)
    {
        *send_cmd = JSON_NOITEM_ERROR;
        cJSON_Delete(jRoot);
        //json串错误
        return NULL;
    }
    int timestamp =  atoi(jTimeStamp->valuestring);
    //释放
    cJSON_Delete(jRoot);
    myPtf("curConsume.transactionType 1111\n");
    //查询结果
    if ((timestamp != curConsume.flowID)&&(curConsume.flowID != 0))
    {
        //返回错误
        sprintf(strRet,"{\"timeStamp\":\"%d\",\"curtimeStamp\":\"%d\",\"state\":3}",timestamp,curConsume.flowID);
        //json串错误
        return NULL;
    }
    else
    {
        myPtf("curConsume.transactionType 2222\n");
        //等待刷卡，可取消
        if (curConsume.status == CONSUME_WAITFORCARD) //可以取消
        {
            //取消扫码
            //让扫码服务停止扫码
            myPtf("curConsume.transactionType = %d\n",curConsume.transactionType);
            if ((curConsume.transactionType==TRSACTTYPE_SCANQRCODE)||(curConsume.transactionType==TRSACTTYPE_READANDSCAN))
            {
                int Cmd = SCANQR_STOP_QRCODE_CMD;
                myPtf("stop get qr\n");
                sendCmd_afunix(cli_scanqr.clisockfd,cli_scanqr.clipath, cli_scanqr.serpath,&Cmd, NULL, 0);
            }
            //结构清空
            memset(&curConsume,0,sizeof(curConsume));
            //写状态
            curConsume.status = CONSUME_SLEEP;
            sprintf(strRet,"{\"timeStamp\":\"%d\",\"state\":1}",timestamp);
        }
        //交易中，已提交云端，等待结果
        else
        {
            myPtf("curConsume.transactionType 333\n");
            //无法取消，返回失败
            sprintf(strRet,"{\"timeStamp\":\"%d\",\"state\":0}",timestamp);

        }
    }
    myPtf("curConsume.transactionType 4444\n");
    //分配内存
    char * ret_buf = (char *)malloc(strlen(strRet)+1);
    memset(ret_buf,0,strlen(strRet)+1);
    memcpy(ret_buf,strRet,strlen(strRet));
    myPtf("dealcmd:recvbuf=%s\n",ret_buf);
    *ret_len = strlen(strRet)+1;
    *send_cmd=UI_CANCEL_CONSUME + 10000;
    //返回
    myPtf("ret_len=%d,ret_buf=%s\n",*ret_len,ret_buf);
    return ret_buf;
}
//消费流水总额查询
char * fn_dealCmd_GetFlowTatal(char *recv_buf,int recv_len,int * ret_len,int *send_cmd)
{
    myPtf("here 2\n");
    if (recv_buf==NULL)
    {
        myPtf("NORECVBUF_ERROR\n");
        *ret_len = 0;
        *send_cmd = NORECVBUF_ERROR;
        return NULL;
    }
    //组织发送串
    int cloudCmdLen=0;
    //myPtf("recvcmdjson:%s,jsonlen=%d\n",recv_buf,recv_len);
    char * strCloudCmd = addCmd(CLD_GETFLOWT_CMD,recv_buf,recv_len,&cloudCmdLen);
    //myPtf("andcmdjson:%s,jsonlen=%d\n",strCloudCmd,cloudCmdLen);
    if(strCloudCmd == NULL)
    {
        *send_cmd = JSON_ADDCMD_ERROR;
        myPtf("JSON_ADDCMD_ERROR\n");
        //json串错误
        return NULL;
    }
    //接收字符串
    int cloudRevLen=0;
    char * strCloudRecv = sendCmd_socket(send_cmd, strCloudCmd, cloudCmdLen, &cloudRevLen);
    free (strCloudCmd);
    if(strCloudRecv == NULL)
    {
        *send_cmd = CLOUD_SENDCMD_ERROR;
        return NULL;
    }
    *ret_len = cloudRevLen;
    *send_cmd=UI_GET_FLOWTOTAL + 10000;
    //返回
    myPtf("ret_len=%d,strCloudRecv=%s\n",*ret_len,strCloudRecv);
    return strCloudRecv;
}
char * fn_dealCmd_GetFlowInfo(char *recv_buf,int recv_len,int * ret_len,int *send_cmd)
{
    if (recv_buf==NULL)
    {
        myPtf("NORECVBUF_ERROR\n");
        *ret_len = 0;
        *send_cmd = NORECVBUF_ERROR;
        return NULL;
    }
    //组织发送串
    int cloudCmdLen=0;
    //myPtf("recvcmdjson:%s,jsonlen=%d\n",recv_buf,recv_len);
    char * strCloudCmd = addCmd(CLD_GETFLOWD_CMD,recv_buf,recv_len,&cloudCmdLen);
    //myPtf("andcmdjson:%s,jsonlen=%d\n",strCloudCmd,cloudCmdLen);
    if(strCloudCmd == NULL)
    {
        *send_cmd = JSON_ADDCMD_ERROR;
        //json串错误
        return NULL;
    }
    //接收字符串
    int cloudRevLen=0;
    char * strCloudRecv = sendCmd_socket(send_cmd, strCloudCmd, cloudCmdLen, &cloudRevLen);
    free (strCloudCmd);
    if(strCloudRecv == NULL)
    {
        *send_cmd = CLOUD_SENDCMD_ERROR;
        return NULL;
    }
    *ret_len = cloudRevLen;
    *send_cmd=UI_GET_FLOWINFO + 10000;
    //返回
    myPtf("ret_len=%d,strCloudRecv=%s\n",*ret_len,strCloudRecv);
    return strCloudRecv;
}
//获取个人账户信息申请
char * fn_dealCmd_GetAccountInfo(char *recv_buf,int recv_len,int * ret_len,int *send_cmd)
{
    //将收到的参数填入结构
    *ret_len=0;
    int Cmd=0;
    time_t timep;
    if (recv_buf == NULL)
    {
        myPtf("NORECVBUF_ERROR\n");
        *send_cmd = NORECVBUF_ERROR;
        return NULL;
    }
    if (curHeartbeat.ret < NEEDRETRY)
    {
        myPtf("CLOUD_SENDCMD_ERROR\n");
        *send_cmd = CLOUD_SENDCMD_ERROR;
        return NULL;
    }
    //解析Json串
    cJSON * jRoot=cJSON_Parse((char *)recv_buf);
    if(!jRoot)
    {
        *send_cmd = JSON_ERROR;
        //json串错误
        return NULL;
    }
    cJSON * jTimeStamp=cJSON_GetObjectItem(jRoot,"timeStamp");
    if(!jTimeStamp)
    {
        *send_cmd = JSON_NOITEM_ERROR;
        cJSON_Delete(jRoot);
        //json串错误
        return NULL;
    }
    cJSON * jTermCode=cJSON_GetObjectItem(jRoot,"termCode");
    if(!jTermCode)
    {
        *send_cmd = JSON_NOITEM_ERROR;
        cJSON_Delete(jRoot);
        //json串错误
        return NULL;
    }
    cJSON * jTransactionType=cJSON_GetObjectItem(jRoot,"transactionType");
    if(!jTransactionType)
    {
        *send_cmd = JSON_NOITEM_ERROR;
        cJSON_Delete(jRoot);
        //json串错误
        return NULL;
    }
    //termid和areaid都是整形
    cJSON * jTermID=cJSON_GetObjectItem(jRoot,"termID");
    if(!jTermID)
    {
        *send_cmd = JSON_NOITEM_ERROR;
        cJSON_Delete(jRoot);
        //json串错误
        return NULL;
    }
    cJSON * jAreaID=cJSON_GetObjectItem(jRoot,"areaID");
    if(!jAreaID)
    {
        *send_cmd = JSON_NOITEM_ERROR;
        cJSON_Delete(jRoot);
        //json串错误
        return NULL;
    }
    time (&timep);
    myPtf("%s:wait for thread over!!!\n",ctime(&timep));
    curConsume.flowID = atoi(jTimeStamp->valuestring);
    myPtf("let us waitting!\n");
    //等待上一笔线程结束
    while(threadFlowID != 0)
    {
        usleep(50000);
    }
    time (&timep);
    myPtf("%s:thread is over!!!\n",ctime(&timep));
    //结构清空
    //memset(&curConsume,0,sizeof(curConsume));
    fn_cleanConsumeStructButflowID();
    //TermCode为字符串类型
    curConsume.flowID = atoi(jTimeStamp->valuestring);
    curConsume.termID = jTermID->valueint;
    strcpy(curConsume.termCode,jTermCode->valuestring);
    curConsume.cmdType = UI_REQ_ACCOUNTINFO;
    curConsume.could_Cmd = CLD_GETACCOINFO_CMD;
    curConsume.transactionType = jTransactionType->valueint;
    ////////////////////////////////////////////////////////////////////
    //回调通知UI界面
    memset(CallBackRetString,0,sizeof(CallBackRetString));
    sprintf(CallBackRetString,"{\"timeStamp\":\"%d\",\"cmdType\":%d,\"state\":5}",curConsume.flowID,curConsume.cmdType);
    Cmd = CONSUME_CALLBACK_QUERET;
    sendCmd_afunix(cli_uicallback.clisockfd,cli_uicallback.clipath,cli_uicallback.serpath,&Cmd,CallBackRetString,strlen(CallBackRetString)+1);
    ////////////////////////////////////////////////////////////////////
    //写状态
    curConsume.status = CONSUME_WAITFORCARD;
    //释放
    cJSON_Delete(jRoot);
    //启动消费服务线程,并将结构传入
    int flowID = curConsume.flowID;
    threadFlowID = curConsume.flowID;
    /*
    pthread_t thread;
	int ret = pthread_create(&thread, NULL, ConsumeThread, &curConsume.flowID);
	if (ret < 0) {
		myPtf("create consume thread error! ret = %d\n",ret);
        *send_cmd = CREATE_CONSUMETHREAD_ERROR;
        //json串错误
        return NULL;
	}
	*/
	//组织返回串
	char strRet[1024];
	sprintf(strRet,"{\"timeStamp\":\"%d\",\"state\":1,\"result\":\"\"}",flowID);

    char * ret_buf = (char *)malloc(strlen(strRet)+1);
    memset(ret_buf,0,strlen(strRet)+1);
    memcpy(ret_buf,strRet,strlen(strRet));
    time (&timep);
    myPtf("%s:dealcmd:recvbuf=%s\n",ctime(&timep),ret_buf);
    *ret_len = strlen(strRet)+1;
    *send_cmd=UI_REQ_ACCOUNTINFO + 10000;
    //返回成功
    return ret_buf;
}

//读取二维码信息申请
char * fn_dealCmd_GetQrcode(char *recv_buf,int recv_len,int * ret_len,int *send_cmd)
{
    //将收到的参数填入结构
    *ret_len=0;
    int Cmd=0;
    time_t timep;
    if (recv_buf==NULL)
    {
        myPtf("NORECVBUF_ERROR\n");
        *send_cmd = NORECVBUF_ERROR;
        return NULL;
    }
    //解析Json串
    cJSON * jRoot=cJSON_Parse((char *)recv_buf);
    if(!jRoot)
    {
        *send_cmd = JSON_ERROR;
        //json串错误
        return NULL;
    }
    cJSON * jTimeStamp=cJSON_GetObjectItem(jRoot,"timeStamp");
    if(!jTimeStamp)
    {
        *send_cmd = JSON_NOITEM_ERROR;
        cJSON_Delete(jRoot);
        //json串错误
        return NULL;
    }
    myPtf("%s:wait for thread over!!!\n",ctime(&timep));
    curConsume.flowID = atoi(jTimeStamp->valuestring);
    int flowID = curConsume.flowID;
    myPtf("let us waitting flowid=%d!\n",flowID);
    //释放
    cJSON_Delete(jRoot);
    //等待上一笔线程结束
    while(threadFlowID != 0)
    {
        usleep(50000);
    }
    time (&timep);
    myPtf("%s:thread is over!!!\n",ctime(&timep));
    //结构清空
    //memset(&curConsume,0,sizeof(curConsume));
    fn_cleanConsumeStructButflowID();
    //写状态
    ////////////////////////////////////////////////////////////////////
    curConsume.status = CONSUME_WAITFORCARD;
    curConsume.cmdType =  UI_REQ_SCANQRCODE;
    //启动消费服务线程,并将结构传入
    curConsume.flowID = flowID;
    myPtf("create scanqr thread curConsume.flowID = %d,flowId=%d\n",curConsume.flowID,flowID);
    myPtf("curConsume.transactionType gggg\n");
    curConsume.transactionType = TRSACTTYPE_SCANQRANDRET;
    threadFlowID = curConsume.flowID;
    //回调通知UI界面
    memset(CallBackRetString,0,sizeof(CallBackRetString));
    sprintf(CallBackRetString,"{\"timeStamp\":\"%d\",\"cmdType\":%d,\"state\":5}",flowID,curConsume.cmdType);
    Cmd = CONSUME_CALLBACK_QUERET;
    sendCmd_afunix(cli_uicallback.clisockfd,cli_uicallback.clipath,cli_uicallback.serpath,&Cmd,CallBackRetString,strlen(CallBackRetString)+1);
    ////////////////////////////////////////////////////////////////////
    /*
    pthread_t thread;
	int ret = pthread_create(&thread, NULL, ScanQrThread, &curConsume.flowID);
	if (ret < 0) {
		myPtf("create scanqr thread error! ret = %d\n",ret);
        *send_cmd = CREATE_SCANQRTHREAD_ERROR;
        //json串错误
        return NULL;
	}*/
	//组织返回串
	char strRet[1024];
	sprintf(strRet,"{\"timeStamp\":\"%d\",\"state\":1,\"result\":\"\"}",flowID);

    char * ret_buf = (char *)malloc(strlen(strRet)+1);
    memset(ret_buf,0,strlen(strRet)+1);
    memcpy(ret_buf,strRet,strlen(strRet));
    time (&timep);
    myPtf("%s:dealcmd:recvbuf=%s\n",ctime(&timep),ret_buf);
    *ret_len = strlen(strRet)+1;
    *send_cmd=UI_REQ_SCANQRCODE + 10000;
    //返回成功
    return ret_buf;
}

//获取版本号
char * fn_dealCmd_GetVersion(char *recv_buf,int recv_len,int * ret_len,int *send_cmd)
{
    time_t timep;
    ///////////////////////////////////////
    //解析TermCode
    *ret_len=0;
    if (recv_buf==NULL)
    {
        myPtf("NORECVBUF_ERROR\n");
        *send_cmd = NORECVBUF_ERROR;
        return NULL;
    }
    //识别当前终端唯一识别号
    //解析Json串
    cJSON * jRoot=cJSON_Parse((char *)recv_buf);
    if(!jRoot)
    {
        *send_cmd = JSON_ERROR;
        //json串错误
        return NULL;
    }
    cJSON * jTermCode=cJSON_GetObjectItem(jRoot,"termCode");
    if(!jTermCode)
    {
        *send_cmd = JSON_NOITEM_ERROR;
        cJSON_Delete(jRoot);
        //json串错误
        return NULL;
    }
    //TermCode为字符串类型
    memset(curHeartbeat.termCode,0,sizeof(curHeartbeat.termCode));
    strcpy(curHeartbeat.termCode,jTermCode->valuestring);
    cJSON_Delete(jRoot);
    ///////////////////////////////////////
    char strTemp[128];
    time (&timep);
    sprintf(strTemp,"{\"consumeser_version\":\"%s\",\"ret\":%d}",CONSUMESERVERSION,curHeartbeat.ret);
    myPtf("get new curHeartbeat:%s,tps=%s\n",strTemp,ctime(&timep));
    char * strRet = malloc(strlen(strTemp)+1);
    memset(strRet,0,strlen(strTemp)+1);
    memcpy(strRet,strTemp,strlen(strTemp));
    *ret_len = strlen(strTemp)+1;
    *send_cmd=UI_GET_VERSION + 10000;
    //返回
    myPtf("ret_len=%d,strRet=%s\n",*ret_len,strRet);
    return strRet;
}

//获取版本号
char * fn_dealCmd_GetScanQrSerVersion(char *recv_buf,int recv_len,int * ret_len,int *send_cmd)
{
    char strTemp[128];

    //获取ScanQrSerVersion;
    int Cmd;
    //获取scanqrser版本号
    Cmd = SCANQR_GET_VERSION_CMD;
    memset(ScanQrSerVersion,0,sizeof(ScanQrSerVersion));
    if (cli_scanqr.clisockfd != 0)
    {
        char * retVersion = sendCmd_afunix(cli_scanqr.clisockfd,cli_scanqr.clipath, cli_scanqr.serpath,&Cmd, NULL, 0);
        memcpy(ScanQrSerVersion,retVersion,strlen(retVersion));
        free(retVersion);
    }
    else
    {
        sprintf(ScanQrSerVersion,"0000");
    }
    myPtf("ScanQrSerVersion ret:%s\n",ScanQrSerVersion);

    //sprintf(strTemp,"{\"scanqrser_version\":\"%s\"}",ScanQrSerVersion);
    strcpy(strTemp,ScanQrSerVersion);
    //json串错误
    char * strRet = malloc(strlen(strTemp)+1);
    memset(strRet,0,strlen(strTemp)+1);
    memcpy(strRet,strTemp,strlen(strTemp));
    *ret_len = strlen(strTemp)+1;
    *send_cmd=UI_GET_SCANQRVERSION + 10000;
    //返回
    myPtf("ret_len=%d,strRet=%s\n",*ret_len,strRet);
    return strRet;
}

//初始化消费服务，包括初始化网络、初始化两个AF_UNIX链接客户端、同步时间、获取各程序版本号等
//
int fn_dealCmd_init()
{
    //初始化 配置文件、网络等公共资源
    //初始化

  	int ret;
    //读取当前目录
    /*
    char szConfigPath[MAX_PATH];
	memset(szConfigPath,0,sizeof(szConfigPath));
	fn_GetCurrentPath(szConfigPath,CONF_FILE_PATH);
    */
    //读取脱机流水文件
	fn_InitialRecords();
    //读取配置信息
    /////////////////////////////////////
    //识别event
    if(fn_StartQRScanThread()<0)
    {
        myPtf("init QRScanThread error!\n");
    }
    /////////////////////////////////////
    char ServerNode[20]="server";
    char SerNodeName[25]="";
    char strTemp[32]="";
    ////////////////////////////////////////////////////////////////////
    ser_conn_t server_conn[MAX_SERVER_NUM];
    int InQr = -1;
    int i;
    myPtf("init connet %s\n",CONFIG_PATH);
    for(i=0;i<MAX_SERVER_NUM;i++)
    {
        sprintf(strTemp, "%d", i+1);
        strcpy(SerNodeName,ServerNode);
        strcat(SerNodeName,strTemp);
        server_conn[i].SerPort = fn_GetIniKeyInt(SerNodeName,"port",CONFIG_PATH);
        strcpy(server_conn[i].SerIP,fn_GetIniKeyString(SerNodeName,"ipaddress",CONFIG_PATH));
        myPtf("IP=%s Port=%d Version=%s\n",server_conn[i].SerIP,server_conn[i].SerPort,CONSUMESERVERSION);
    }
    InQr = fn_GetIniKeyInt("qrcodeser","InQr",CONFIG_PATH);
    myPtf("read InQr = %d\n",InQr);
    /*
    ser_conn_t server_conn[MAX_SERVER_NUM];
    server_conn[0].SerPort = 9093;
    strcpy(server_conn[0].SerIP,"10.1.70.13");
    server_conn[1].SerPort = 9094;
    strcpy(server_conn[1].SerIP,"10.1.70.13");
    */
    //初始化网络连接
    //链接服务器
    if(init_client_socket(server_conn)<0)
    {
        myPtf("init connect error\n");
        return -2;  //网络连接失败
    }
    myPtf("init connect OK!\n");
    //////////////////////////////////////////////////////////////////
    //启动本地服务
    int Cmd;
    //初始化回调接口连接
    strcpy(cli_uicallback.clipath,CONSUMECALLBACK_PATH_C);
    strcpy(cli_uicallback.serpath,CONSUMECALLBACK_PATH_S);
    cli_uicallback.clisockfd = conn_afunix(cli_uicallback.clipath);
    myPtf("cli_uicallback.clisockfd=%d----------------------------------clipath=%s\n",cli_uicallback.clisockfd,CONSUMECALLBACK_PATH_C);
    //初始化摄像头AF_UNIX链接
    strcpy(cli_scanqr.clipath,SCANQR_PATH_C);
    strcpy(cli_scanqr.serpath,SCANQR_PATH_S);

    if (InQr > 0)
    {
        cli_scanqr.clisockfd = conn_afunix(cli_scanqr.clipath);
    }
    else
    {
        cli_scanqr.clisockfd = 0;
    }
    //cli_scanqr.clisockfd = conn_afunix(cli_scanqr.clipath);

    myPtf("cli_scanqr.clisockfd=%d----------------------------------clipath=%s\n",cli_scanqr.clisockfd,SCANQR_PATH_C);
    //启动扫码
    Cmd = SCANQR_BEGIN_CAPTURE_CMD;
    char * strQr=NULL;
    sendCmd_afunix(cli_scanqr.clisockfd,cli_scanqr.clipath, cli_scanqr.serpath,&Cmd, NULL, 0);
    myPtf("begin capture! retCmd = %d\n",Cmd);
    //打开串口
    //串口参数设置
    com_param comparam;
    comparam.baud=9600;
    comparam.databits=8;
    comparam.parity='n';
    comparam.stopbits=1;
    //初始化串口
    serialfd=serial_init(SERIAL_DEVICE,&comparam);
    if (serialfd<0)
    {
        myPtf("serial_init error,code is %d \n",serialfd);
    }
    //启动心跳线程
    pthread_t heartbeat_thread;
	ret = pthread_create(&heartbeat_thread, NULL, HeartBeatThread, NULL);
	if (ret < 0) {
		myPtf("create HeartBeat thread error! ret = %d\n",ret);
	}
    pthread_t consume_thread;
	ret = pthread_create(&consume_thread, NULL, ConsumeThread, &curConsume.flowID);
	if (ret < 0) {
		myPtf("create consume thread error! ret = %d\n",ret);
	}

    return 0;
}

int fn_dealCmd_destroy()
{
    //退出心跳线程
    stopbeat = 1;
    //关闭串口
    serial_exit(serialfd);
    //停止外接扫码
    fn_EndQRScanThread();
    //停止扫码
    int Cmd = SCANQR_STOP_CAPTURE_CMD;
    char * strQr=NULL;
    sendCmd_afunix(cli_scanqr.clisockfd,cli_scanqr.clipath, cli_scanqr.serpath,&Cmd, NULL, 0);
    myPtf("stop capture! retCmd = %d\n",Cmd);
    //关闭扫码
    close_afunix(cli_scanqr.clisockfd,cli_scanqr.clipath);
    //关闭回调
    close_afunix(cli_uicallback.clisockfd,cli_uicallback.clipath);
    //关闭云连接
    close_client_socket();
    return 0;
}
