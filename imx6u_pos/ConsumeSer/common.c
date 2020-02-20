/*
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
*/
#include "a.h"
#include "common.h"

/*
void myPtf(const char* format, ...)
{
    va_list ap;
    int n;
    va_start(ap, format);
    n = vprintf(format, ap);
    va_end(ap);
    return;
}*/

static char *CASTFILE_CMD = "cat /proc/bus/input/devices > /usr/local/nkty/temp/deviceslog.txt" ;
static char *DEVICE_LIST_FILE = "/usr/local/nkty/temp/deviceslog.txt";

//脱机流水存储文件
static char *OFFLINE_FLOWDATA_FILE = "/usr/local/nkty/offline_flow.dat";
static char *OFFLINE_FLOWBAK_FILE = "/usr/local/nkty/offline_bak.dat";

static char *ACCOLIST_FILE = "/usr/local/nkty/account_list.dat";
static char *CONFIG_FILE = "/usr/local/nkty/offline_config.dat";
//脱机流水数据缓存
static char *Offline_FlowBuff = NULL;
//脱机流水数量
static int  Offline_FlowNum = 0;


//写入配置文件
int fn_WriteConfig(char * config,int len)
{
    FILE * file;
    //打开文件(ab)追加二进制文件
    file = fopen(CONFIG_FILE,"wb");
    if(file != NULL)
    {
        setvbuf(file,NULL,_IONBF,0);
        //写流水到磁盘
        fwrite(config,1,len,file);
        fclose(file);
        return 0;
    }
    else
        return -1;
}
//读取配置文件
int fn_ReadConfig(char * config,int len)
{
    FILE * file;
    myPtf("fn_ReadConfig:len = %d\n",len);
    //打开文件(ab)追加二进制文件
    file = fopen(CONFIG_FILE,"rb");
    if(file != NULL)
    {
        myPtf("fn_ReadConfig:open OK!\n");
        fseek(file,0L,SEEK_SET);
        fread(config,1,len,file);
        myPtf("fn_ReadConfig:read UseTermType=%d!\n",((config_t *)config)->UseTermType);
        fclose(file);
        return 0;
    }
    else
        return -1;
}
//写数据包
//pData表示写入数据
//PackageNum=1 表示第一包，需要覆盖原来文件，其余表示追加文件
//dataLen 表示数据长度
int fn_WriteAccoPack(char * pData,int PackageNum,int dataLen)
{
    FILE * file;
    if(PackageNum == 1)
    {
        //打开文件(wb)覆盖写二进制文件
        file = fopen(ACCOLIST_FILE,"wb");
    }
    else
    {
        //打开文件(ab)追加二进制文件
        file = fopen(ACCOLIST_FILE,"ab");
    }
    if(file != NULL)
    {
        setvbuf(file,NULL,_IONBF,0);
        //写流水到磁盘
        fwrite(pData,1,dataLen,file);
        fclose(file);
        return 0;
    }
    else
        return -1;
}

//在人员名单中查找账号
int fn_FindKeyByAccountId(char * pUsers,int Num,int accountId)
{
    myPtf("fn_FindKeyByAccountId:Num = %d,accountId = %d,pUsers=%d \n",Num,accountId,(int)pUsers);
    userlist_t *user=(userlist_t *)pUsers;
    //int Address = int(pUsers);
    for (int i=0;i<Num;i++)
    {
        myPtf("fn_FindKeyByAccountId:accountId = %d,%d\n",(int)user,user->accountId);
        if(user->accountId == accountId)
            return 1;   //找到返回1
        else
            user++;
    }
    //找不到返回0
    return 0;
}

//在人员名单中查找卡号
int fn_FindKeyByCardId(char * pUsers,int Num,int cardId)
{
    myPtf("fn_FindKeyByCardId:Num = %d,cardId = %d,pUsers=%d \n",Num,cardId,(int)pUsers);
    userlist_t *user=(userlist_t *)pUsers;
    //int Address = int(pUsers);
    for (int i=0;i<Num;i++)
    {
        if(user->cardNo == cardId)
        {
            myPtf("fn_FindKeyByCardId: find ok! cardId = %d,%d\n",(int)user,user->cardNo);
            return 1;   //找到返回1
        }
        else
            user++;
    }
    //找不到返回0
    return 0;
}


//读数据包，返回读到的数据内容，pAccNum表示数据条数，指针方式返回。
char * fn_ReadAccoPack(int * pAccNum)
{
    *pAccNum = 0;
    char * pData = NULL;
    //查看是否有文件存在
    if(access(ACCOLIST_FILE, F_OK) != 0)
    {
        myPtf("have not accolist_File\n");
        return pData;
    }
    //打开文件
    FILE * file = fopen(ACCOLIST_FILE,"rb");
    if(file == NULL)
    {
        myPtf("can not open accolist_File\n");
        return pData;
    }
    //获取文件大小
    fseek(file,0L,SEEK_END);
    int datalen = ftell(file);
    if(datalen > 0)
    {
        //读取文件到缓存中
        fseek(file,0L,SEEK_SET);
        pData = (char *)malloc(datalen);
        fread(pData,1,datalen,file);
        *pAccNum = datalen/sizeof(userlist_t);
    }
    myPtf("get accolist_File datalen=%d,size=%d,acconum=%d,pData=%d\n",datalen,sizeof(userlist_t),*pAccNum,(int)pData);
    //关闭文件
    fclose(file);
    return pData;
}



int fn_GetOffline_FlowNum()
{
    return Offline_FlowNum;
}


//初始化脱机流水,装入Offline_FlowBuff，条数写入 Offline_FlowNum
int fn_InitialRecords()
{
    Offline_FlowNum = 0;
    if (Offline_FlowBuff != NULL)
    {
        free(Offline_FlowBuff);
    }
    Offline_FlowBuff = NULL;
    myPtf("fn_InitialRecords start\n");
    //查看是否有文件存在
    if(access(OFFLINE_FLOWDATA_FILE, F_OK) != 0)
    {
        myPtf("have not offline_file\n");
        return Offline_FlowNum;
    }
    //打开文件
    FILE * file = fopen(OFFLINE_FLOWDATA_FILE,"rb");
    if(file == NULL)
    {
        myPtf("can not open offline_file\n");
        //有文件但无法打开，返回-1
        return -1;
    }
    //获取文件大小
    fseek(file,0L,SEEK_END);
    int datalen = ftell(file);
    if(datalen > 0)
    {
        //读取文件到缓存中
        fseek(file,0L,SEEK_SET);
        Offline_FlowBuff = (char *)malloc(datalen);
        fread(Offline_FlowBuff,1,datalen,file);
        Offline_FlowNum = datalen/sizeof(offline_flow_t);
    }
    myPtf("get offline_file datalen=%d,size=%d,flownum=%d\n",datalen,sizeof(offline_flow_t),Offline_FlowNum);
    //关闭文件
    fclose(file);
    return Offline_FlowNum;
}
//记录一条流水在最后，包括记录到内存中以及文件中，调用fn_AddRecordToFile
//成功返回0，失败返回-1；
int fn_AddRecord(char * data)
{
    //扩大内存
    char * newbuff;
    newbuff = (char *)realloc(Offline_FlowBuff,sizeof(offline_flow_t)*(Offline_FlowNum+1));
    if (newbuff == NULL)
    {
        //扩大失败
        return -1;
    }
    else
    {
        Offline_FlowBuff = newbuff;
    }
    //将记录写到内存中
    int buffaddr = (int)Offline_FlowBuff+sizeof(offline_flow_t)*(Offline_FlowNum);
    memcpy((char*)buffaddr,data,sizeof(offline_flow_t));

    //首先将该记录写入备份文件中
    //打开文件(ab)追加二进制文件
    FILE * filebak = fopen(OFFLINE_FLOWBAK_FILE,"ab");
    if(filebak != NULL)
    {
        setvbuf(filebak,NULL,_IONBF,0);
        //写流水到磁盘
        fwrite(data,sizeof(offline_flow_t),1,filebak);
        fclose(filebak);
    }
    int filelen = 0;
    //将该记录写入文件中
    //打开文件(ab)追加二进制文件
    FILE * file = fopen(OFFLINE_FLOWDATA_FILE,"ab");

    if(file == NULL)
    {
        setvbuf(file,NULL,_IONBF,0);
        //有文件但无法打开，返回-1
        //回退
        Offline_FlowBuff = realloc(Offline_FlowBuff,sizeof(offline_flow_t)*(Offline_FlowNum));
        return -1;
    }
    //写流水到磁盘
    fwrite(data,sizeof(offline_flow_t),1,file);
    fclose(file);

    /////////////////////////////////////////////////////////////////////
    //回读？？？
    //打开文件(ab)追加二进制文件
    file = fopen(OFFLINE_FLOWDATA_FILE,"rb");
    if(file == NULL)
    {
        //有文件但无法打开，返回-1
        //回退
        return -1;
    }
    //获取文件大小
    fseek(file,0L,SEEK_END);
    filelen = ftell(file);
    //首先判断大小是否正确，如果大小错误，返回-1
    if(filelen != sizeof(offline_flow_t)*(Offline_FlowNum+1))
    {
        myPtf("OFFLINE_FLOWDATA_FILE size error！should:%d,real:%d",sizeof(offline_flow_t)*(Offline_FlowNum+1),filelen);
        fclose(file);
        return -1;
    }
    //移动到最后一笔流水
    fseek(file,sizeof(offline_flow_t)*(Offline_FlowNum),SEEK_SET);
    offline_flow_t checkBuff;
    offline_flow_t orgBuff;
    memcpy(&orgBuff,data,sizeof(orgBuff));
    memset(&checkBuff,0,sizeof(checkBuff));
    fread(&checkBuff,1,sizeof(checkBuff),file);
    //如果内容错误，返回-1
    if((checkBuff.timeStamp != orgBuff.timeStamp)||(checkBuff.curTime != orgBuff.curTime)
       ||(checkBuff.accountId != orgBuff.accountId)||(checkBuff.cardNo != orgBuff.cardNo)||(checkBuff.flowMoney != orgBuff.flowMoney))
    {
        myPtf("OFFLINE_FLOWDATA_FILE info error！orgBuff:%d,%d,%d,%d,%d;checkBuff:%d,%d,%d,%d,%d",
              orgBuff.timeStamp,orgBuff.curTime,orgBuff.accountId,orgBuff.cardNo,orgBuff.flowMoney,
              checkBuff.timeStamp,checkBuff.curTime,checkBuff.accountId,checkBuff.cardNo,checkBuff.flowMoney);
        fclose(file);
        return -1;
    }
    fclose(file);
    /////////////////////////////////////////////////////////////////////

    //记录数增加
    Offline_FlowNum++;
    return 0;
}

//取出当前第一条流水之后的一批流水，用于上传 data在函数外分配
int fn_GetRecord(char * data,int number)
{
    //如果当前剩余流水量多余取出量，最多取出number个流水
    //如果当前剩余流水量少于等于取出量，全部取出
    int getnumber = Offline_FlowNum<number?Offline_FlowNum:number;
    //将该流水拷贝出来
    memcpy(data,(char *)Offline_FlowBuff,sizeof(offline_flow_t)*getnumber);
    return getnumber;
}

//删除当前第一条流水之后的一批流水，用于上传后处理 data在函数外分配
int fn_DelRecord(int number)
{
    if(Offline_FlowNum > number)
    {
        //如果当前剩余流水量多余删除量，最多删除number个流水
        //分配新内存块
        char * newbuf = (char *)malloc(sizeof(offline_flow_t)*(Offline_FlowNum-number));
        if(newbuf == NULL)
            return -1;
        int leftbuf = (int)Offline_FlowBuff + sizeof(offline_flow_t)*number;
        //将该流水拷贝出来
        memcpy(newbuf,(char *)leftbuf,sizeof(offline_flow_t)*(Offline_FlowNum-number));
        //回收原内存块
        free(Offline_FlowBuff);
        Offline_FlowBuff = newbuf;
        Offline_FlowNum -= number;
    }
    else
    {
        //如果当前剩余流水量少于等于删除量，全部删除
        free(Offline_FlowBuff);
        Offline_FlowBuff = NULL;
        Offline_FlowNum = 0;
    }
    return Offline_FlowNum;
}
//将未上传记录回写文件
int fn_WriteRecords()
{
    int ret = -1;
    myPtf("fn_WriteRecords\n");
    //打开文件(w+b)复写二进制文件
    FILE * file = fopen(OFFLINE_FLOWDATA_FILE,"w+b");
    if (file != NULL)
    {
        setvbuf(file,NULL,_IONBF,0);
        //写流水到磁盘
        if(fwrite(Offline_FlowBuff,sizeof(offline_flow_t),Offline_FlowNum,file)>0)
        {
            myPtf("fn_WriteRecords OK! filesize=%d*%d\n",sizeof(offline_flow_t),Offline_FlowNum);
            ret = 1;
        }
        else
        {
            myPtf("fn_WriteRecords Error! filesize=%d*%d\n",sizeof(offline_flow_t),Offline_FlowNum);
        }
        fclose(file);
    }
    return ret;
}

int getCurrentTime()
{
   struct timeval tv;
   gettimeofday(&tv,NULL);
   //myPtf("tv.tv_sec=%ld,tv.tv_usec=%ld\n",tv.tv_sec,tv.tv_usec);
   //return ((long long int)tv.tv_sec) * 1000 + ((long long int)tv.tv_usec) / 1000;
   return (tv.tv_sec%86400)*10000 + tv.tv_usec/100;
}

int code_convert(char *from_charset,char *to_charset,char *inbuf,int inlen,char *outbuf,int outlen)
{
        iconv_t cd;
        int rc;
        char **pin = &inbuf;
        char **pout = &outbuf;

        cd = iconv_open(to_charset,from_charset);
        if (cd==0)
        {
            perror("iconv_open");
            return -1;
        }

        size_t a=inlen;
        size_t b=outlen;

        memset(outbuf,0,outlen);
        //if (iconv(cd,pin,(size_t *)&inlen,pout,(size_t *)&outlen) == -1)
        if (iconv(cd,pin,&a,pout,&b) == -1)
        {
            perror("iconv");
            return -1;
        }
        iconv_close(cd);
        return 0;
}

int u2g(char *inbuf,int inlen,char *outbuf,int outlen)
{
        return code_convert("utf-8","gb2312",inbuf,inlen,outbuf,outlen);
}

int g2u(char *inbuf,size_t inlen,char *outbuf,size_t outlen)
{
        return code_convert("gb2312","utf-8",inbuf,inlen,outbuf,outlen);
        memcpy(outbuf,inbuf,inlen);
        return 0;
}

//错误提示
int fn_ShowErr(int ErrorCode)
{
    switch(ErrorCode)
    {
        //显示错误信息
        case -1:
            myPtf("errorcode -1\n");
            break;
        case -2:
            myPtf("errorcode -2\n");
            break;
        default:
            myPtf("errorcode 其它\n");
            break;
    }
    return 1;
}

//获取开机毫秒数
unsigned long GetTickCount()
{
    /*
    struct timespec ts;

    clock_gettime(CLOCK_MONOTONIC, &ts);

    return (ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
    */
    return 0;
}


//获取当前程序目录
int fn_GetCurrentPath(char buf[],char *pFileName)
{
#ifdef WIN32
	GetModuleFileName(NULL,buf,MAX_PATH);
#else
	char pidfile[64];
	int bytes;
	int fd;

	sprintf(pidfile, "/proc/%d/cmdline", getpid());

	fd = open(pidfile, O_RDONLY, 0);
	bytes = read(fd, buf, 256);
	close(fd);
	buf[MAX_PATH] = '\0';

#endif
	char * p = &buf[strlen(buf)];
	do
	{
		*p = '\0';
		p--;
#ifdef WIN32
	} while( '\\' != *p );
#else
	} while( '/' != *p );
#endif

	p++;

	//配置文件目录
	memcpy(p,pFileName,strlen(pFileName));
	return 0;
}

//从INI文件读取字符串类型数据
char *fn_GetIniKeyString(char *title,char *key,char *filename)
{
	FILE *fp;
	char szLine[1024];
	static char tmpstr[1024];
	int rtnval;
	int i = 0;
	int flag = 0;
	char *tmp;
	//myPtf("打开配置文件%s\n",filename);
	if((fp = fopen(filename, "r")) == NULL)
	{
		myPtf("have   no   such   file %s\n",filename);
		return "";
	}
	char line1[256];
	char line2[256];
	char line3[256];

	while(!feof(fp))
	{
		memset(line1,0,256);
		fgets(line1, sizeof(line1), fp);//IP地址
		line1[strlen(line1)-1]='\0';//去掉fgets自动增加的换行符


		memset(tmpstr,0,1024);
		strcpy(tmpstr,"[");
		strcat(tmpstr,title);
		strcat(tmpstr,"]");
		int pos=0;
		int len=0;
		if( strncmp(tmpstr,line1,strlen(tmpstr)) == 0 )
		{//含有title

			memset(line2,0,256);
			fgets(line2, sizeof(line2), fp);//IP地址
			line2[strlen(line2)-1]='\0';//去掉fgets自动增加的换行符
			memset(line3,0,256);
			fgets(line3, sizeof(line3), fp);//IP地址
			line3[strlen(line3)-1]='\0';//去掉fgets自动增加的换行符
			//判断是否含有关键字
			if(strstr(line2,key)!=NULL)
			{//含有关键字key
				//myPtf("%s含有关键字%s\n",line2,key);
				//查找字符串中出现关键字的位置
				tmp = strchr(line2, '=');
				if(tmp)
				{
					pos=tmp-line2+1;
					len=strlen(line2);
				        //myPtf("字符串%s出现关键字%s的位置%d,%d\n",line2,key,pos,len);
					memset(tmpstr,0,1024);
					sprintf(tmpstr,"%s",&line2[pos]);

				}
				break;
			}//含有关键字
			else if(strstr(line3,key)!=NULL)
			{//含有关键字key
				//myPtf("%s含有关键字%s\n",line3,key);
				//查找字符串中出现关键字的位置
				tmp = strchr(line3, '=');
				if( tmp )
				{
					pos=tmp-line3+1;
					len=strlen(line3);
				        //myPtf("字符串%s出现关键字%s的位置%d,%d\n",line3,key,pos,len);
					memset(tmpstr,0,1024);
					sprintf(tmpstr,"%s",&line3[pos]);
				}

				break;
			}//含有关键字key

			break;
		}//含有title



		//linebuf=malloc(256);
		//linebuf=fn_GetStringBySpecailChar(line);
		//myPtf("行处理返回%s",linebuf);
		/*rtnval = fgetc(fp);

		if(rtnval == EOF)
		{
			break;
		}
		else
		{
			szLine[i++] = rtnval;
		}

		if(rtnval == '\n')
		{
#ifndef WIN32
			i--;
#endif
			szLine[--i] = '\0';
			i = 0;
			tmp = strchr(szLine, '=');

			if(( tmp != NULL )&&(flag == 1))
			{
				if(strstr(szLine,key)!=NULL)
				{
					//注释行
					if ('#' == szLine[0])
					{
					}

					//else if ( '\/' == szLine[0] && '\/' == szLine[1] )
					//{

					//}
					else
					{
						//找打key对应变量
						strcpy(tmpstr,tmp+1);
						fclose(fp);
						return tmpstr;
					}
				}
			}
			else
			{
				strcpy(tmpstr,"[");
				strcat(tmpstr,title);
				strcat(tmpstr,"]");
				if( strncmp(tmpstr,szLine,strlen(tmpstr)) == 0 )
				{
					//找到title
					flag = 1;
				}
			}
		}*/
	}

	fclose(fp);
	//myPtf("找到%s\n",tmpstr);
	return &tmpstr[0];
}

//从INI文件读取整类型数据
int fn_GetIniKeyInt(char *title,char *key,char *filename)
{
	return atoi(fn_GetIniKeyString(title,key,filename));
}


char * GetKBList(int * pKBNumber)
{
    *pKBNumber = 0;
	char cmd[1024];
	memset(cmd,0,1024);
	char buf[1024];
	memset(buf,0,1024);
	FILE *ptr = NULL;

    int keybdcount = 0;
    //1.0将系统文件映射为临时文件
	system(CASTFILE_CMD);

	//2.0通过临时文件获取总数量
	sprintf(cmd,"cat %s | grep bus -c",DEVICE_LIST_FILE);
	if((ptr = popen(cmd, "r"))==NULL)
	{
		return NULL;
	}
	memset(buf, 0, sizeof(buf));
	fgets(buf, sizeof(buf),ptr);
	fclose(ptr);
    keybdcount = atoi(buf);
    //如果没读到，退出
    if(keybdcount == 0)
        return NULL;
    //3.0循环读取每个设备信息
    //3.1分配3维数组空间
    char (*deviceList)[DEV_ITEMLEN][DEV_INFOLEN] = (char (*)[DEV_ITEMLEN][DEV_INFOLEN])malloc(keybdcount*DEV_ITEMLEN*DEV_INFOLEN);
    memset(deviceList,0,keybdcount*DEV_ITEMLEN*DEV_INFOLEN);
    //3.2定义读取位置,char*的指针二维数组
    char *itemPos[DEV_ITEMLEN][2] =
    {
        {"Bus","2"},
        {"Bus","3"},
        {"Bus","4"},
        {"Bus","5"},
        {"Name","2"},
        {"Phys","2"},
        {"Sysfs","2"},
        {"Handlers","2"}
    };
    char * pTemp;
    //3.2循环读取_1重循环，设备数量
	for(int i=0;i<keybdcount;i++)
	{
        //3.3循环读取_2重循环，设备项数
        for(int j=0;j<DEV_ITEMLEN;j++)
        {
            memset(cmd,0,sizeof(cmd));
            if (j<4)
            {
                sprintf(cmd,"cat %s | grep %s | sed -n %dp  | awk '{print $%s}' | awk -F['='] '{print $2}'"
                        ,DEVICE_LIST_FILE,itemPos[j][0],i+1,itemPos[j][1]);
            }
            else
            {
                sprintf(cmd,"cat %s | grep %s | sed -n %dp | awk -F['='] '{print $2}'"
                        ,DEVICE_LIST_FILE,itemPos[j][0],i+1);
            }
            //myPtf("i=%d,j=%d,cmd=%s\n",i,j,cmd);
            if((ptr = popen(cmd, "r"))==NULL)
            {
                myPtf("open error! continued!\n");
                continue;
            }
            memset(buf,0,sizeof(buf));
            //3.4将内容拷贝出来
            if((fgets(buf, sizeof(buf),ptr))!= NULL)
            {
                //myPtf("%s\n",buf);
                pTemp = (char *)(((long)deviceList)+i*DEV_ITEMLEN*DEV_INFOLEN+j*DEV_INFOLEN);
                memcpy(pTemp,buf,DEV_INFOLEN-1);
            }
            else
            {
                //myPtf("read error!\n");
            }
            fclose(ptr);
        }
	}

    //单个输入设备信息，DEV_ITEMNUM表示每个设备有8项信息，DEV_INFOLEN表示每项信息有256个字符串来描述
    char deviceInfo[DEV_ITEMLEN][DEV_INFOLEN];

    //4.0显示验证
	for(int i=0;i<keybdcount;i++)
	{
	    //拷贝一个设备信息
	    pTemp = (char *)(((long)deviceList)+i*DEV_ITEMLEN*DEV_INFOLEN);
	    memcpy(deviceInfo,pTemp,DEV_ITEMLEN*DEV_INFOLEN);
	    //显示该设备信息
	    /*
        myPtf("i=%d,DEV_BUS=%d,info=%s",i,DEV_BUS,deviceInfo[DEV_BUS]);
        myPtf("i=%d,DEV_VENDOR=%d,info=%s",i,DEV_VENDOR,deviceInfo[DEV_VENDOR]);
        myPtf("i=%d,DEV_PORDUCT=%d,info=%s",i,DEV_PORDUCT,deviceInfo[DEV_PORDUCT]);
        myPtf("i=%d,DEV_VERSION=%d,info=%s",i,DEV_VERSION,deviceInfo[DEV_VERSION]);
        myPtf("i=%d,DEV_NAME=%d,info=%s",i,DEV_NAME,deviceInfo[DEV_NAME]);
        myPtf("i=%d,DEV_PHYS=%d,info=%s",i,DEV_PHYS,deviceInfo[DEV_PHYS]);
        myPtf("i=%d,DEV_SYSFS=%d,info=%s",i,DEV_SYSFS,deviceInfo[DEV_SYSFS]);
        myPtf("i=%d,DEV_HANDLERS=%d,info=%s\n",i,DEV_HANDLERS,deviceInfo[DEV_HANDLERS]);
        */
	}
    //myPtf("GetKBList return\n");
    *pKBNumber = keybdcount;
    return (char *)deviceList;
}
