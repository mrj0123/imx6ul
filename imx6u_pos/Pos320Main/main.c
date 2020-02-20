#include "afunix_udp.h"
#include "dispose.h"
#include "dealcmd.h"
#include "system.h"
#include "a.h"
#include "log.h"
#include <math.h>



#define CONF_FILE_PATH	"Config.ini"
#define GPIO_NUM 23

#include "md5.h"
 
#include <stdio.h>
#include <stdlib.h>


#include <sys/types.h>
#include <sys/stat.h>


#include <unistd.h>
#include <fcntl.h>

#include "nksocket.h"
#include "inifile.h"
 
#include <time.h>
#include <ctype.h>

#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "cJSON.h"

#include "common.h"

#include <pthread.h>

#include <printf.h>

#include <errno.h>


#include "getnetsetting.h"

#include "sendcmd_cloud.h"

#define LOCKFILE "/var/run/pos320main.pid"

#define LOCKMODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)


#define READ_DATA_SIZE	1024
#define MD5_SIZE		16
#define MD5_STR_LEN		(MD5_SIZE * 2)

#define CMD_STR_LEN 1024


int Compute_string_md52(char *string_ori, char *md5_str);

int Compute_file_md5(const char *file_path, char *value);


int already_running(const char *filename);

int lockfile(int fd);

//检查进程是否存在
int checkProcessExist(char *processname);

//线程函数
//void mythreadfun(void);

//自动升级
int myautoupdate();

int my_system(const char * cmd,char *szretbuf);

//自动启动
int myautostart();
int mythreadfun();
//启动服务
int startmainservice();

//返回键盘输入个数
int GetKeyBoardCount(int *size);

//返回键盘输入列表
int GetKeyBoardList(int *size);
//获得硬件唯一识别码
char * GetHardwareSerialNo(int *reclen);




//////////////////////////////////




//正在升级标志
int g_isrunningupdate=0;

int my_system(const char * cmd,char *szretbuf)
{
	FILE * fp;
	int res; char buf[1024];
	memset(szretbuf,0,1024);
	if(cmd == NULL)
	{
		myPtf("my_system cmd is NULL!\n");
		return -1;
	}
	if ((fp = popen(cmd, "r") ) == NULL)
	{
		perror("popen");
		myPtf("popen error: %s/n", strerror(errno)); return -1;
	}
	else
	{
		while(fgets(buf, sizeof(buf), fp))
		{
			//myPtf("%s", buf);
			memset(szretbuf,0,1024);
			sprintf(szretbuf,"%s",buf);
		}
		if ( (res = pclose(fp)) == -1)
		{
			myPtf("close popen file pointer fp error!\n"); return res;
		}
		else if (res == 0)
		{
			return res;
		}
		else
		{

			myPtf("popen res is :%d\n", res); return res;
		}
	}
}

//启动主程序
int main()
{
    
	/*int keybdcount=0;
	int myret=GetKeyBoardCount(&keybdcount);
	myPtf("%d,%d\n",myret,keybdcount);
	if(myret==1)
	{//返回1，分配内存
		g_kbd_list=malloc(keybdcount*sizeof(kbd_list_t));
		myret=GetKeyBoardList(&keybdcount);	
		int i=0;
		for(int i=0;i<keybdcount;i++)
		{
			myPtf("%d,Bus=%s,Vendor=%s,Product=%s,Version=%s,Name=%s,Phys=%s,Sysfs=%s\n",i,g_kbd_list[i].bus,g_kbd_list[i].vendor,
				g_kbd_list[i].product,g_kbd_list[i].version,g_kbd_list[i].name,g_kbd_list[i].phys,g_kbd_list[i].sysfs);
			
		}
	}//返回1，分配内存
	return 0;

*/
	/*char xxx[1024];
	char yyy[1024];
	memset(xxx,0,1024);
	sprintf(xxx,"sh %s/getipaddress.sh",USER_SCRIPT_DIRECTORY);
	my_system(xxx,&yyy[0]);
	yyy[strlen(yyy)-1]='\0';
	myPtf("%s\n",yyy);
	return 0;*/

	if  (already_running(LOCKFILE))
            return 0;
    	myPtf("version=%s\n",POSMAIN_VERSION);
    	write_log_s("====version=%s running====\n",POSMAIN_VERSION);
	   
	getGPRSsetting();
    	write_log_s("====4G Setting\n");
    	write_log_s("nettype:%d\n",g_gprs_setting.nettype);
    	write_log_s("netstatus:%d\n",g_gprs_setting.status);
    	write_log_s("ip address:%s\n",g_gprs_setting.ipaddress);
   	g_isrunningupdate=0;

    	if(AUTOUPDATE_FLAG==1)
	{//开启自动升级	
		myPtf("%s\n","自动升级版本");
		myautoupdate();
		return 0;
	}//开启自动升级
	else
	{//不开启自动升级
		if(AUTOSTART_OTHER==1)
		{//自动启动其他模块
			//自动启动
			myautostart(); 
		
		}//自动启动其他模块
		
		
	}//不开启自动升级
	//启动服务
	startmainservice();
    	return 0;
}

//启动服务
int startmainservice()
{



	
	//启动服务
	int serverfd = s_init_net(GATE_PATH_S);
	if (serverfd<=0)
	{
	 myPtf("serverfd init error:%d",serverfd);
	 return -1;
	}
	//循环接收命令
	fn_dispose(serverfd);

	//关闭网络
	close_net(serverfd,GATE_PATH_S);
	return 0;
}
int mythreadfun()
{
	while(1)
	{
		sleep(5);		
		myPtf("autostart thread function begin\n");
		char cmd[1024];
		memset(cmd,0,1024);
		sprintf(cmd,"sh /usr/local/nkty/autosingle.sh consumeser" );
		system(cmd);
		memset(cmd,0,1024);
		sprintf(cmd,"sh /usr/local/nkty/autosingle.sh scanqrser" );
		system(cmd);
		memset(cmd,0,1024);
		sprintf(cmd,"sh /usr/local/nkty/autosingle.sh secscreen" );
		system(cmd);
		char *processname="weston-desktop";
		//判断weston-desktop
		int existsflag=checkProcessExist(processname);
		myPtf("进程:%s,启动状态:%d\n",processname,existsflag);	
		/*if(existsflag==0)
		{//进程未启动，需要启动进程
			myPtf("weston-desktop not exists,need start weston-desktop\n");
			memset(cmd,0,1024);
			sprintf(cmd,"/etc/init.d/weston start" );
			system(cmd);
			sleep(3);	
		}//进程未启动，需要启动进程*/
		memset(cmd,0,1024);
		sprintf(cmd,"sh /usr/local/nkty/autosingle.sh terminalProject" );
		system(cmd);


		myPtf("autostart thread function end\n");
		
	}
	return 0;
}
//自动启动
int myautostart()
{
	myPtf("myautostart begin \n");

	//创建线程，用来检查各个程序的启动版本，此线程永久执行，随主程序启动而启动，随主程序关闭而关闭
	pthread_t id;  

	int t_ret;  

	t_ret = pthread_create( &id, NULL, (void *) mythreadfun, NULL); 

	if( t_ret != 0 ) {  
		myPtf(" Create thread error!\n");  
	}  
	myPtf(" Create thread ok!\n");  
	myPtf("myautostart end \n");
	return 0;
}
//自动升级
int myautoupdate()
{
	//===========开始连接升级服务器，获得更新列表=========
	char szfilename1[MAX_PATH];
	memset(szfilename1,0,MAX_PATH);
	sprintf(szfilename1,"%s","/usr/local/nkty/script/nktyserver.conf");
	//myPtf("文件名：%s\n",szfilename1);

	int i=0;
	ser_conn_t serconn[MAX_SERVER_NUM]; 

	char *serveripPtr=NULL;
	serveripPtr =  (char *)malloc(100*sizeof(char *));
	int port=0;
	char szsername[100];
	for(i=0;i<MAX_SERVER_NUM;i++)
	{
		memset(serveripPtr,0,100);
		memset(szsername,0,100);
		sprintf(szsername,"server%d",(i+1));
		szsername[strlen(szsername)]='\0';
		serveripPtr=fn_GetIniKeyString(szsername,"ipaddress",szfilename1);
		sprintf(serconn[i].SerIP,"%s",serveripPtr);
		port=fn_GetIniKeyInt(szsername,"port",szfilename1);
		serconn[i].SerPort=port;
	}

	int ret=init_client_socket(serconn);
	myPtf("连接升级服务器返回值%d\n",ret);
	if(ret!=0)
	{//连接升级服务器失败，直接启动程序
		if(AUTOSTART_OTHER==1)
		{//自动启动其他模块
			myautostart();
	    	}//自动启动其他模块
		//启动服务
	    	startmainservice();
		return 0;
	}//连接升级服务器失败，直接启动程序
	//============================准备发送命令===================================
	//分配内存
	
	char cmd[1024];
	memset(cmd,0,1024);

	//表示正在升级程序
	g_isrunningupdate=1;

	char* ptrBuffer=malloc(1024*sizeof(char *));
	memset(ptrBuffer,0,1024*sizeof(char *));

	char szhardserialno[64];
	

	/*memset(cmd,0,1024);
	//获得硬件唯一识别码
        sprintf(cmd, "cat /sys/fsl_otp/HW_OCOTP_CFG1  > %s/hardsn.txt",USER_TEMP_DIRECTORY);
	        system(cmd);
	//文件名字
	char szfilename2[200];
	memset(szfilename2,0,200);
	sprintf(szfilename2,"%s/hardsn.txt",USER_TEMP_DIRECTORY);
	//打开文件
	FILE *fp = fopen(szfilename2, "r");
	memset(szhardserialno,0,256);
	//读取一行
	fgets(szhardserialno, sizeof(szhardserialno), fp);
	fclose(fp);
	
	memset(cmd,0,1024);
	//获得硬件唯一识别码
        sprintf(cmd, "rm -rf %s/hardsn.txt",USER_TEMP_DIRECTORY);
        system(cmd);
*/
	int ilen=0;
	char *szbufferOut=NULL;
	szbufferOut=(char *)GetHardwareSerialNo(&ilen);
	
	memset(szhardserialno,0,64);
	sprintf(szhardserialno,"%s",szbufferOut);
	free(szbufferOut);
	myPtf("终端唯一识别码%s\n",szhardserialno);
	//==================================发送命令===============================================
	int sendlen=0;
	char sztosend[1024];
	memset(sztosend,0,1024);
	client_Cmd_t *client_updatecmd;
	client_updatecmd=malloc(sizeof(client_Cmd_t));
	//命令字，3011
	int iCmd=(int)UPDATE_INFO_GET;
	//填充参数
	client_updatecmd->cmd=(int)UPDATE_INFO_GET;
	//终端版本号
	sprintf(client_updatecmd->termVersion,"%s","1.1");
	//终端唯一识别码
	sprintf(client_updatecmd->termCode,"%s",szhardserialno);

	client_updatecmd->termCode[strlen(client_updatecmd->termCode)]='\0';
	//计算发送字节数
	sendlen=sizeof(client_Cmd_t);

	int reclen=0;
		
	myPtf("发送命令%d,命令长度%d\n",iCmd,sendlen);
	myPtf("%s,%s,%d\n",client_updatecmd->termVersion,client_updatecmd->termCode,client_updatecmd->cmd);

	//char * sendCmd_socket(int *pCmd, char * pBuf, int bufLen, int *retLen);
	char szdata[256];
	memset(szdata,0,256);
	sprintf(szdata,"{\"cmd\":%d,\"termVersion\":\"%s\",\"termCode\":\"%s\"}",iCmd,"1",client_updatecmd->termCode);	
	sendlen=strlen(szdata);
	szdata[sendlen]='0';
 	//接收数据的包身
     	char *myret = NULL;	
	myret=malloc(4096);
	myret=sendCmd_socket(&iCmd,(char *)szdata,sendlen,&reclen);
	myPtf("命令返回1111%s,返回长度%d\n",myret,reclen);
	//如果返回长度为0，说明与升级服务器连接失败
	if(reclen==0) 
	{//如果返回长度为0，说明与升级服务器连接失败
		//表示正在升级程序
		g_isrunningupdate=0;
		if(AUTOSTART_OTHER==1)
		{//自动启动其他模块
			myautostart();
	    	}//自动启动其他模块

		startmainservice();
		return 0;	
	}//如果返回长度为0，说明与升级服务器连接失败

	/*char *temp="{\"cmd\":3011,\"ret\":10,\"msg\":null,\"ftpUser\":\"anonymous\",\"ftpPwd\":\"\",\"upInfos\":[{\"md5\":\"3D181F6B39F9029772B3FA0D21D97C1A\",\"downloadPath\":\"http://www.jcshdy.com/QuestionNaireWeb/appdownload/terminal/Pos320Main\",\"upGrades\":1},{\"md5\":\"518119CC7CE7FE78DE5D9481EC2D7F13\",\"downloadPath\":\"ftp://10.1.70.52/RD/update/terminal/terminalProject\",\"upGrades\":2}]}";*/

	/*char *temp="{\"cmd\":3011,\"ret\":10,\"msg\":null,\"ftpUser\":\"anonymous\",\"ftpPwd\":\"\",\"upInfos\":[{\"md5\":\"ab32b81af8d16b0e731a2c3c7fb4ab15\",\"downloadPath\":\"ftp://10.1.70.52/RD/update/terminal/Pos320Main\",\"upGrades\":2,\"setupPath\":\"/\"},{\"md5\":\"EAE2A556786DBCD600BF972DC6F399D5\",\"downloadPath\":\"ftp://10.1.70.52/RD/update/terminal/terminalProject\",\"upGrades\":2,\"setupPath\":\"/\"},{\"md5\":\"84D3C5AF7243455A1999C2D5CB819CE7\",\"downloadPath\":\"ftp://10.1.70.52/RD/update/terminal/consumeser\",\"upGrades\":2,\"setupPath\":\"/\"}]}";
*/
	cJSON* cjson = cJSON_Parse(myret);
	//cJSON* cjson = cJSON_Parse(temp);
	if(cjson == NULL){

		write_log_s("%s\n","updateinfo json pack into cjson error...");
	}
	
	//write_log_s("%s\n","json pack into cjson ok...");
	//write_log_s("======%s===========\n",temp);

	//开始对数据进行json解码
	//返回数据的命令字
	int iretcmd= (int)cJSON_GetObjectItem(cjson,"cmd")->valueint;//获取键值内容
	//返回数据的返回值
	int iretval= (int)cJSON_GetObjectItem(cjson,"ret")->valueint;//获取键值内容
	//返回数据的ftp账号
	char *szftpusername=(char *)cJSON_GetObjectItem(cjson,"ftpUser")->valuestring;//获取键值内容
	char *szftppassword=(char *)cJSON_GetObjectItem(cjson,"ftpPwd")->valuestring;//获取键值内容
	//myPtf("json解释iretcmd:%d,iretval:%d,ftp usename:%s,ftp password:%s\n",iretcmd,iretval,szftpusername,szftppassword);
	
	
	
	cJSON *upInfos = cJSON_GetObjectItem(cjson, "upInfos"); // cjosnDate为上述的cjson数据

	int iCount = cJSON_GetArraySize(upInfos); /*获取数组长度*/
	//删除升级程序列表文件
	memset(cmd,0,1024);
	sprintf(cmd,"rm -rf %s/update/update.txt",USER_HOME_DIRECTORY);
	system(cmd);

	//检查dns
	memset(cmd,0,1024);
	sprintf(cmd,"sh %s/checkdns.sh",USER_SCRIPT_DIRECTORY);
	system(cmd);



	//需要更新的文件个数
	int needupdatecount=0;
	
	for(i=0;i<iCount;i++)
	{//遍历每个文件
		cJSON *tempJSON = cJSON_GetArrayItem(upInfos, i); // cjosnDate为上述的cjson数据	
		char *md5=(char *)cJSON_GetObjectItem(tempJSON,"md5")->valuestring;//获取键值内容
		char *downloadPath=(char *)cJSON_GetObjectItem(tempJSON,"downloadPath")->valuestring;//获取键值内容
		int upGrades=(int)cJSON_GetObjectItem(tempJSON,"upGrades")->valueint;//获取键值内容 
		
		char *setupPath=(char *)cJSON_GetObjectItem(tempJSON,"setupPath")->valuestring;//获取键值内容
		char *versionCode=(char *)cJSON_GetObjectItem(tempJSON,"versionCode")->valuestring;//获取键值内容

		char newmd5[100];
		memset(newmd5,0,100);

		for(int p=0;p<strlen(md5);p++)
		{
			if((md5[p]>='a')&&(md5[p]<='z'))
			{
				newmd5[p]=md5[p]-32;
			}
			else
			{
				newmd5[p]=md5[p];
			}
		}
		newmd5[strlen(md5)]='\0';
		char str[2048];
		memset(str,0,2048);
		sprintf(str,"%s",downloadPath);
		//获得文件名,截取字符串
		char delims[] = "/";
    		char *result = NULL;
		result = strtok( str, delims );
		int k=0;
		while( result != NULL ) {
		    //myPtf( "result is %s\n", result );
		    result = strtok( NULL, delims );
			k=k+1;
		}
		//myPtf( "index is %d\n", k );
		int m=0;
		memset(str,0,2048);
		sprintf(str,"%s",downloadPath);
		result = strtok( str, delims );
		while( result != NULL ) {
		   // myPtf( "result is %s\n", result );
		    result = strtok( NULL, delims );
			if(m==k-2)
			{
				break;
			}
			m=m+1;
		}

		
		//myPtf("遍历数组111111111%d,%s,%s,%d,%d,%s\n",i,md5,downloadPath,upGrades,k,result);
	
		char szfilename[200];
		memset(szfilename,0,200);
    		sprintf(szfilename,"%s/%s/%s",USER_HOME_DIRECTORY,setupPath,result);
		//myPtf("准备更新%s\n", szfilename);

		char szupdatepath[200];
		memset(szupdatepath,0,200);
    		sprintf(szupdatepath,"%s/%s",USER_HOME_DIRECTORY,"update");
		myPtf("%s\n", szupdatepath);

		int checkret=0;

		char cmd[1024];
		char md5_str[MD5_STR_LEN + 1];
		//判断文件update.conf是否存在 
		if((access(szfilename,F_OK))!=-1)   
	    	{//文件存在,计算本地文件的MD5码，并和服务器上的文件的MD5比较
			myPtf("%s存在，判断MD5\n", szfilename);
			memset(md5_str,0,MD5_STR_LEN + 1);
			int 	ret2 = Compute_file_md5(szfilename, md5_str);
			if (0 == ret2)
			{//计算本地MD5成功
				myPtf("[file - %s] md5 value:\n", szfilename);
				myPtf("本地文件md51:%s,%d\n", md5_str,strlen(md5_str));
				myPtf("下载文件md52:%s,%d\n", md5,strlen(md5));
				myPtf("下载文件md53:%s,%d\n", newmd5,strlen(newmd5));
				

				if(strcmp(md5_str, newmd5) != 0)
				{//md5不相等，说明不是同一个文件，需要更新
					if (access(szupdatepath,R_OK)!=0)
					{//目录不存在，创建目录
						memset(cmd,0,1024);
						sprintf(cmd,"mkdir %s",szupdatepath);
						system(cmd);
						memset(cmd,0,1024);
						sprintf(cmd,"chmod -R 777 %s",szupdatepath);
						system(cmd);
					}
					myPtf("%s\n", "md5不相同，需要更新");
					//关闭进程
					//memset(cmd,0,1024);
					//sprintf(cmd,"sh /%s/killsingle.sh %s",USER_HOME_DIRECTORY,result);
					//myPtf("%s\n",cmd);					
					//system(cmd);
					//下载进程
					//设置权限
					memset(cmd,0,1024);
					sprintf(cmd,"chmod 777 %s/update/%s",USER_HOME_DIRECTORY,result);
					system(cmd);
					
					

					//判断是否http下载
					if (upGrades==1)
					{//http下载

						//删除文件
						memset(cmd,0,1024);
						sprintf(cmd,"rm -rf %s/update/%s",USER_HOME_DIRECTORY,result);
						checkret=system(cmd);
						myPtf("删除update目录里面的文件%s,返回值%d\n",result,checkret);
						FILE *fp1;
						char buffer1[80];
						char url1[256];
						memset(url1,0,256);
						sprintf(url1,"ftp://%s:%s@%s ",szftpusername,szftppassword,&downloadPath[6]);	
						char ss1[200];
						memset(ss1,0,200);
						sprintf(ss1,"%s/checkurl.sh %s",USER_HOME_DIRECTORY,url1);
						fp1 = popen(ss1,"r");
						fgets(buffer1,sizeof(buffer1),fp1);
						myPtf("检查ftp http %s有效行返回%s\n",url1,buffer1);
						pclose(fp1);
						buffer1[strlen(buffer1)]='\0';
						checkret=atoi(buffer1);
						myPtf("数字返回%d,%d\n",checkret,checkret-200);
						if(checkret==200)
						{
							myPtf("http访问%s 成功\n",downloadPath);
							memset(cmd,0,1024);
							sprintf(cmd,"sudo wget %s -O %s/update/%s ",downloadPath,USER_HOME_DIRECTORY,result);
							myPtf("%s\n",cmd);
							system(cmd);
							memset(cmd,0,1024);
							sprintf(cmd,"echo http download %s finished",result);
							system(cmd);
							//分配权限
							memset(cmd,0,1024);
							sprintf(cmd,"chmod 777 %s/update/%s",USER_HOME_DIRECTORY,result);
							system(cmd);
						}
						else
						{
							myPtf("http访问%s 失败\n",downloadPath);
							continue;//继续下一个
						}

						


					}//http下载
					else if (upGrades==2)
					{//ftp下载
						//删除文件
						memset(cmd,0,1024);
						sprintf(cmd,"rm -rf %s/update/%s",USER_HOME_DIRECTORY,result);
						checkret=system(cmd);
						myPtf("删除update目录里面的文件%s,返回值%d\n",result,checkret);

						memset(cmd,0,1024);
						FILE *fp2;
						char buffer2[80];
						char url2[256];
						memset(url2,0,256);
						sprintf(url2,"ftp://%s:%s@%s ",szftpusername,szftppassword,&downloadPath[6]);
						char ss2[200];
						memset(ss2,0,200);
						sprintf(ss2,"%s/checkurl.sh %s",USER_HOME_DIRECTORY,url2);
						fp2 = popen(ss2,"r");	
						fgets(buffer2,sizeof(buffer2),fp2);
						myPtf("检查ftp url %s有效行返回%s\n",url2,buffer2);
						pclose(fp2);

						buffer2[strlen(buffer2)]='\0';
						checkret=atoi(buffer2);
						myPtf("数字返回%d,%d\n",checkret,checkret-226);
						if(checkret==226)
						{
							myPtf("ftp访问%s 成功\n",downloadPath);
							
							myPtf("ftp下载%s,md5:%s\n",result,md5);						
							sprintf(cmd,"wget ftp://%s:%s@%s -O %s/update/%s",szftpusername,szftppassword,
							&downloadPath[6],USER_HOME_DIRECTORY,result);

							int icmd=system(cmd);
							myPtf("wget ftp命令返回%d\n",icmd);	
							memset(cmd,0,1024);
							sprintf(cmd,"echo ftp download %s finished\n",result);
							system(cmd);
							//分配权限
							memset(cmd,0,1024);
							sprintf(cmd,"chmod 777 %s/update/%s",USER_HOME_DIRECTORY,result);
							system(cmd);
						}
						else
						{
							myPtf("ftp访问%s 失败\n",downloadPath);
							continue;//继续下一个
						}

						//     ftp://10.1.70.52/RD/update/terminal/terminalProject
						//wget ftp://anonymous:@10.1.70.52/RD/update/terminal/terminalProject   -O //usr/local/nkty/update/terminalProject
												
						
						
						

					}//ftp下载
					
					//关闭进程
					//判断进程weston-desktop-是否已经启动
					int existsflag=checkProcessExist(result);
	    				myPtf("进程:%s,启动状态:%d\n",result,existsflag);	
					if(existsflag==1)
					{//进程已启动，需要结束进程
						char *buf1="Pos320Main";
						int kkk=strncmp(result,buf1,10);
						myPtf("进程:%s,是否:Pos320Main，%d\n",result,kkk);	
						if(kkk != 0)
						{
							myPtf("POs320Main kill %s\n ",result);
							memset(cmd,0,1024);
							sprintf(cmd,"killall -9 %s ",result);
							system(cmd);
						}	
					}//进程已启动，需要结束进程
					
					memset(cmd,0,1024);
					sprintf(cmd,"echo %s,%s >> %s/update/update.txt",result,setupPath,USER_HOME_DIRECTORY);
					system(cmd);

					//需要更新的文件个数+1
					needupdatecount=needupdatecount+1;
				}//md5不相等，说明不是同一个文件，需要更新
				

				
			}
		}//文件存在,计算本地文件的MD5码，并和服务器上的文件的MD5比较
		else
		{//文件不存在，直接下载
			myPtf("%s不存在，直接下载\n", szfilename);			
				if (access(szupdatepath,R_OK)!=0)
				{
       					memset(cmd,0,1024);
					sprintf(cmd,"mkdir %s",szupdatepath);
					system(cmd);
					memset(cmd,0,1024);
					sprintf(cmd,"chmod -R 777 %s",szupdatepath);
					system(cmd);
				}

				myPtf("%s\n", "文件不存在，直接下载");
		
				//下载进程
				//判断是否http下载
				if (upGrades==1)
				{//http下载
					//删除文件
					memset(cmd,0,1024);
					sprintf(cmd,"rm -rf %s/update/%s",USER_HOME_DIRECTORY,result);
					checkret=system(cmd);
					myPtf("删除update目录里面的文件%s,返回值%d\n",result,checkret);

					FILE *fp3;
					char buffer3[80];
					char url3[256];
					memset(url3,0,256);
					sprintf(url3,"ftp://%s:%s@%s ",szftpusername,szftppassword,&downloadPath[6]);	
					char ss3[200];
					memset(ss3,0,200);
					sprintf(ss3,"%s/checkurl.sh %s",USER_HOME_DIRECTORY,url3);
					fp3 = popen(ss3,"r");	

					fgets(buffer3,sizeof(buffer3),fp3);
					myPtf("检查 http %s有效行返回%s\n",url3,buffer3);
					pclose(fp3);

					buffer3[strlen(buffer3)]='\0';
					checkret=atoi(buffer3);
					myPtf("数字返回%d,%d\n",checkret,checkret-200);
					if(checkret==200)
					{
						myPtf("http访问%s 成功\n",downloadPath);
						memset(cmd,0,1024);
						sprintf(cmd,"sudo wget %s -O %s/update/%s ",downloadPath,USER_HOME_DIRECTORY,result);
						myPtf("%s\n",cmd);
						system(cmd);
				
						memset(cmd,0,1024);
						sprintf(cmd,"echo download %s finished",result);
						system(cmd);
					}
					else
					{
						myPtf("htt访问%s 失败，%d\n",downloadPath,checkret);
						continue;//继续下一个
					}


					
				}//http下载
				else if (upGrades==2)
				{//ftp下载
					
						//删除文件
						memset(cmd,0,1024);
						sprintf(cmd,"rm -rf %s/update/%s",USER_HOME_DIRECTORY,result);
						checkret=system(cmd);
						myPtf("删除update目录里面的文件%s,返回值%d\n",result,checkret);
						FILE *fp4;
						char buffer4[80];
						char url4[256];
						memset(url4,0,256);
						sprintf(url4,"ftp://%s:%s@%s ",szftpusername,szftppassword,&downloadPath[6]);	
						char ss4[200];
						memset(ss4,0,200);
						sprintf(ss4,"%s/checkurl.sh %s",USER_HOME_DIRECTORY,url4);
						fp4 = popen(ss4,"r");	

						fgets(buffer4,sizeof(buffer4),fp4);
						myPtf("检查 ftp %s有效行返回%s\n",url4,buffer4);
						pclose(fp4);
						
						buffer4[strlen(buffer4)]='\0';
						checkret=atoi(buffer4);
						myPtf("数字返回%d,%d\n",checkret,checkret-226);
						if(checkret==226)
						{
							myPtf("ftp访问%s， 成功\n",downloadPath);
							
													
							sprintf(cmd,"wget ftp://%s:%s@%s -O %s/update/%s",szftpusername,szftppassword,
										&downloadPath[6],USER_HOME_DIRECTORY,result);

							int icmd=system(cmd);
							myPtf("wget ftp命令返回%d\n",icmd);	
							memset(cmd,0,1024);
							sprintf(cmd,"echo ftp download %s finished\n",result);
							system(cmd);
							//分配权限
							memset(cmd,0,1024);
							sprintf(cmd,"chmod 777 %s/update/%s",USER_HOME_DIRECTORY,result);
							system(cmd);
						}
						else
						{
							myPtf("htt访问%s 失败，%d\n",downloadPath,checkret);
							continue;//继续下一个
						}
						
						
				}//ftp下载


				//关闭进程
			        //判断进程weston-desktop-是否已经启动
				int existsflag=checkProcessExist(result);
    				myPtf("进程:%s,启动状态:%d\n",result,existsflag);	
				if(existsflag==1)
				{//进程已启动，需要结束进程
					char *buf1="Pos320Main";
					int kkk=strncmp(result,buf1,10);
					myPtf("进程:%s,是否:Pos320Main，%d\n",result,kkk);	
					if(kkk != 0)
					{
						myPtf("POs320Main kill %s ",result);						
						memset(cmd,0,1024);
						sprintf(cmd,"killall -9 %s ",result);
						system(cmd);
					}
				}//进程已启动，需要结束进程
				
				memset(cmd,0,1024);
				sprintf(cmd,"echo %s,%s >> %s/update/update.txt",result,setupPath,USER_HOME_DIRECTORY);
				system(cmd);
				

				//需要更新的文件个数+1
				needupdatecount=needupdatecount+1;
		}//文件不存在，直接下载
	}//遍历每个文件
 	
	

	


		//delete cjson
		cJSON_Delete(cjson);
		myPtf("===========需要升级的文件数%d==========\n",needupdatecount);
	
	    //表示正在升级程序
	    g_isrunningupdate=0;
	    if(needupdatecount==0)
	    {//没有有需要更新的程序
		if(AUTOSTART_OTHER==1)
		{//自动启动其他模块
			myautostart();	
		}//自动启动其他模块
		startmainservice();
		return 0;
		
	    }//没有需要更新的程序
	    else
	    {//有需要更新的程序
		myPtf("==============执行update.sh================");
		memset(cmd,0,1024);
		sprintf(cmd,"sh %s/update.sh",USER_HOME_DIRECTORY);
		system(cmd);
		return 0;
	    }//有需要更新的程序
}
//线程主函数，定时检查每个进程最近允许访问时间，与系统当前时间比较，如果超出一定值，说明进程已经不存在，需要启动进程
/*void mythreadfun(void)
{
	char bufftime1[256];
	char bufftime2[256];
	char cmd[1024];
	while(1)
	{	
	memset(bufftime1,0,256);

	time_t time_log = time(NULL);
	struct tm* tm_log = localtime(&time_log);
 	sprintf(bufftime1,"%04d-%02d-%02d %02d:%02d:%02d",tm_log->tm_year + 1900, tm_log->tm_mon + 1, tm_log->tm_mday,tm_log->tm_hour, 
		tm_log->tm_min, tm_log->tm_sec);
 	

	myPtf("线程函数开始%s\n,",bufftime1);
	
	myPtf("正在升级标志1%d\n,",g_isrunningupdate);
	myPtf("正在升级标志2%d\n,",g_isrunningupdate);
//	myPtf("%s,%s,%s,%d\n",g_versiontimelist[0].processname,g_versiontimelist[0].updatetime,bufftime1,g_versiontimelist[0].flag);
//	myPtf("%s,%s,%s,%d\n",g_versiontimelist[1].processname,g_versiontimelist[1].updatetime,bufftime1,g_versiontimelist[1].flag);
//	myPtf("%s,%s,%s,%d\n",g_versiontimelist[2].processname,g_versiontimelist[2].updatetime,bufftime1,g_versiontimelist[2].flag);
//	myPtf("%s,%s,%s,%d\n",g_versiontimelist[3].processname,g_versiontimelist[3].updatetime,bufftime1,g_versiontimelist[3].flag);
	//比较时间，判断是否超过一定时间
	long longnow=GetTick(bufftime1);
  	//consumeser
	long long1=GetTick(g_versiontimelist[1].updatetime);

	double fTime1=difftime(longnow,long1);
	
	myPtf("比较consumeser两个时间戳:%ld,%ld，差值%f\n",long1,longnow,fTime1);
		
	//计算时间差的秒数
	int isec1=(int)(fTime1);
	myPtf("差值%d秒\n",isec1);
	if(isec1>=RESTART_DELAY)
	{//超过5秒
		//检查进程是否存在
		char *termname="consumeser";
		int checkflag=checkProcessExist(termname);
		if(checkflag==0)
		{//进程不存在
			myPtf("%s\n","发现进程consumeser不存在，需要启动进程");
			memset(cmd,0,1024);
			sprintf(cmd,"sh %s/startsingle.sh consumeser",USER_HOME_DIRECTORY);
			system(cmd);		
		}//进程不存在	
		else
		{//进程存在
			myPtf("%s\n","发现进程consumeser存在，需要更新存在时间");
			memset(bufftime1,0,256);
			time_t time_log01 = time(NULL);
			struct tm* tm_log01 = localtime(&time_log01);
		 	sprintf(bufftime1,"[%04d-%02d-%02d %02d:%02d:%02d]",tm_log01->tm_year + 1900, tm_log01->tm_mon + 1, tm_log01->tm_mday,tm_log01->tm_hour,tm_log01->tm_min, tm_log01->tm_sec);
			sprintf(g_versiontimelist[1].updatetime,"%s",bufftime1);
		}//进程存在

	}//超过5秒

	time_t time_log3 = time(NULL);
	struct tm* tm_log3 = localtime(&time_log3);
	memset(bufftime2,0,256);
 	sprintf(bufftime2,"%04d-%02d-%02d %02d:%02d:%02d",tm_log3->tm_year + 1900, tm_log3->tm_mon + 1, tm_log3->tm_mday,tm_log3->tm_hour, 
		tm_log3->tm_min, tm_log3->tm_sec);
	

	//scanqrser
	long long3=GetTick(g_versiontimelist[3].updatetime);

	double fTime3=difftime(longnow,long3);
	
	myPtf("比较scanqr两个时间戳:%ld,%ld，差值%f\n",long3,longnow,fTime3);
		
	//计算时间差的秒数
	int isec3=(int)(fTime3);
	myPtf("差值%d秒\n",isec3);
	if(isec3>=RESTART_DELAY)
	{//超过5秒
		myPtf("%s\n","1需要关闭和重启此进程scanqrser");
		//检查进程是否存在
		char *termname="scanqrser";
		int checkflag=checkProcessExist(termname);
		if(checkflag==0)
		{//进程不存在
			myPtf("%s\n","发现进程scanqrser不存在，需要启动进程");
			memset(cmd,0,1024);
			sprintf(cmd,"sh %s/startsingle.sh scanqrser",USER_HOME_DIRECTORY);
			system(cmd);		
		}//进程不存在
		else
		{//进程存在
			myPtf("%s\n","发现进程scanqrser存在，需要更新存在时间");
			memset(bufftime1,0,256);
			time_t time_log03 = time(NULL);
			struct tm* tm_log03 = localtime(&time_log03);
		 	sprintf(bufftime1,"[%04d-%02d-%02d %02d:%02d:%02d]",tm_log03->tm_year + 1900, tm_log03->tm_mon + 1, tm_log03->tm_mday,tm_log03->tm_hour,tm_log03->tm_min, tm_log03->tm_sec);
			sprintf(g_versiontimelist[3].updatetime,"%s",bufftime1);
		}//进程存在
	}//超过5秒

	time_t time_log2 = time(NULL);
	struct tm* tm_log2 = localtime(&time_log2);
	memset(bufftime2,0,256);
 	sprintf(bufftime2,"%04d-%02d-%02d %02d:%02d:%02d",tm_log2->tm_year + 1900, tm_log2->tm_mon + 1, tm_log2->tm_mday,tm_log2->tm_hour, 
		tm_log2->tm_min, tm_log2->tm_sec);
	
	//secscreen
	long long2=GetTick(g_versiontimelist[2].updatetime);

	double fTime2=difftime(longnow,long2);
	
	myPtf("比较secscreen两个时间戳:%ld,%ld，差值%f\n",long2,longnow,fTime2);
		
	//计算时间差的秒数
	int isec2=(int)(fTime2);
	myPtf("差值%d秒\n",isec2);
	if(isec2>=RESTART_DELAY)
	{//超过5秒
		myPtf("%s\n","1需要关闭和重启此进程secscreen");
		//检查进程是否存在
		char *termname="secscreen";
		int checkflag=checkProcessExist(termname);
		if(checkflag==0)
		{//进程不存在
			myPtf("%s\n","发现进程secscreen不存在，需要启动进程");
			memset(cmd,0,1024);
			sprintf(cmd,"sh %s/startsingle.sh secscreen",USER_HOME_DIRECTORY);
			system(cmd);		
		}//进程不存在		
		else
		{//进程存在
			myPtf("%s\n","发现进程secscreen存在，需要更新存在时间");
			memset(bufftime1,0,256);
			time_t time_log02 = time(NULL);
			struct tm* tm_log02 = localtime(&time_log02);
		 	sprintf(bufftime1,"[%04d-%02d-%02d %02d:%02d:%02d]",tm_log02->tm_year + 1900, tm_log02->tm_mon + 1, tm_log02->tm_mday,tm_log02->tm_hour,tm_log02->tm_min, tm_log02->tm_sec);
			sprintf(g_versiontimelist[2].updatetime,"%s",bufftime1);
		}//进程存在


	}//超过5秒

	
	time_t time_log4 = time(NULL);
	struct tm* tm_log4 = localtime(&time_log4);
	memset(bufftime2,0,256);
 	sprintf(bufftime2,"%04d-%02d-%02d %02d:%02d:%02d",tm_log4->tm_year + 1900, tm_log4->tm_mon + 1, tm_log4->tm_mday,tm_log4->tm_hour, 
		tm_log4->tm_min, tm_log4->tm_sec);
	
	//terminalProject
	long long4=GetTick(g_versiontimelist[0].updatetime);

	double fTime4=difftime(longnow,long4);
	
	myPtf("比较terminalProject两个时间戳:%ld,%ld，差值%f\n",long4,longnow,fTime4);
		
	//计算时间差的秒数
	int isec4=(int)(fTime4);
	myPtf("差值%d秒\n",isec4);
	if(isec4>=RESTART_DELAY)
	{//超过5秒
		myPtf("%s\n","1需要关闭和重启此进程terminalProject");
		//检查进程是否存在
		char *termname="terminalProject";
		int checkflag=checkProcessExist(termname);
		if(checkflag==0)
		{//进程不存在
			myPtf("%s\n","发现进程terminalProject不存在，需要启动进程");
			//启动terminalProject应该在weston启动后才能进行
			//可能需要重启weston，此处未实现
			//启动weston /etc/init.d/weston start
			//重启weston /etc/init.d/weston restart
	
			memset(cmd,0,1024);
			sprintf(cmd,"sh %s/startsingle.sh terminalProject",USER_HOME_DIRECTORY);
			system(cmd);		
		}//进程不存在
		else
		{//进程存在
			myPtf("%s\n","发现进程terminalProject存在，需要更新存在时间");
			memset(bufftime1,0,256);
			time_t time_log04 = time(NULL);
			struct tm* tm_log04 = localtime(&time_log04);
		 	sprintf(bufftime1,"[%04d-%02d-%02d %02d:%02d:%02d]",tm_log04->tm_year + 1900, tm_log04->tm_mon + 1, tm_log04->tm_mday,tm_log04->tm_hour,tm_log04->tm_min, tm_log04->tm_sec);
			sprintf(g_versiontimelist[0].updatetime,"%s",bufftime1);
		}//进程存在
		
	}//超过5秒

	time_t time_log5 = time(NULL);
	struct tm* tm_log5 = localtime(&time_log5);
	memset(bufftime2,0,256);
 	sprintf(bufftime2,"%04d-%02d-%02d %02d:%02d:%02d",tm_log5->tm_year + 1900, tm_log5->tm_mon + 1, tm_log5->tm_mday,tm_log5->tm_hour, 
		tm_log5->tm_min, tm_log5->tm_sec);
	
	


	

	memset(bufftime1,0,256);
	time_t time_log1 = time(NULL);
	struct tm* tm_log1 = localtime(&time_log1);
 	sprintf(bufftime1,"[%04d-%02d-%02d %02d:%02d:%02d]",tm_log1->tm_year + 1900, tm_log1->tm_mon + 1, tm_log1->tm_mday,tm_log1->tm_hour, 
		tm_log1->tm_min, tm_log1->tm_sec);
	myPtf("%s线程函数结束\n",bufftime1);
	sleep(60);//单位是秒！！！
	}
	
}*/
//检查进程是否存在
int checkProcessExist(char *processname)
{
	FILE *ptr = NULL;
	char cmd[128] ;
	memset(cmd,0,128);
	sprintf(cmd,"ps -ef | grep %s | grep -v grep | wc -l",processname);
	int status = 0;
	char buf[150];
	int count;
	
	if((ptr = popen(cmd, "r"))==NULL)
	{
		myPtf("popen err\n");	
		return 0;
	}

	memset(buf, 0, sizeof(buf));

	if((fgets(buf, sizeof(buf),ptr))!= NULL)//获取进程和子进程的总数
	{
		count = atoi(buf);
		if(count <= 0)//当进程数小于等于0时，说明进程不存在
		{
			return 0;
		}
		else
		{//大于0，进程存在
			return 1;
		}//大于0，进程存在
			
	}
	fclose(ptr);		
	return 0;	
	
}

int Compute_file_md5(const char *file_path, char *md5_str)
{
	int i;
	int fd;
	int ret;
	unsigned char data[READ_DATA_SIZE];
	unsigned char md5_value[MD5_SIZE];
	MD5_CTX md5;
 
	fd = open(file_path, O_RDONLY);
	if (-1 == fd)
	{
		perror("open");
		return -1;
	}
 
	// init md5
	MD5Init(&md5);
 
	while (1)
	{
		ret = read(fd, data, READ_DATA_SIZE);
		if (-1 == ret)
		{
			perror("read");
			return -1;
		}
 
		MD5Update(&md5, data, ret);
 
		if (0 == ret || ret < READ_DATA_SIZE)
		{
			break;
		}
	}
 
	close(fd);
 
	MD5Final(&md5, md5_value);
 
	for(i = 0; i < MD5_SIZE; i++)
	{
		snprintf(md5_str + i*2, 2+1, "%02X", md5_value[i]);
	}
	md5_str[MD5_STR_LEN] = '\0'; // add end
 
	return 0;
}

/* set advisory lock on file */
int lockfile(int fd)
{
	struct flock fl;
	fl.l_type = F_WRLCK;/* write lock */
	fl.l_start = 0;
	fl.l_whence = SEEK_SET;
	fl.l_len = 0;//lock the whole file
	return(fcntl(fd, F_SETLK, &fl));
}



int already_running(const char *filename)
{
	int fd;
	char buf[16];
	fd = open(filename, O_RDWR | O_CREAT, LOCKMODE);
	if (fd < 0) {
		myPtf("can't open %s: %m\n", filename);
		exit(1);
	}
	/* 先获取文件锁 */
	if (lockfile(fd) == -1) {
		if (errno == EACCES || errno == EAGAIN) {
			myPtf("file: %s already locked\n", filename);
			close(fd);
			return 1;
		}
		myPtf("can't lock %s: %m\n", filename);
		exit(1);
	}
	/* 写入运行实例的pid */
	ftruncate(fd, 0);
	sprintf(buf, "%ld", (long)getpid());
	write(fd, buf, strlen(buf) + 1);
	return 0;
}

//获得硬件唯一识别码
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
		//for(int i=0;i<strlen(newbuf);i++)
		//{
		//	
		//	if(newbuf[i]>96&&newbuf[i]<123)
		//	   {
		//	    newbuf[i]=newbuf[i]-32;
		//	    continue;
		//	   }
		//}		
	}
	fclose(ptr);
	return newbuf;

}



//计算字符串的MD5
int Compute_string_md52(char *string_ori, char *md5_str)
{
	int i;
	
	int ret;
	unsigned char data[READ_DATA_SIZE];
	unsigned char md5_value[MD5_SIZE];
	MD5_CTX md5;
 
	memcpy(data,string_ori,strlen(string_ori));
	// init md5
	MD5Init(&md5);
 	
	
 	ret=strlen(string_ori);
	MD5Update(&md5, data, ret);
 
	
 
	MD5Final(&md5, md5_value);
 
	for(i = 0; i < MD5_SIZE; i++)
	{
		snprintf(md5_str + i*2, 2+1, "%02x", md5_value[i]);
	}
	md5_str[MD5_STR_LEN] = '\0'; // add end
 
	return 0;
}

/*
//返回键盘输入个数
int GetKeyBoardCount(int *size)
{
	char cmd[1024];
	memset(cmd,0,1024);
	sprintf(cmd,"cat /proc/bus/input/devices | grep bus -c");
	char buf[1024];
	memset(buf,0,1024);
	FILE *ptr = NULL;
	if((ptr = popen(cmd, "r"))==NULL)
	{
		
		return -1;
	}

	memset(buf, 0, sizeof(buf));
	if((fgets(buf, sizeof(buf),ptr))!= NULL)//获取进程和子进程的总数
	{
		//myPtf("%s\n",buf);
	
		
	}
	fclose(ptr);
	*size=atoi(buf);
	

	return 1;
}
//返回键盘输入列表
int GetKeyBoardList(int *size)
{
	char cmd[1024];
	memset(cmd,0,1024);
	int i=0;
	int icount=*size;
	FILE *ptr = NULL;
	char buf[1024];
	for(i=0;i<=icount;i++)
	{
		memset(cmd,0,1024);
		sprintf(cmd,"cat /proc/bus/input/devices | grep Bus | sed -n %dp  | awk '{print $2}' | awk -F['='] '{print $2}'",(i+1));
		
		memset(buf,0,1024);
		
		if((ptr = popen(cmd, "r"))==NULL)
		{
			
			continue;
		}

		memset(buf, 0, sizeof(buf));
		if((fgets(buf, sizeof(buf),ptr))!= NULL)//获取进程和子进程的总数
		{
			memset(g_kbd_list[i].bus,0,256);
			sprintf(g_kbd_list[i].bus,"%s",buf);
			//myPtf("%d,bus=%s\n",i,buf);
			
			
		}
		fclose(ptr);
		memset(cmd,0,1024);		
		sprintf(cmd,"cat /proc/bus/input/devices | grep Bus | sed -n %dp  | awk '{print $3}' | awk -F['='] '{print $2}'",(i+1));
		char buf[1024];
		memset(buf,0,1024);
		FILE *ptr = NULL;
		if((ptr = popen(cmd, "r"))==NULL)
		{
			
			continue;
		}

		memset(buf, 0, sizeof(buf));
		if((fgets(buf, sizeof(buf),ptr))!= NULL)//获取进程和子进程的总数
		{
			memset(g_kbd_list[i].vendor,0,256);
			sprintf(g_kbd_list[i].vendor,"%s",buf);
			//myPtf("%d,vendor=%s\n",i,buf);
			
			
		}
		fclose(ptr);
		memset(cmd,0,1024);		
		sprintf(cmd,"cat /proc/bus/input/devices | grep Bus | sed -n %dp  | awk '{print $4}' | awk -F['='] '{print $2}'",(i+1));
		
		memset(buf,0,1024);
		
		if((ptr = popen(cmd, "r"))==NULL)
		{
			
			continue;
		}

		memset(buf, 0, sizeof(buf));
		if((fgets(buf, sizeof(buf),ptr))!= NULL)//获取进程和子进程的总数
		{
			memset(g_kbd_list[i].product,0,256);
			sprintf(g_kbd_list[i].product,"%s",buf);
			//myPtf("%d,product=%s\n",i,buf);
			
			
		}
		fclose(ptr);
		memset(cmd,0,1024);		
		sprintf(cmd,"cat /proc/bus/input/devices | grep Bus | sed -n %dp  | awk '{print $5}' | awk -F['='] '{print $2}'",(i+1));
		
		memset(buf,0,1024);
		
		if((ptr = popen(cmd, "r"))==NULL)
		{
			
			continue;
		}

		memset(buf, 0, sizeof(buf));
		if((fgets(buf, sizeof(buf),ptr))!= NULL)//获取进程和子进程的总数
		{
			memset(g_kbd_list[i].version,0,256);
			sprintf(g_kbd_list[i].version,"%s",buf);
			//myPtf("%d,version=%s\n",i,buf);
			
			
		}
		fclose(ptr);
		memset(cmd,0,1024);		
		sprintf(cmd,"cat /proc/bus/input/devices | grep Name | sed -n %dp  | awk '{print $2}' | awk -F['='] '{print $2}'",(i+1));
		
		memset(buf,0,1024);
		
		if((ptr = popen(cmd, "r"))==NULL)
		{
			
			continue;
		}

		memset(buf, 0, sizeof(buf));
		if((fgets(buf, sizeof(buf),ptr))!= NULL)//获取进程和子进程的总数
		{
			
			memset(g_kbd_list[i].name,0,256);
			
			sprintf(g_kbd_list[i].name,"%s",&buf[1]);
			if(g_kbd_list[i].name,g_kbd_list[i].name[strlen(g_kbd_list[i].name)-2]=='"')
			{
				g_kbd_list[i].name,g_kbd_list[i].name[strlen(g_kbd_list[i].name)-2]='\0';
			}
			//myPtf("%d,name=%s\n",i,g_kbd_list[i].name);
			
			
		}
		fclose(ptr);
		memset(cmd,0,1024);		
		sprintf(cmd,"cat /proc/bus/input/devices | grep Phys | sed -n %dp  | awk '{print $2}' | awk -F['='] '{print $2}'",(i+1));
		
		memset(buf,0,1024);
		
		if((ptr = popen(cmd, "r"))==NULL)
		{
			
			continue;
		}

		memset(buf, 0, sizeof(buf));
		if((fgets(buf, sizeof(buf),ptr))!= NULL)//获取进程和子进程的总数
		{
			memset(g_kbd_list[i].phys,0,1024);
			sprintf(g_kbd_list[i].phys,"%s",buf);
			
			//myPtf("%d,Phys=%s,phys=%s\n",i,buf,g_kbd_list[i].phys);
			
			
		}
		fclose(ptr);
		memset(cmd,0,1024);		
		sprintf(cmd,"cat /proc/bus/input/devices | grep Sysfs | sed -n %dp  | awk '{print $2}' | awk -F['='] '{print $2}'",(i+1));
		
		memset(buf,0,1024);
		
		if((ptr = popen(cmd, "r"))==NULL)
		{
			
			continue;
		}

		memset(buf, 0, sizeof(buf));
		if((fgets(buf, sizeof(buf),ptr))!= NULL)//获取进程和子进程的总数
		{
			memset(g_kbd_list[i].sysfs,0,1024);
			sprintf(g_kbd_list[i].sysfs,"%s",buf);
			
			//myPtf("%d,Phys=%s,phys=%s\n",i,buf,g_kbd_list[i].phys);
			
			
		}
		fclose(ptr);


	}
	return 0;
}*/
