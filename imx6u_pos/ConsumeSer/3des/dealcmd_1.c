#include "a.h"
#include "cJSON.h"
#include "common.h"
#include "dealcmd.h"
#include "sendcmd.h"
#include "serial.h"
#include "sendcmd_cloud.h"
#include <time.h>
#include <pthread.h>

#define MAX_PATH 260
#define CONF_FILE_PATH	"Config.ini"

const char SERIAL_READCARD_CMD[]={0x02,0xE0,0x01,0xA6,0x89,0xCC};   //串口读卡命令字

static char FCODE[]="12457801";
static char QRdeskey[24] = "YXP,sEY@.X&yrt@lZnk.2@O0";//秘钥
static DES_cblock QRivsetup = "R*AK@Z&w";

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
    char pwd[8];            //卡片消费密码
    char consumeNum[128];   //二维码（支付宝或微信）
    char retstring[512];    //消费结果，当status=3时有效。服务端返回的字符串
}consume_struct_t;

static consume_struct_t curConsume;

static char TermCode[50];           //终端唯一识别序号
static int TermID = 0;
static int AreaID = 0;

static int threadFlowID = 0;
//static char ConsumeSerVersion[50];      //版本号

static client_unsocket_t cli_scanqr;        //扫码使用结构
static client_unsocket_t cli_secscreen;     //副屏显示使用结构
static int serialfd;                        //串口使用结构

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

    char strCmd[20];
    memset(strCmd,0,20);
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
//返回0表示成功，此时返回的accountId有效，否则accountId=0
//返回-1表示二维码解码失败
//返回-2表示超时失败
//返回-3表示厂商号不一致
int fn_DecryptForBOC(char * ciphertext,int * accountId)
{

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
    if (strlen(outbuff)!=48)
    {
        free(outbuff);
        myPtf("error 2!\n");
        return QR_DECODE_ERROR;
    }
    //初始化参数
    char factoryCode[9];
    memset(factoryCode,0,9);
    char accId[25];
    memset(accId,0,25);
    char consumetime[17];
    memset(consumetime,0,17);
    //取出参数
    memcpy(factoryCode,outbuff,8);
    memcpy(accId,outbuff+8,24);
    memcpy(consumetime,outbuff+32,14);
    free(outbuff);
    //判断时间格式有效性
    for(int i=0;i<14;i++)
    {
        if((consumetime[i]<0x30)||(consumetime[i]>0x39))
        {
            myPtf("time format error!\n");
            return QR_DECODE_ERROR;
        }
    }
    //比较厂商编号
    if (strcmp(factoryCode,FCODE)!=0)
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
    if((diff>300)||(diff<-300))
    {
        return QR_DECODE_TIMEOUT;
    }
    //返回账号
    *accountId = atoi(accId);
    return 0;
}

//消费服务线程
static void * ConsumeThread(void *arg)
{
    //int flowID = *(int *)arg;
    int flowID = curConsume.flowID;
    threadFlowID = flowID;
    myPtf("start ConsumeThread threadFlowID=%d\n",threadFlowID);
    int Cmd;
    int ret;
    char * strQr = NULL;
    char recvbuf[64]; //用于读卡命令返回
    int recvlen;//需要接收19个字符
    int recvleft;
    int recvedlen;
    int LoopTimes;
    curConsume.fType = -1;

    char strCloudConsume[512];//用于向云端发送数据，因为存在反复发送，所以固定内存数组。考虑以后二维码长度最大128，总共设计512字节
    int istrlen;
    int send_cmd;
    char * strRecvConsume;
    //当流水被取消，或者已刷卡，返回
    while(curConsume.flowID == flowID)
    {
        myPtf("0curConsume.flowid=%d,flowID=%d,threadFlowID=%d",curConsume.flowID,flowID,threadFlowID);
        int ret = 0;
        if (curConsume.status == CONSUME_FINISH)
        {
            break;
        }
        //等待刷卡或扫码
        if(curConsume.status == CONSUME_WAITFORCARD)
        {
            myPtf("curConsume.status==CONSUME_WAITFORCARD,start read qr or card\n");
            //读二维码
            Cmd = SCANQR_GET_QRCODE_CMD;
            myPtf("get qr\n");
            strQr = sendCmd_afunix(cli_scanqr.clisockfd,cli_scanqr.clipath, cli_scanqr.serpath,&Cmd, NULL, 0);
            if (strQr!=NULL)
            {
                //读到二维码
                myPtf("get qr:%s\n",strQr);
                /////////////////////////////////////////////
                //支付宝扫码
                //strcpy(curConsume.consumeNum,strQr);
                //curConsume.fType = 3;
                /////////////////////////////////////////////
                //解码，中银特殊要求，解码后为账号
                ret = fn_DecryptForBOC(strQr,&curConsume.accountID);
                myPtf("decrypt accountid = %d\n",curConsume.accountID);
                //解码成功，进入下一阶段
                if (ret == 0)
                {
                    curConsume.fType = 2;
                    curConsume.status = CONSUME_HAVECARDNO;
                }
                else
                {
                    sprintf(curConsume.retstring,"{\"timeStamp\":\"%d\",\"scanret\":%d}",flowID,ret);
                    curConsume.fType = 2;
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
            myPtf("read qr fault,curConsume.fType=%d\n",curConsume.fType);
            //扫码未成功，改为读卡
            if(curConsume.status == CONSUME_WAITFORCARD)
            {
                //检查是否读到卡
                ret=serial_send(serialfd,SERIAL_READCARD_CMD,6);
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
                    ret=serial_recv(serialfd,recvbuf+(recvedlen),recvleft,100);
                    recvedlen+=ret;
                    recvleft-=ret;
                    if ((LoopTimes++)>5)
                        break;
                }
                myPtf("serial_revlen:%d \nrecvbuf:",recvedlen);
                for(int i=0;i<recvedlen;i++)
                {
                    myPtf("%02X",recvbuf[i]);
                }
                myPtf("\n");
                //解析卡号
                if((recvbuf[0]==0x05)&&(recvbuf[1]==0xE0)&&(recvbuf[2]==0x07)&&(recvbuf[3]==0x51)
                   &&(recvbuf[8]==0x90)&&(recvbuf[9]==0x00))
                {
                    curConsume.cardID=(recvbuf[4]<<24)+(recvbuf[5]<<16)+(recvbuf[6]<<8)+recvbuf[7];
                    curConsume.fType = 0;
                    curConsume.status = 2;
                }
            }
            //读到卡号或二维码
            if(curConsume.status == CONSUME_HAVECARDNO)
            {
                //置状态标记
                myPtf("read ok,curConsume.fType=%d,curConsume.status=CONSUME_HAVECARDNO\n",curConsume.fType);
                //组织JSON串
                istrlen=0;
                memset(strCloudConsume,0,sizeof(strCloudConsume));
                //字符串这里不要换行写，不然会占用更大空间，甚至导致strCloudConsume越界.
                sprintf(strCloudConsume,"{\"areaID\":%d,\"cardID\":%d,\"cmd\":3001,\"consumeNum\":\"%s\",\"contypeID\":%d,\"fType\":%d,\"flowID\":%d,\"flowMoney\":%d,\"pwd\":\"%s\",\"termCode\":\"%s\",\"termID\":%d,\"accountId\":%d}",
                                          AreaID,curConsume.cardID,curConsume.consumeNum,
                                          curConsume.contypeID,curConsume.fType,flowID,curConsume.flowMoney,
                                          curConsume.pwd,TermCode,TermID,curConsume.accountID);
                myPtf("1consume json:%s\n",strCloudConsume);
                //向云端发送请求
                strRecvConsume = sendCmd_socket(&send_cmd, strCloudConsume, strlen(strCloudConsume), &istrlen);
                myPtf("ret!!!\n");
                //myPtf("1recv from cloud ,len=%d,data=%s",strRecvConsume,strRecvConsume);
                myPtf("1recv from cloud ,data=%s",strRecvConsume);
                //考虑调用失败问题，服务器未收到该流水
                curConsume.status=CONSUME_WAITFORCLOUD;
                //如果未返回结果，继续调后续查询接口
                if(strRecvConsume==NULL)
                {
                    continue;
                }
                //如果返回结果，填写结果
                memcpy(curConsume.retstring,strRecvConsume,istrlen);
                free(strRecvConsume);
                strRecvConsume = NULL;
                curConsume.status=CONSUME_FINISH;
                //退出本线程
                continue;

            }
            else
            //未读到卡号或二维码，此时curConsume.status任然为1，继续续等待刷卡或扫码
            {
                myPtf("not read cardno or qr,curConsume.status:%d\n",curConsume.status);
                continue;
            }
        }
        //发送云端查询请求
        if(curConsume.status==CONSUME_WAITFORCLOUD)
        {
            myPtf("send to cloud,curConsume.status==%d\n",curConsume.status);
            //发送云端查询请求
            istrlen=0;
            memset(strCloudConsume,0,sizeof(strCloudConsume));
            sprintf(strCloudConsume,"{\"cmd\":%d,\"flowID\":%d,\"termCode\":\"%s\",\"termID\":%d}",CLD_GETCONSUMERET_CMD,flowID,TermCode,TermID);
            myPtf("2consume json:%s\n",strCloudConsume);
            //向云端发送请求
            strRecvConsume = sendCmd_socket(&send_cmd, strCloudConsume, strlen(strCloudConsume), &istrlen);
            myPtf("ret2222222222222\n");
            myPtf("2recv from cloud ,len=%d,data=%s",istrlen,strRecvConsume);
            //如果未返回结果，继续调后续查询接口
            if(strRecvConsume == NULL)
            {
                myPtf("1curConsume.flowid=%d,flowID=%d,threadFlowID=%d",curConsume.flowID,flowID,threadFlowID);
                continue;
            }
            //如果返回结果，填写结果
            memcpy(curConsume.retstring,strRecvConsume,istrlen);
            free(strRecvConsume);
            strRecvConsume = NULL;
            curConsume.status=CONSUME_FINISH;
        }
    }
    threadFlowID = 0;
    myPtf("end ConsumeThread threadFlowID=%d\n",threadFlowID);
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
//获取终端参数
char * fn_dealCmd_GetTermInfo(char *recv_buf,int recv_len,int * ret_len,int *send_cmd)
{
    //////////////////////////////////////////////////////
    //获取时间JSON串
    cJSON * jRoot;

    char strGetTime[20]="{\"cmd\":3008}";
    int iTimelen=0;
    myPtf("gettime:%s\n",strGetTime);
    char * strRecvTime = sendCmd_socket(send_cmd, strGetTime, strlen(strGetTime), &iTimelen);
    if (strRecvTime==NULL)
    {
        *send_cmd = CLOUD_SENDCMD_ERROR;
        return NULL;
    }
    //解析Json串
    jRoot=cJSON_Parse((char *)strRecvTime);
    if(!jRoot)
    {
        *send_cmd = JSON_ERROR;
        free(strRecvTime);
        //json串错误
        return NULL;
    }
    cJSON * jTime=cJSON_GetObjectItem(jRoot,"nowTime");
    if(!jTime)
    {
        *send_cmd = JSON_NOITEM_ERROR;
        cJSON_Delete(jRoot);
        free(strRecvTime);
        //json串错误
        return NULL;
    }
    //解析时间
    char strTime[20] = "";
    strcpy(strTime,jTime->valuestring);
    cJSON_Delete(jRoot);
    free(strRecvTime);
    //修改系统时间
    myPtf("server time is %s\n",strTime);
    time_t sertime = FormatTime(strTime);
    int ret = stime(&sertime);  //修改时间需要root权限
    myPtf("modify time ret = %d\n",ret);

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
    //TermCode为字符串类型
    memset(TermCode,0,sizeof(TermCode));
    strcpy(TermCode,jTermCode->valuestring);
    myPtf("TermCode=%s,jTermCode=%s,len=%d,size=%d\n",TermCode,jTermCode->valuestring,strlen(TermCode),sizeof(TermCode));
    cJSON_Delete(jRoot);

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
            TermID = jTermID->valueint;
        }
        cJSON * jAreaID=cJSON_GetObjectItem(jTerm,"areaID");
        if(jAreaID)
        {
            AreaID = jAreaID->valueint;
        }
    }
    cJSON_Delete(jRoot);

    *ret_len = strlen(strCloudRecv)+1;  //OK
    *send_cmd=UI_GET_TERMINFO + 10000;
    //返回
    myPtf("ret_len=%d,ret_buf=%s\n",*ret_len,strCloudRecv);
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

//请求消费
char * fn_dealCmd_ReqConsume(char *recv_buf,int recv_len,int * ret_len,int *send_cmd)
{
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
    myPtf("wait for thread over!!!\n");
    curConsume.flowID = atoi(jTimeStamp->valuestring);
    myPtf("let us waitting!\n");
    //等待上一笔线程结束
    while(threadFlowID != 0)
    {
        usleep(10000);
    }
    myPtf("thread is over!!!\n");
    //结构清空
    memset(&curConsume,0,sizeof(curConsume));
    //TermCode为字符串类型
    curConsume.flowMoney = atoi(jMoney->valuestring);
    curConsume.flowID = atoi(jTimeStamp->valuestring);
    curConsume.contypeID = atoi(jType->valuestring);
    memset(curConsume.pwd,0,sizeof(curConsume.pwd));
    strcpy(curConsume.pwd,jPwd->valuestring);
    //写状态
    curConsume.status = CONSUME_WAITFORCARD;
    //释放
    cJSON_Delete(jRoot);
    //启动消费服务线程,并将结构传入
    int flowID = curConsume.flowID;
    pthread_t thread;
	int ret = pthread_create(&thread, NULL, ConsumeThread, NULL);
	if (ret < 0) {
		myPtf("create consume thread error! ret = %d\n",ret);
        *send_cmd = CREATE_CONSUMETHREAD_ERROR;
        //json串错误
        return NULL;
	}
	//组织返回串
	char strRet[1024];
	sprintf(strRet,"{\"timeStamp\":\"%d\",\"state\":1,\"result\":\"\"}",flowID);

    char * ret_buf = (char *)malloc(strlen(strRet)+1);
    memset(ret_buf,0,strlen(strRet)+1);
    memcpy(ret_buf,strRet,strlen(strRet));
    myPtf("dealcmd:recvbuf=%s\n",ret_buf);
    *ret_len = strlen(strRet)+1;
    *send_cmd=UI_REQ_CONSUME + 10000;
    //返回成功
    return ret_buf;
}
//查询消费结果
char * fn_dealCmd_QueConsumeRet(char *recv_buf,int recv_len,int * ret_len,int *send_cmd)
{
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
    //查询结果
    myPtf("timestamp=%d,curtimestamp=%d,curstatus=%d\n",timestamp,curConsume.flowID,curConsume.status);
    char strRet[1024];
    if (timestamp != curConsume.flowID)
    {
        //返回错误
        sprintf(strRet,"{\"timeStamp\":\"%d\",\"state\":3}",timestamp);//流水已取消
        //json串错误
    }
    else
    {
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
    int timestamp =  jTimeStamp->valueint;
    //释放
    cJSON_Delete(jRoot);
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
        //等待刷卡，可取消
        if (curConsume.status == CONSUME_WAITFORCARD) //可以取消
        {
            //结构清空
            memset(&curConsume,0,sizeof(curConsume));
            //写状态
            curConsume.status = CONSUME_SLEEP;
            sprintf(strRet,"{\"timeStamp\":\"%d\",\"state\":1}",timestamp);
        }
        //交易中，已提交云端，等待结果
        else
        {
            //无法取消，返回失败
            sprintf(strRet,"{\"timeStamp\":\"%d\",\"state\":0}",timestamp);

        }
    }

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
    myPtf("here 3\n");
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
    myPtf("here 4\n");
    char * strCloudRecv = sendCmd_socket(send_cmd, strCloudCmd, cloudCmdLen, &cloudRevLen);
    myPtf("here 5\n");
    free (strCloudCmd);
    myPtf("here 6\n");
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
//初始化消费服务，包括初始化网络、初始化两个AF_UNIX链接客户端、同步时间、获取各程序版本号等
//
int fn_dealCmd_init()
{
    //初始化 配置文件、网络等公共资源
    //初始化
    /*
  	int ret;
    //读取当前目录
    char szConfigPath[MAX_PATH];
	memset(szConfigPath,0,sizeof(szConfigPath));
	fn_GetCurrentPath(szConfigPath,CONF_FILE_PATH);
    //读取配置信息

    char ServerNode[20]="Server";
    char SerNodeName[25]="";
    char strTemp[32]="";
    ////////////////////////////////////////////////////////////////////
    ser_conn_t server_conn[MAX_SERVER_NUM];
    int i;
    myPtf("init connet\n");
    for(i=0;i<MAX_SERVER_NUM;i++)
    {
        sprintf(strTemp, "%d", i+1);
        strcpy(SerNodeName,ServerNode);
        strcat(SerNodeName,strTemp);
        server_conn[i].SerPort = fn_GetIniKeyInt(SerNodeName,"port",szConfigPath);
        strcpy(server_conn[i].SerIP,fn_GetIniKeyString(SerNodeName,"ip",szConfigPath));
        myPtf("IP=%s Port=%d Version=%s\n",server_conn[i].SerIP,server_conn[i].SerPort,CONSUMESERVERSION);
    }
    */
    ser_conn_t server_conn[MAX_SERVER_NUM];
    server_conn[0].SerPort = 9093;
    strcpy(server_conn[0].SerIP,"10.1.70.13");
    server_conn[1].SerPort = 9094;
    strcpy(server_conn[1].SerIP,"10.1.70.13");
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
    //初始化摄像头AF_UNIX链接
    strcpy(cli_scanqr.clipath,SCANQR_PATH_C);
    strcpy(cli_scanqr.serpath,SCANQR_PATH_S);
    cli_scanqr.clisockfd = conn_afunix(cli_scanqr.clipath);
    myPtf("cli_scanqr.clisockfd=%d----------------------------------clipath=%s\n",cli_scanqr.clisockfd,SCANQR_PATH_C);
    //启动扫码
    Cmd = SCANQR_BEGIN_CAPTURE_CMD;
    char * strQr=NULL;
    sendCmd_afunix(cli_scanqr.clisockfd,cli_scanqr.clipath, cli_scanqr.serpath,&Cmd, NULL, 0);
    myPtf("begin capture! retCmd = %d\n",Cmd);
    //初始化副屏AF_UNIX连接
    //副屏显示结构
    strcpy(cli_secscreen.clipath,SECSCREEN_PATH_C);
    strcpy(cli_secscreen.serpath,SECSCREEN_PATH_S);
    cli_secscreen.clisockfd = conn_afunix(cli_secscreen.clipath);
    myPtf("cli_secscreen.clisockfd=%d--------------------------------clipath=%s\n",cli_secscreen.clisockfd,SECSCREEN_PATH_C);
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
    return 0;
}

int fn_dealCmd_destroy()
{
    //关闭串口
    serial_exit(serialfd);
    //关闭扫码
    close_afunix(cli_scanqr.clisockfd,cli_scanqr.clipath);
    //关闭副屏

    //关闭云连接
    close_client_socket();
    return 0;
}
