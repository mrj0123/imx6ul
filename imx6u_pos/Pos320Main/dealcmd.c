

#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "dealcmd.h"

#include <sys/time.h>
#include <ctype.h>

#include "common.h"

#include "log.h"
#include "cJSON.h"
#include "dispose.h"
#include "a.h"

#include "md5.h"

#define READ_DATA_SIZE	1024
#define MD5_SIZE		16
#define MD5_STR_LEN		(MD5_SIZE * 2)



#include <sys/stat.h>


#include <sys/sysinfo.h>

#include "inifile.h"
#include "nksocket.h"
#include "base64.h"

//计算字符串的md5
int Compute_string_md5(char *string_ori, char *md5_str);

int file_size2(char* filename)
{
  struct stat statbuf;
  stat(filename,&statbuf);
  int size=statbuf.st_size;

  return size;
}



//获得网卡状态，依据：ifconfig -a 查找 RUNNING 有RUNNING 表示连接，没有RUNNING表示没有连接
int GetNetStat( char *devicename){
	char    buffer[BUFSIZ];
	FILE    *read_fp;
	int        chars_read;
	int        ret;
	memset( buffer, 0, BUFSIZ );
	char szfilename[200];
	sprintf(szfilename,"ifconfig %s | grep RUNNING",devicename);
	read_fp = popen(szfilename, "r");
	if ( read_fp != NULL )
	{
		chars_read = fread(buffer, sizeof(char), BUFSIZ-1, read_fp);
		if (chars_read > 0)
		{
			ret = 1;
		}
		else
		{
			ret = -1;
		}
		pclose(read_fp);
	}
	else
	{
		ret = -1;
	}
	return ret;
}

//	int i=0;
//    i = GetNetStat();
//    myPtf( "\nNetStat = %d\n", i


//处理函数 获得硬件序列号
char * fn_dealCmd_GetHardwareSerialNo(char * recv_buf,int recv_len,int * ret_len)
{
	write_log_s("%s\n", "进入处理函数fn_dealCmd_GetHardwareSerialNo");

	char *buf=NULL;

	buf=(char *)malloc(256);
	memset(buf,0,256);

	char bufftime1[256];
	memset(bufftime1,0,256);

	time_t time_log = time(NULL);
	struct tm* tm_log = localtime(&time_log);
 	sprintf(bufftime1,"%04d_%02d_%02d_%02d_%02d_%02d_s1",tm_log->tm_year + 1900, tm_log->tm_mon + 1, tm_log->tm_mday,tm_log->tm_hour,
tm_log->tm_min, tm_log->tm_sec);
	bufftime1[22]='\0';
	char cmd[1024];
	memset(cmd,0,1024);
        sprintf(cmd, "echo %s >%s/1.txt",bufftime1,USER_TEMP_DIRECTORY );
        system(cmd);
	memset(cmd,0,1024);
        sprintf(cmd, "chmod 777 %s/1.txt",USER_TEMP_DIRECTORY );
        system(cmd);

	memset(cmd,0,1024);
        sprintf(cmd, "echo %s >> %s/1.txt","\n" ,USER_TEMP_DIRECTORY);
        system(cmd);

	memset(cmd,0,1024);
        sprintf(cmd, "cat /sys/fsl_otp/HW_OCOTP_CFG1  >> %s/1.txt",USER_TEMP_DIRECTORY);
        system(cmd);


	memset(cmd,0,1024);
	sprintf(cmd, "echo %s >>  %s/1.txt","\n",USER_TEMP_DIRECTORY );
        system(cmd);
	char szfilename[200];
	memset(szfilename,0,200);
	sprintf(szfilename,"%s/1.txt",USER_TEMP_DIRECTORY);
	char *newbuf;
	newbuf=(char *)malloc(256);
	memset(newbuf,0,256);

	FILE *fp = fopen(szfilename, "r");
	if(fp == NULL)
	{
		//释放内存
		free(buf);
		buf=NULL;
		fclose(fp);
		//myPtf("打开文件错误");
		sprintf(newbuf,"{\"serialno\":\"%s\"}","");
		*ret_len=strlen(newbuf);
		return newbuf;
	}
	char line[256];
	int len;
	memset(line,0,256);
	fgets(line, sizeof(line), fp);
	sprintf(buf,"%s",line);


	buf[22]='\0';
	char *szbuf=NULL;
	szbuf=(char *)malloc(256);
	memset(szbuf,0,256);
	write_log_s("buf:%s,%d\n", buf,strlen(buf));
	write_log_s("bufftime1:%s,%d\n", bufftime1,strlen(bufftime1));
	if(strcmp(buf, bufftime1) == 0)
	{//如果相等，说明是同一个文件
		write_log_s(" %s\n ", " 相等 ");
		memset(line,0,256);
		fgets(line, sizeof(line), fp);

		memset(buf,0,256);
		sprintf(buf,"%s",&line[2]);//从第三个字符开始

	}//如果相等，说明是同一个文件




	fclose(fp);

	//删除临时文件
	memset(cmd,0,1024);
        sprintf(cmd, "rm -rf %s",szfilename);
        system(cmd);

	write_log_s("%s\n", "离开处理函数fn_dealCmd_GetHardwareSerialNo");

	//int iret=mytrim(buf);
	buf[strlen(buf)-1]='\0';



//	write_log_s("硬件唯一识别码2:%s,%d\n", szbuf,strlen(szbuf));

	sprintf(newbuf,"{\"serialno\":\"%s\"}",buf);
	//释放内存
	free(szbuf);
	szbuf=NULL;

	//释放内存
	free(buf);
	buf=NULL;


//	myPtf("%s,%s,%s",szbuf,buf,newbuf);
	//write_log_s("最后结果:%s,%d,哈哈哈\n", newbuf,strlen(newbuf));
	*ret_len=strlen(newbuf);
	return newbuf;
}

//命令处理函数，获得网络连接状态
char * fn_dealCmd_GetNetworkConnectStatus(char * recv_buf,int recv_len,int * ret_len)
{
/*
数据格式:{“netState”:”1”,“networkType”:”1”,“powerrange”:”15”,
” serverState”:1，serialno”:”nktyterm0001”}
netState:1—连接，0—未连接
networkType:1—有线连接，2—wifi连接，3—4G
powerrange:15,20,30

*/
	write_log_s("%s\n", "进入处理函数fn_dealCmd_GetNetworkConnectStatus");


	char *buf=NULL;

	buf=(char *)malloc(256);
	memset(buf,0,256);

	char bufftime1[256];
	memset(bufftime1,0,256);

	time_t time_log = time(NULL);
	struct tm* tm_log = localtime(&time_log);
 	sprintf(bufftime1,"%04d_%02d_%02d_%02d_%02d_%02d_s1",tm_log->tm_year + 1900, tm_log->tm_mon + 1, tm_log->tm_mday,tm_log->tm_hour,
tm_log->tm_min, tm_log->tm_sec);
	bufftime1[22]='\0';
	char cmd[1024];
	memset(cmd,0,1024);
	//生成时间戳，写入文件
        sprintf(cmd, "echo %s > %s/1.txt",bufftime1,USER_TEMP_DIRECTORY);
        system(cmd);
	memset(cmd,0,1024);
        sprintf(cmd, "echo %s >>%s/1.txt","\n", USER_TEMP_DIRECTORY);
        system(cmd);

	//确认当前有线网卡是否有效
	//先得到有线网卡的IP
	memset(cmd,0,1024);
	sprintf(cmd, "sh %s/getipaddress.sh >> %s/1.txt",USER_SCRIPT_DIRECTORY,USER_TEMP_DIRECTORY);
	system(cmd);

	memset(cmd,0,1024);
        sprintf(cmd, "echo %s >>%s/1.txt","\n", USER_TEMP_DIRECTORY);
        system(cmd);


	//第二步得到网关地址
	memset(cmd,0,1024);
	sprintf(cmd, "sh %s/getgateway.sh >> %s/1.txt",USER_SCRIPT_DIRECTORY,USER_TEMP_DIRECTORY);
	system(cmd);

	memset(cmd,0,1024);
        sprintf(cmd, "echo %s >>%s/1.txt","\n", USER_TEMP_DIRECTORY);
        system(cmd);


	//第四步得到服务器1地址
	memset(cmd,0,1024);
	sprintf(cmd, "%s/getserverip.sh 1 >> %s/1.txt",USER_SCRIPT_DIRECTORY,USER_TEMP_DIRECTORY);
	system(cmd);
	memset(cmd,0,1024);
	sprintf(cmd, "echo %s >>%s/1.txt","\n" ,USER_TEMP_DIRECTORY);
	system(cmd);

	//第四步得到服务器1地址
	memset(cmd,0,1024);
	sprintf(cmd, "sh %s/getserverport.sh 1 >> %s/1.txt",USER_SCRIPT_DIRECTORY,USER_TEMP_DIRECTORY);
	system(cmd);
	memset(cmd,0,1024);
	sprintf(cmd, "echo %s >> %s/1.txt","\n",USER_TEMP_DIRECTORY );
	system(cmd);

	//第四步得到服务器1地址
	memset(cmd,0,1024);
	sprintf(cmd, "sh %s/getserverip.sh 2 >> %s/1.txt",USER_SCRIPT_DIRECTORY,USER_TEMP_DIRECTORY);
	system(cmd);
	memset(cmd,0,1024);
	sprintf(cmd, "echo %s >> %s/1.txt","\n" ,USER_TEMP_DIRECTORY);
	system(cmd);
	//第四步得到服务器1地址
	memset(cmd,0,1024);
	sprintf(cmd, "sh %s/getserverport.sh 2 >> %s/1.txt",USER_SCRIPT_DIRECTORY,USER_TEMP_DIRECTORY);
	system(cmd);
	memset(cmd,0,1024);
	sprintf(cmd, "echo %s >> %s/1.txt","\n",USER_TEMP_DIRECTORY );
	system(cmd);

	//打开文件sh /etc/nkty/getserverip.sh 1
	char szfilename[200];
	memset(szfilename,0,200);
	sprintf(szfilename,"%s/1.txt",USER_TEMP_DIRECTORY);

	char *newbuf;
	newbuf=(char *)malloc(1024);
	memset(newbuf,0,1024);


	FILE *fp = fopen(szfilename, "r");
	if(fp == NULL)
	{
		fclose(fp);
		//myPtf("打开文件错误");
		sprintf(newbuf,"{\"netState\":\"%d\",\"networkType\":\"%d\",\"powerrange\":\"%d\",\"serverState\":\"%d\"}",
		0,1,0,0);
		*ret_len=strlen(newbuf);
		return newbuf;
	}
	char line[256];
	char line2[256];//网关地址
	char line3[256];//硬件序列号
	char line4[256];//服务器地址
	char line5[256];//服务器端口

	char line6[256];//服务器地址
	char line7[256];//服务器端口


	int len;

	memset(line,0,256);
	fgets(line, sizeof(line), fp);
	sprintf(buf,"%s",line);

	int networkType=1;//网络类型,1为有线、2为无线、3为4G
        int netState=0;//网络状态
	int powerrange=0;
	int serverState=0;//服务器状态



	buf[22]='\0';
	char *szbuf=NULL;
	szbuf=(char *)malloc(256);
	memset(szbuf,0,256);

	write_log_s("buf:%s,%d\n", buf,strlen(buf));
	write_log_s("bufftime1:%s,%d\n", bufftime1,strlen(bufftime1));
	if(strcmp(buf, bufftime1) == 0)
	{//如果相等，说明是同一个文件
		write_log_s("%s\n", "相等");
		memset(line,0,256);
		fgets(line, sizeof(line), fp);//ip地址
		line[strlen(line)-1]='\0';//去掉fgets自动增加的换行符
		memset(line2,0,256);
		fgets(line2, sizeof(line2), fp);//网关
		line2[strlen(line2)-1]='\0';//去掉fgets自动增加的换行符
		//memset(line3,0,256);
		//fgets(line3, sizeof(line3), fp);//硬件序列号
		//line3[strlen(line3)-1]='\0';//去掉fgets自动增加的换行符

		memset(line4,0,256);
		fgets(line4, sizeof(line4), fp);//服务器IP

		line4[strlen(line4)-1]='\0';//去掉fgets自动增加的换行符

		memset(line5,0,256);


		fgets(line5, sizeof(line5), fp);//硬件序列号
		line5[strlen(line5)-1]='\0';//去掉fgets自动增加的换行符


		memset(line6,0,256);
		fgets(line6, sizeof(line6), fp);//硬件序列号
		line6[strlen(line6)-1]='\0';//去掉fgets自动增加的换行符


		memset(line7,0,256);
		fgets(line7, sizeof(line7), fp);//硬件序列号
		line7[strlen(line7)-1]='\0';//去掉fgets自动增加的换行符



		write_log_s("line=%s,line2=%s\n", line,line2);
		write_log_s("line3=%s,line4=%s\n", line3,line4);
		write_log_s("line5=%s,line6=%s\n", line5,line6);
		write_log_s("line7=%s\n", line7);

		//第三部判断网关是否可用

		sprintf(cmd ,"ping -c 1 %s | grep \"1 packets transmitted, 1 received\" > %s/2.txt",line2,USER_TEMP_DIRECTORY);
		system(cmd);
		write_log_s("%s\n", cmd);
		unsigned long filesize=0;
		 struct stat sizebuf;
		char szfilename[200];
		memset(szfilename,0,200);
		sprintf(szfilename,"%s/2.txt",USER_TEMP_DIRECTORY);


		if(stat(szfilename, &sizebuf)<0)
	    	{ //
			filesize=0;
	    	}
		else
		{
			filesize=(unsigned long)sizebuf.st_size;
			if(filesize>0)
			{

			}

		}
		write_log_s("文件大小filesize:%d\n", filesize);
		write_log_s("==========%s,%s==========\n", line4,line5);
		sprintf(szfilename,"rm -rf %s/2.txt",USER_TEMP_DIRECTORY);
		system(szfilename);
		if(filesize>0)
		{
			netState=1;
		}
		else
		{
			netState=0;
		}
		serverState=0;
		/*memset(cmd,0,1024);
		sprintf(cmd ,"ping -c 1 %s | grep \"1 packets transmitted, 1 received\"> %s/2.txt",line4,USER_TEMP_DIRECTORY);
		system(cmd);
		write_log_s("%s\n", cmd);
		unsigned long filesize2=0;
		 struct stat sizebuf2;
		memset(szfilename,0,200);
		sprintf(szfilename,"%s/2.txt",USER_TEMP_DIRECTORY);
		if(stat(szfilename, &sizebuf2)<0)
	    	{ //
			filesize2=0;
	    	}
		else
		{
			filesize2=(unsigned long)sizebuf2.st_size;
			if(filesize2>0)
			{

			}
		}
		write_log_s("文件大小filesize2:%d\n", filesize2);
		sprintf(szfilename,"rm -rf %s/2.txt",USER_TEMP_DIRECTORY);
		system(szfilename);
		if(filesize2>0)
		{
			serverState=0;
			//判断端口是否是否开放
			memset(cmd,0,1024);
			sprintf(cmd ,"echo -e \"\n\"|telnet %s %s | grep Connected> %s/2.txt",line4,line5,USER_TEMP_DIRECTORY);
			system(cmd);
			write_log_s("%s\n", cmd);
			unsigned long filesize3=0;
			 struct stat sizebuf3;
			memset(szfilename,0,200);
			sprintf(szfilename,"%s/2.txt",USER_TEMP_DIRECTORY);
			if(stat(szfilename, &sizebuf3)<0)
		    	{ //
				filesize3=0;
		    	}
			else
			{
				filesize3=(unsigned long)sizebuf3.st_size;
			}
			write_log_s("文件大小filesize3:%d\n", filesize3);
			sprintf(szfilename,"rm -rf %s/2.txt",USER_TEMP_DIRECTORY);
			system(szfilename);
			if(filesize3==0)
			{//文件长度为0，表示端口是开放的
				serverState=1;
			}//文件长度为0，表示端口是开放的
			else
			{//文件长度不为0，表示端口不开放
				serverState=0;
			}//文件长度不为0，表示端口不开放

		}
		else
		{
			serverState=0;
		}*/


	}//如果相等，说明是同一个文件

	fclose(fp);




	//write_log_s("%s,%ld\n", "离开处理函数fn_dealCmd_GetNetworkConnectStatus",filesize);

//	if(filesize>0)
//	{
//		sprintf(newbuf,"{\"networkstatus\":\"%s\"}","ok");
//	}
//	else
//	{
//		sprintf(newbuf,"{\"networkstatus\":\"%s\"}","error");
//	}
	//网络类型

	//sprintf(newbuf,"{\"netState\":\"%d\",\"networkType\":\"%d\",\"powerrange\":\"%d\",\"serverState\":\"%d\",\"serialno\":\"%s\"}",
	//netState,networkType,powerrange,serverState,&line3[2]);

	char devicename[200];
	memset(devicename,0,200);
	sprintf(devicename,"%s","eth0");
	int checkflag=GetNetStat(devicename);

	memset(devicename,0,200);
	sprintf(devicename,"%s","wlan0");
	int checkflag2=GetNetStat(devicename);
	write_log_s("执行:GetNetStat：%d,%d\n", checkflag,checkflag2);
	if((checkflag==-1)&&(checkflag2==1))
	{
		networkType=2;
	}
	else if((checkflag==1)&&(checkflag2==-1))
	{
		networkType=1;
	}
	sprintf(newbuf,"{\"netState\":\"%d\",\"networkType\":\"%d\",\"powerrange\":\"%d\",\"serverState\":\"%d\"}",
	netState,networkType,powerrange,serverState);


	//删除临时文件
	sprintf(szfilename,"rm -rf %s/1.txt",USER_TEMP_DIRECTORY);
	system(szfilename);

	free(szbuf);
	szbuf=NULL;

	free(buf);
	buf=NULL;

	//system("rm -rf 2.txt");

//	myPtf("%s,%s,%s",szbuf,buf,newbuf);
	write_log_s("最后结果:%s,%d,哈哈哈\n", newbuf,strlen(newbuf));
	*ret_len=strlen(newbuf);
	return newbuf;
}


//命令处理函数，设置网络参数
char * fn_dealCmd_SetNetWorkSetting(char * recv_buf,int recv_len,int * ret_len)
{
	write_log_s("%s\n", "进入处理函数fn_dealCmd_SetNetWorkSetting");
	char *recv_data=(char *)malloc(recv_len+1);
    	memset(recv_data,0,recv_len+1);
    	memcpy(recv_data,recv_buf,recv_len);

	write_log_s("%s,%s,%d\n", "收到数据",recv_data,recv_len);
	//开始对收到的json进行解释
	//{"ipaddress":"10.100.3.1","netmask":"255.255.255.0","gateway":"10.100.2.254","dns1":"10.100.1.1","dns2":"10.100.1.1","flag":"1","usedhcp":"1"}


	cJSON* cjson = cJSON_Parse(recv_data);
	if(cjson == NULL){

		write_log_s("%s\n","json pack into cjson error...");
	}

	write_log_s("%s\n","json pack into cjson ok...");

	//{"ipaddress":"10.1.70.46","netmask":"255.255.255.0","gateway":"10.1.70.254","dns1":"10.100.1.1","dns2":"202.99.96.68","flag":"1","usedhcp":"1"}
	//收到数据,{"dns1":"10.100.1.1","dns2":"8.8.8.8","gateway":"10.1.70.254","ipaddress":"10.1.70.90","netmask":"255.255.255.0"},113
	char* test_1_string = (char*)cJSON_GetObjectItem(cjson,"ipaddress")->valuestring;//获取键值内容

	char* test_2_string = (char*)cJSON_GetObjectItem(cjson,"netmask")->valuestring;//获取键值内容
	char* test_3_string = (char*)cJSON_GetObjectItem(cjson,"gateway")->valuestring;//获取键值内容
	char* test_4_string = (char*)cJSON_GetObjectItem(cjson,"dns1")->valuestring;//获取键值内容，dns1
	char* test_5_string = (char*)cJSON_GetObjectItem(cjson,"dns2")->valuestring;//获取键值内容,dns2

	int iflag1 = (int)cJSON_GetObjectItem(cjson,"flag")->valueint;//获取键值内容,flag 1启用网络 0禁用网络
	int iflag2 = (int)cJSON_GetObjectItem(cjson,"usedhcp")->valueint;//获取键值内容,usedhcp 1启用dhcp 0静态IP





	char cmd[1024];

	/*if(iflag1==1)
	{//启用有线网络，禁用无线网络

		memset(cmd,0,1024);
		sprintf(cmd,"sh %s/startwired.sh",USER_SCRIPT_DIRECTORY);
		system(cmd);

	}//启用有线网络，禁用无线网络
	else
	{//禁用有线网络，启用无线网络
		memset(cmd,0,1024);
		sprintf(cmd,"sh %s/startwireless.sh",USER_SCRIPT_DIRECTORY);
		system(cmd);

	}//启用有线网络，启用无线网络
*/
	if(iflag2==1)
	{//启用DHCP，禁用静态IP

		memset(cmd,0,1024);
		sprintf(cmd,"sh %s/startdhcp.sh %s",USER_SCRIPT_DIRECTORY,test_1_string);
		system(cmd);

	}//启用DHCP，禁用静态IP
	else
	{//禁用DHCP，启用静态IP
		memset(cmd,0,1024);
		sprintf(cmd,"sh %s/startstatic.sh",USER_SCRIPT_DIRECTORY);
		system(cmd);

		memset(cmd,0,1024);
		sprintf(cmd,"sh %s/changeip.sh %s",USER_SCRIPT_DIRECTORY,test_1_string);
		system(cmd);


		memset(cmd,0,1024);
		sprintf(cmd,"sh %s/changenetmask.sh %s",USER_SCRIPT_DIRECTORY,test_2_string);
		system(cmd);

		memset(cmd,0,1024);
		sprintf(cmd,"sh %s/changegateway.sh %s",USER_SCRIPT_DIRECTORY,test_3_string);
		system(cmd);


		memset(cmd,0,1024);
		sprintf(cmd,"sh %s/changedns.sh %s %s",USER_SCRIPT_DIRECTORY,test_4_string,test_5_string);
		system(cmd);
		if(iflag1==1)
		{//启用有线网络，禁用无线网络

			memset(cmd,0,1024);
			sprintf(cmd,"sh %s/startwired.sh",USER_SCRIPT_DIRECTORY);
			system(cmd);

		}//启用有线网络，禁用无线网络
		else
		{//禁用有线网络，启用无线网络
			memset(cmd,0,1024);
			sprintf(cmd,"sh %s/startwireless.sh",USER_SCRIPT_DIRECTORY);
			system(cmd);

		}//启用有线网络，启用无线网络

	}//禁用DHCP，启用静态IP

	memset(cmd,0,1024);
	sprintf(cmd,"ifdown eth0 ");
	system(cmd);
	memset(cmd,0,1024);
	sprintf(cmd,"ifup eth0 ");
	system(cmd);

	//delete cjson
	cJSON_Delete(cjson);








	write_log_s("%s\n", "离开处理函数fn_dealCmd_SetNetWorkSetting");
	char *newbuf;
	newbuf=(char *)malloc(32);
	memset(newbuf,0,32);
	sprintf(newbuf,"%d",NETWORK_INFOMATION_SET_CMD);
	*ret_len=strlen(newbuf);

	//释放内存
	free(recv_data);
	recv_data=NULL;



	return newbuf;
}


//命令处理函数，获得网络参数
char * fn_dealCmd_GetNetWorkSetting(char * recv_buf,int recv_len,int * ret_len)
{
	write_log_s("%s\n", "进入处理函数fn_dealCmd_GetNetWorkSetting");

	char *newbuf;
	newbuf=(char *)malloc(1024);

	char line[256];
	char line1[256];//IP地址
	char line2[256];//子网掩码
	char line3[256];//网关
	char line4[256];//Mac地址
	char line5[256];//DNS1
	char line6[256];//DNS2
	char line7[256];//netstatus
	char line8[256];//dhcp

	memset(line1,0,256);
	memset(line2,0,256);
	memset(line3,0,256);
	memset(line4,0,256);
	memset(line5,0,256);
	memset(line6,0,256);
	memset(line7,0,256);
	memset(line8,0,256);


	char cmd[1024];

	char myresult[1024];
	memset(myresult,0,1024);
	char szcmd[1024];
	memset(szcmd,0,1024);
	sprintf(szcmd, "sh %s/getipaddress.sh",USER_SCRIPT_DIRECTORY);
	memset(myresult,0,1024);
	int ret=GetCommandReturnValue(&szcmd[0],&myresult[0]);
	sprintf(line1,myresult);


	memset(szcmd,0,1024);
	sprintf(szcmd, "sh %s/getnetmask.sh",USER_SCRIPT_DIRECTORY);
	memset(myresult,0,1024);
	ret=GetCommandReturnValue(&szcmd[0],&myresult[0]);
	sprintf(line2,myresult);


	memset(szcmd,0,1024);
	sprintf(szcmd, "sh %s/getgateway.sh",USER_SCRIPT_DIRECTORY);
	memset(myresult,0,1024);
	ret=GetCommandReturnValue(&szcmd[0],&myresult[0]);
	sprintf(line3,myresult);

	memset(szcmd,0,1024);
	sprintf(szcmd, "sh %s/getmac.sh eth0",USER_SCRIPT_DIRECTORY);
	memset(myresult,0,1024);
	ret=GetCommandReturnValue(&szcmd[0],&myresult[0]);
	sprintf(line4,myresult);


	memset(szcmd,0,1024);
	sprintf(szcmd, "sh %s/getnetstatus.sh",USER_SCRIPT_DIRECTORY);
	memset(myresult,0,1024);
	ret=GetCommandReturnValue(&szcmd[0],&myresult[0]);
	sprintf(line7,myresult);



	memset(szcmd,0,1024);
	sprintf(szcmd, "sh %s/getdhcpflag.sh 1",USER_SCRIPT_DIRECTORY);
	memset(myresult,0,1024);
	ret=GetCommandReturnValue(&szcmd[0],&myresult[0]);
	sprintf(line8,myresult);





	//第五步得到DNS
	memset(cmd,0,1024);
	sprintf(cmd, "sh %s/getdns.sh ",USER_SCRIPT_DIRECTORY);//此脚本自己生成dns.txt
	system(cmd);

	char szfilename[200];
	memset(szfilename,0,200);
	sprintf(szfilename,"%s/dns.txt",USER_TEMP_DIRECTORY);

	//打开文件
	FILE *fp1 = fopen(szfilename, "r");
	if(fp1 != NULL)
	{
		memset(line5,0,256);
		fgets(line5, sizeof(line5), fp1);
		line5[strlen(line5)-1]='\0';//去掉fgets自动增加的换行符
		memset(line6,0,256);
		fgets(line6, sizeof(line6), fp1);
		line6[strlen(line6)-1]='\0';//去掉fgets自动增加的换行符
	}
	fclose(fp1);

	memset(szfilename,0,200);
	sprintf(szfilename,"rm -rf %s/dns.txt",USER_TEMP_DIRECTORY);
	system(szfilename);




	memset(newbuf,0,1024);
	sprintf(newbuf,"{\"ipaddress\":\"%s\",\"netmask\":\"%s\",\"gateway\":\"%s\",\"mac\":\"%s\",\"dns1\":\"%s\",\"dns2\":\"%s\",\"flag\":\"%s\",\"usedhcp\":\"%s\"}",line1,line2,line3,line4,line5,line6,line7,line8);

	*ret_len=strlen(newbuf);


//	myPtf("%s,%s,%s",szbuf,buf,newbuf);
	write_log_s("最后结果:%s,%d,哈哈哈\n", newbuf,*ret_len);

	/*char *buf=NULL;

	buf=(char *)malloc(1024);
	memset(buf,0,1024);

	char bufftime1[256];
	memset(bufftime1,0,256);


	time_t time_log = time(NULL);
	struct tm* tm_log = localtime(&time_log);
 	sprintf(bufftime1,"%04d_%02d_%02d_%02d_%02d_%02d_s1",tm_log->tm_year + 1900, tm_log->tm_mon + 1, tm_log->tm_mday,tm_log->tm_hour,

tm_log->tm_min, tm_log->tm_sec);
	bufftime1[22]='\0';
	char cmd[1024];
	//生成时间戳，写入文件
	memset(cmd,0,1024);
        sprintf(cmd, "echo %s > %s/1.txt",bufftime1,USER_TEMP_DIRECTORY );
        system(cmd);
	memset(cmd,0,1024);
	sprintf(cmd, "echo %s >> %s/1.txt","\n",USER_TEMP_DIRECTORY );
        system(cmd);

	//确认当前有线网卡是否有效
	//先得到有线网卡的IP
	memset(cmd,0,1024);
	sprintf(cmd, "sh %s/getipaddress.sh >> %s/1.txt",USER_SCRIPT_DIRECTORY,USER_TEMP_DIRECTORY);
	system(cmd);
	memset(cmd,0,1024);
        sprintf(cmd, "echo %s >> %s/1.txt","\n",USER_TEMP_DIRECTORY );
        system(cmd);

	//第二步得到子网掩码
	memset(cmd,0,1024);
	sprintf(cmd, "sh %s/getnetmask.sh>> %s/1.txt",USER_SCRIPT_DIRECTORY,USER_TEMP_DIRECTORY);
	system(cmd);
	memset(cmd,0,1024);
        sprintf(cmd, "echo %s >> %s/1.txt","\n",USER_TEMP_DIRECTORY );
        system(cmd);

	//第三步得到网关地址
	memset(cmd,0,1024);
	sprintf(cmd, "sh %s/getgateway.sh >> %s/1.txt",USER_SCRIPT_DIRECTORY,USER_TEMP_DIRECTORY);
	system(cmd);
	memset(cmd,0,1024);
        sprintf(cmd, "echo %s >> %s/1.txt","\n",USER_TEMP_DIRECTORY );
        system(cmd);

	//第四步得到MAC地址
	memset(cmd,0,1024);
	sprintf(cmd, "sh %s/getmac.sh eth0 >> %s/1.txt",USER_SCRIPT_DIRECTORY,USER_TEMP_DIRECTORY);
	system(cmd);
	memset(cmd,0,1024);
        sprintf(cmd, "echo %s >> %s/1.txt","\n",USER_TEMP_DIRECTORY );
        system(cmd);


	memset(cmd,0,1024);
       sprintf(cmd, "sh %s/getnetstatus.sh >> %s/1.txt",USER_SCRIPT_DIRECTORY,USER_TEMP_DIRECTORY);
        system(cmd);

	memset(cmd,0,1024);
        sprintf(cmd, "echo %s >> %s/1.txt","\n",USER_TEMP_DIRECTORY );
        system(cmd);


	memset(cmd,0,1024);
       sprintf(cmd, "sh %s/getdhcpflag.sh 1 >> %s/1.txt",USER_SCRIPT_DIRECTORY,USER_TEMP_DIRECTORY);
        system(cmd);

	memset(cmd,0,1024);
        sprintf(cmd, "echo %s >> %s/1.txt","\n",USER_TEMP_DIRECTORY );
        system(cmd);


	//第五步得到DNS
	memset(cmd,0,1024);
	sprintf(cmd, "sh %s/getdns.sh ",USER_SCRIPT_DIRECTORY);//此脚本自己生成dns.txt
	system(cmd);

	char szfilename[200];
	memset(szfilename,0,200);
	sprintf(szfilename,"%s/1.txt",USER_TEMP_DIRECTORY);




	//打开文件
	FILE *fp = fopen(szfilename, "r");
	if(fp == NULL)
	{
		fclose(fp);
		//myPtf("打开文件错误");
		memset(newbuf,0,1024);
		sprintf(newbuf,"{\"ipaddress\":\"%s\",\"netmask\":\"%s\",\"gateway\":\"%s\",\"dns1\":\"%s\",\"dns2\":\"%s\",\"mac\":\"%s\",\"flag\":\"%s\",\"usedhcp\":\"%s\"}","","","","","","","0","0");
		*ret_len=strlen(newbuf);
		return newbuf;

	}
	char line[256];
	char line1[256];//IP地址
	char line2[256];//子网掩码
	char line3[256];//网关
	char line4[256];//Mac地址
	char line5[256];//DNS1
	char line6[256];//DNS2
	char line7[256];//netstatus
	char line8[256];//dhcp

	memset(line1,0,256);
	memset(line2,0,256);
	memset(line3,0,256);
	memset(line4,0,256);
	memset(line5,0,256);
	memset(line6,0,256);
	memset(line7,0,256);
	memset(line8,0,256);
	int len;

	memset(line,0,256);
	fgets(line, sizeof(line), fp);
	sprintf(buf,"%s",line);

	buf[22]='\0';

	//write_log_s("buf:%s,%d\n", buf,strlen(buf));
	//write_log_s("bufftime1:%s,%d\n", bufftime1,strlen(bufftime1));
	if(strcmp(buf, bufftime1) == 0)
	{//如果相等，说明是同一个文件
		//write_log_s("%s\n", "相等");
		memset(line1,0,256);
		fgets(line1, sizeof(line1), fp);//IP地址
		line1[strlen(line1)-1]='\0';//去掉fgets自动增加的换行符
		memset(line2,0,256);
		fgets(line2, sizeof(line2), fp);//子网掩码
		line2[strlen(line2)-1]='\0';//去掉fgets自动增加的换行符


		memset(line3,0,256);
		fgets(line3, sizeof(line3), fp);//网关
		line3[strlen(line3)-1]='\0';//去掉fgets自动增加的换行符

		memset(line4,0,256);
		fgets(line4, sizeof(line4), fp);//mac地址
		line4[strlen(line4)-1]='\0';//去掉fgets自动增加的换行符

		memset(line7,0,256);
		fgets(line7, sizeof(line7), fp);//netstatus
		line7[strlen(line7)-1]='\0';//去掉fgets自动增加的换行符

		memset(line8,0,256);
		fgets(line8, sizeof(line8), fp);//usedhcp
		line8[strlen(line8)-1]='\0';//去掉fgets自动增加的换行符



	}//如果相等，说明是同一个文件
	//释放内存
	if(buf!=NULL)
	{
		free(buf);
		buf=NULL;
	}


	fclose(fp);

	memset(szfilename,0,200);
	sprintf(szfilename,"%s/dns.txt",USER_TEMP_DIRECTORY);

	//打开文件
	FILE *fp1 = fopen(szfilename, "r");
	if(fp1 != NULL)
	{
		memset(line5,0,256);
		fgets(line5, sizeof(line5), fp1);
		line5[strlen(line5)-1]='\0';//去掉fgets自动增加的换行符
		memset(line6,0,256);
		fgets(line6, sizeof(line6), fp1);
		line6[strlen(line6)-1]='\0';//去掉fgets自动增加的换行符
	}
	fclose(fp1);


	//flag:0启用有线，1启用无线，2同时启用有线和无线
	//usedhcp:1使用dhcp 0不适用dhcp，即使用静态地址
	memset(newbuf,0,1024);
	sprintf(newbuf,"{\"ipaddress\":\"%s\",\"netmask\":\"%s\",\"gateway\":\"%s\",\"mac\":\"%s\",\"dns1\":\"%s\",\"dns2\":\"%s\",\"flag\":\"%s\",\"usedhcp\":\"%s\"}",line1,line2,line3,line4,line5,line6,line7,line8);

	*ret_len=strlen(newbuf);
	//删除临时文件
	memset(szfilename,0,200);
	sprintf(szfilename,"rm -rf %s/1.txt",USER_TEMP_DIRECTORY);
	system(szfilename);

	memset(szfilename,0,200);
	sprintf(szfilename,"rm -rf %s/dns.txt",USER_TEMP_DIRECTORY);
	system(szfilename);


//	myPtf("%s,%s,%s",szbuf,buf,newbuf);
	write_log_s("最后结果:%s,%d,哈哈哈\n", newbuf,*ret_len);


	*/
	write_log_s("%s\n", "离开处理函数fn_dealCmd_GetNetWorkSetting");

	return newbuf;


}



//命令处理函数，设置无线wifi参数
char * fn_dealCmd_SetWifiSetting(char * recv_buf,int recv_len,int * ret_len)
{
	write_log_s("%s\n", "进入处理函数fn_dealCmd_SetWifiSetting");
	char *recv_data=(char *)malloc(recv_len+1);
    	memset(recv_data,0,recv_len+1);
    	memcpy(recv_data,recv_buf,recv_len);

	write_log_s("%s,%s,%d\n", "收到数据",recv_data,recv_len);
	//开始对收到的json进行解释
	//{"wifiswitch":"1","ssid":"rdwifi2016","password":"wififornkty"}


	cJSON* cjson = cJSON_Parse(recv_data);
	if(cjson == NULL){

		write_log_s("%s\n","json pack into cjson error...");
		char *newbuf;
		newbuf=(char *)malloc(32);
		memset(newbuf,0,32);
		sprintf(newbuf,"%d",WIFI_SETTING_GET_CMD+10);
		*ret_len=strlen(newbuf);
		free(recv_data);
		recv_data=NULL;
		return newbuf;
	}

	write_log_s("%s\n","json pack into cjson ok...");



	int iflag = (int)cJSON_GetObjectItem(cjson,"wifiswitch")->valueint;//获取键值内容

	char* ssid_2_string = (char*)cJSON_GetObjectItem(cjson,"ssid")->valuestring;//获取键值内容
	char* password_3_string = (char*)cJSON_GetObjectItem(cjson,"password")->valuestring;//获取键值内容

	write_log_s("wifiswitch_1_string:%d,ssid_2_string:%s,password_3_string:%s\n",iflag,ssid_2_string,password_3_string );


	//保存当前设置的SSID，密码

	//char szflag[10];
	//memset(szflag,0,10);
	//sprintf(szflag,"%s",wifiswitch_1_string);
	//int iflag=atoi(szflag);
	//=========================开始修改无线参数=========================
	char cmd[1024];
	if(iflag==1)
	{
		write_log_s("%s，ESSID:%s,PASSWORD:%s\n","无线开关设置状态:开",ssid_2_string,password_3_string);

		//memset(cmd,0,1024);
		//sprintf(cmd,"ifdown wlan0\n");
		//system(cmd);

		memset(cmd,0,1024);
		sprintf(cmd,"sh %s/changessid.sh %s\n",USER_SCRIPT_DIRECTORY,ssid_2_string);
		system(cmd);
		memset(cmd,0,1024);
		sprintf(cmd,"sh %s/changessidpwd.sh %s\n",USER_SCRIPT_DIRECTORY,password_3_string);
		system(cmd);
		//memset(cmd,0,1024);
		//sprintf(cmd,"iwconfig wlan0 essid \"%s\"\n",ssid_2_string);
		//system(cmd);
		//memset(cmd,0,1024);
		//sprintf(cmd,"iwconfig wlan0 key s:\"%s\"\n",password_3_string);
		//system(cmd);
		//memset(cmd,0,1024);
		//sprintf(cmd,"iwconfig wlan0 ap auto \n");
		//system(cmd);

		//memset(cmd,0,1024);
		//sprintf(cmd,"ifup wlan0\n");
		//system(cmd);
		/*char ssidname[60];
		memset(ssidname,0,60);
		sprintf(ssidname,"%s","FAST-66B626");

		char ssidpwd[60];
		memset(ssidpwd,0,60);
		sprintf(ssidpwd,"%s","123456789");*/

		memset(cmd,0,1024);
		//sprintf(cmd,"sh %s/setwifistatus.sh 1 %s %s \n",USER_SCRIPT_DIRECTORY,ssid_2_string,password_3_string);
		sprintf(cmd,"sh %s/startwireless.sh\n",USER_SCRIPT_DIRECTORY);
		system(cmd);

	}
	else
	{
		write_log_s("%s\n","无线开关设置状态:关");
		memset(cmd,0,1024);
		sprintf(cmd,"sh %s/setwifistatus.sh 0 \n",USER_SCRIPT_DIRECTORY);
		system(cmd);


	}
		memset(cmd,0,1024);
		sprintf(cmd,"ifdown wlan0 ");
		system(cmd);

		memset(cmd,0,1024);
		sprintf(cmd,"ifup eth0 ");
		system(cmd);
		memset(cmd,0,1024);
		sprintf(cmd,"ifdown eth0 ");
		system(cmd);

	//=========================修改无线参数结束=========================
	//delete cjson
	cJSON_Delete(cjson);
	write_log_s("%s\n", "离开处理函数fn_dealCmd_SetWifiSetting");
	char *newbuf;
	newbuf=(char *)malloc(32);
	sprintf(newbuf,"%d",WIFI_SETTING_SET_CMD);
	*ret_len=strlen(newbuf);
	free(recv_data);
	recv_data=NULL;
	return newbuf;
}


//命令处理函数，获得无线wifi列表,此接口不再使用，2018-12-13
char * fn_dealCmd_GetWifiSetting(char * recv_buf,int recv_len,int * ret_len)
{
	write_log_s("%s\n", "进入处理函数fn_dealCmd_GetWifiSetting");

	char *newbuf;
	newbuf=(char *)malloc(4096);

	sprintf(newbuf,"{\"ssids\":\"%s\",\"channels\":\"%s\",\"powerrange\":\"%s\"}","","","");

	/*char *buf=(char *)malloc(2048);
	memset(buf,0,2048);


	sprintf(buf,"{\"ssids\":\"%s\",\"channels\":\"%s\",\"powerrange\":\"%s\"}","","","");

		*ret_len=strlen(buf);



	char cmd[1024];
	//memset(cmd,0,1024);
	//system("sudo ifdown wlan0");
	//system("sleep 1");
	//system("sudo ifup wlan0");
	//system("sleep 1");
	//write_log_s("%s\n", "sudo ifup wlan0");
	memset(cmd,0,1024);
	sprintf(cmd,"rm -rf %s/wifilist2.txt",USER_TEMP_DIRECTORY);
	system(cmd);

	memset(cmd,0,1024);
        sprintf(cmd, "sh %s/getssidlist.sh",USER_SCRIPT_DIRECTORY);
	system(cmd);


	write_log_s("sh %s/getssidlist.sh\n",USER_SCRIPT_DIRECTORY);
	memset(cmd,0,1024);
        sprintf(cmd,"chmod 777 %s/wifilist2.txt",USER_TEMP_DIRECTORY);
	system(cmd);




	char szfilename[200];
	memset(szfilename,0,200);
	sprintf(szfilename,"%s/wifilist2.txt",USER_TEMP_DIRECTORY);


	fp3=fopen(szfilename,"r");

	if(fp3==NULL)
	{
	 //不存在
		write_log_s("%s,%s\n", "open wifilist2.txterror 1",szfilename);
		sprintf(buf,"{\"ssids\":\"%s\",\"channels\":\"%s\",\"powerrange\":\"%s\"}","","","");
		*ret_len=strlen(buf);

		write_log_s("%s\n", "离开处理函数fn_dealCmd_GetWifiSetting");
		return buf;

	}
 	fclose(fp3);


	//打开文件:
	FILE *fp = fopen(szfilename, "r");
	if(fp == NULL)
	{
		fclose(fp);
//		myPtf("打开文件错误");
		write_log_s("%s,%s\n", "open wifilist2.txterror 2",szfilename);
		sprintf(buf,"{\"ssids\":\"%s\",\"channels\":\"%s\",\"powerrange\":\"%s\"}","","","");
		*ret_len=strlen(buf);
		free(buf);
		buf=NULL;
		write_log_s("%s\n", "离开处理函数fn_dealCmd_GetWifiSetting");
		return buf;
	}
	char line[256];


	memset(line,0,256);

	int len;

	int i=0;
	char wlist1[2048];
	memset(wlist1,0,2048);
	char wlist2[2048];
	memset(wlist2,0,2048);
	char wlist3[2048];
	memset(wlist3,0,2048);
	char mybuf[256];
	char mybuf2[256];

	while(!feof(fp))
	{
		memset(line,0,256);
		fgets(line, sizeof(line), fp);
		memset(mybuf,0,256);
		memset(mybuf2,0,256);
		sprintf(mybuf,"%s",&line[1]);
		mybuf[strlen(mybuf)-2]='\0';
		i=i+1;
		//write_log_s("第%d行数据,%s\n", i,mybuf);
		if(i==1)
		{
			sprintf(wlist1,"%s",mybuf);
			sprintf(wlist2,"%d",i);
			sprintf(wlist3,"%s","20");
		}
		else
		{
			strcat(wlist1,",");
			strcat(wlist1,mybuf);
			strcat(wlist2,",");
			sprintf(mybuf2,"%d",i);
			strcat(wlist2,mybuf2);
			strcat(wlist3,",");
			strcat(wlist3,"20");


		}



	}




	fclose(fp);

	char *newbuf;
	newbuf=(char *)malloc(4096);
	write_log_s("wlist1:%s\n", wlist1);
	sprintf(newbuf,"{\"ssids\":\"%s\",\"channels\":\"%s\",\"powerrange\":\"%s\"}",wlist1,wlist2,wlist3);

	//删除临时文件
	memset(szfilename,0,200);
	sprintf(szfilename,"rm -rf %s/1.txt",USER_TEMP_DIRECTORY);
	system(szfilename);

	memset(szfilename,0,200);
	sprintf(szfilename,"rm -rf %s/wifilist2.txt",USER_TEMP_DIRECTORY);
	system(szfilename);


//	myPtf("%s,%s,%s",szbuf,buf,newbuf);
	//write_log_s("最后结果:%s,%d,哈哈哈\n", newbuf,strlen(newbuf));
	*ret_len=strlen(newbuf);

	free(buf);
	buf=NULL;*/
	write_log_s("%s\n", "离开处理函数fn_dealCmd_GetWifiSetting");
	return newbuf;


}

//命令处理函数，获得无线wifi状态
char * fn_dealCmd_GetWifiStatus(char * recv_buf,int recv_len,int * ret_len)
{
	write_log_s("%s\n", "进入处理函数fn_dealCmd_GetWifiStatus");

	char *newbuf;
	newbuf=(char *)malloc(1024);
	memset(newbuf,0,1024);

	char cmd[1024];

	int len;
	int ret=0;
	char buf[1024];
	char line6[1024];
	memset(line6,0,1024);

	memset(cmd,0,1024);
	sprintf(cmd, "sh %s/getssid.sh",USER_SCRIPT_DIRECTORY);
	ret=GetCommandReturnValue(&cmd[0],&buf[0]);
	sprintf(line6,"%s",buf);
	char line7[1024];
	memset(line7,0,1024);
	memset(cmd,0,1024);
	sprintf(cmd, "sh %s/getssidpwd.sh",USER_SCRIPT_DIRECTORY);
	ret=GetCommandReturnValue(&cmd[0],&buf[0]);
	sprintf(line7,"%s",buf);
	char line4[1024];
	memset(line4,0,1024);
	memset(cmd,0,1024);
	sprintf(cmd, "sh %s/getnetstatus.sh",USER_SCRIPT_DIRECTORY);
	ret=GetCommandReturnValue(&cmd[0],&buf[0]);
	sprintf(line4,"%s",buf);

	/*char line[256];
	char szfilename[256];
	memset(szfilename,0,256);
	sprintf(szfilename,"%s","/etc/wpa_supplicant.conf");


	//打开文件:
	FILE *fp = fopen(szfilename, "r");
	if(fp == NULL)
	{
		fclose(fp);

		sprintf(newbuf,"{\"ip\":\"%s\",\"netmask\":\"%s\",\"mac\":\"%s\",\"flag\":\"%s\",\"usedhcp\":\"%s\",\"essid\":\"%s\",\"password\":\"%s\",\"status\":\"%s\"}","","","","","","","","");

		*ret_len=strlen(newbuf);
		return newbuf;
	}


	//int i=0;
	while(!feof(fp))
	{
		memset(line,0,256);
		fgets(line, sizeof(line), fp);
		write_log_s("%d,%s\n",i,line);
		i=i+1;
	}

	fclose(fp);*/
	sprintf(newbuf,"{\"ip\":\"%s\",\"netmask\":\"%s\",\"mac\":\"%s\",\"flag\":\"%s\",\"usedhcp\":\"%s\",\"essid\":\"%s\",\"password\":\"%s\",\"status\":\"%s\"}","","","",line4,"0",line6,line7,"0");

	*ret_len=strlen(newbuf);
	return newbuf;


	/*char cmd[1024];

	char line1[256];//ip

	memset(line1,0,256);

	char line2[256];//netmask

	memset(line2,0,256);

	char line3[256];//mac

	memset(line3,0,256);

	char line4[256];//netstatus

	memset(line4,0,256);

	char line5[256];//usedhcp

	memset(line5,0,256);

	char line6[256];//essid


	memset(line6,0,256);

	char line7[256];//password

	memset(line7,0,256);

	char line8[256];//running status

	memset(line8,0,256);*/
/*
	int len;
	int ret=0;
	char buf[1024];

	memset(cmd,0,1024);
	sprintf(cmd, "ifconfig -a | grep wlan0 -c");

	memset(buf,0,1024);
	ret=GetCommandReturnValue(&cmd[0],&buf[0]);
	buf[1]='\0';
	int checkcount=atoi(buf);
	write_log_s("无线网卡是否存在%s,%d\n", buf,checkcount);

	memset(cmd,0,1024);
	sprintf(cmd, "sh %s/getnetstatus.sh",USER_SCRIPT_DIRECTORY);

	memset(buf,0,1024);
	ret=GetCommandReturnValue(&cmd[0],&buf[0]);
	buf[1]='\0';
	int checkrunning=atoi(buf);
	write_log_s("无线网卡是否正在运行%s,%d\n", buf,checkrunning);

	sprintf(line4,"%d",checkrunning);//0有线,1无线,2有线无线同时启用,3有线无线同时停用

	memset(cmd,0,1024);
	sprintf(cmd, "sh %s/getssid.sh",USER_SCRIPT_DIRECTORY);
	memset(buf,0,1024);
	ret=GetCommandReturnValue(&cmd[0],&buf[0]);
	buf[strlen(buf)-1]='\0';
	sprintf(line6,"%s",buf);


	memset(cmd,0,1024);
	sprintf(cmd, "sh %s/getssidpwd.sh",USER_SCRIPT_DIRECTORY);
	memset(buf,0,1024);
	ret=GetCommandReturnValue(&cmd[0],&buf[0]);
	buf[strlen(buf)-1]='\0';
	sprintf(line7,"%s",buf);


	memset(cmd,0,1024);
	sprintf(cmd, "sh %s/validwifi.sh",USER_SCRIPT_DIRECTORY);
	memset(buf,0,1024);
	ret=GetCommandReturnValue(&cmd[0],&buf[0]);
	buf[strlen(buf)-1]='\0';
	sprintf(line8,"%s",buf);



	FILE *ptr = NULL;
	memset(cmd,0,1024);
	//================检查是否存在无线网卡设备，开始================
	sprintf(cmd,"ifconfig -a | grep wlan0 -c");
	memset(buf,0,1024);

	if((ptr = popen(cmd, "r"))==NULL)
	{

		sprintf(newbuf,"{\"ip\":\"%s\",\"netmask\":\"%s\",\"mac\":\"%s\",\"flag\":\"%s\",\"usedhcp\":\"%s\",\"essid\":\"%s\",\"password\":\"%s\",\"status\":\"%s\"}",line1,line2,line3,line4,line5,line6,line7,line8);

		*ret_len=strlen(newbuf);
		return newbuf;
	}

	memset(buf, 0, sizeof(buf));
	if((fgets(buf, sizeof(buf),ptr))!= NULL)//获取进程和子进程的总数
	{
		//myPtf("%s\n",buf);


	}
	fclose(ptr);
	buf[strlen(buf)]='\0';
	int icheckcount=atoi(buf);
	write_log_s("无线网卡是否存在：%d\n", icheckcount);

	//=============获得无线网卡IP，开始=============
	memset(cmd,0,1024);
	sprintf(cmd, "sh %s/getipforwlan.sh",USER_SCRIPT_DIRECTORY);

	memset(buf,0,1024);

	if((ptr = popen(cmd, "r"))==NULL)
	{

		sprintf(newbuf,"{\"ip\":\"%s\",\"netmask\":\"%s\",\"mac\":\"%s\",\"flag\":\"%s\",\"usedhcp\":\"%s\",\"essid\":\"%s\",\"password\":\"%s\",\"status\":\"%s\"}",line1,line2,line3,line4,line5,line6,line7,line8);

		*ret_len=strlen(newbuf);
		return newbuf;
	}

	memset(buf, 0, sizeof(buf));
	if((fgets(buf, sizeof(buf),ptr))!= NULL)//获取进程和子进程的总数
	{

		buf[strlen(buf)-1]='\0';
		sprintf(line1,"%s",buf);


	}
	fclose(ptr);
	//=============获得无线网卡IP，结束=============


	//=============获得无线网卡子网掩码，开始=============
	memset(cmd,0,1024);
	sprintf(cmd, "sh %s/getnetmaskforwlan.sh",USER_SCRIPT_DIRECTORY);

	memset(buf,0,1024);

	if((ptr = popen(cmd, "r"))==NULL)
	{

		sprintf(newbuf,"{\"ip\":\"%s\",\"netmask\":\"%s\",\"mac\":\"%s\",\"flag\":\"%s\",\"usedhcp\":\"%s\",\"essid\":\"%s\",\"password\":\"%s\",\"status\":\"%s\"}",line1,line2,line3,line4,line5,line6,line7,line8);

		*ret_len=strlen(newbuf);
		return newbuf;
	}

	memset(buf, 0, sizeof(buf));
	if((fgets(buf, sizeof(buf),ptr))!= NULL)//获取进程和子进程的总数
	{
		//myPtf("%s\n",buf);
		buf[strlen(buf)-1]='\0';
		sprintf(line2,"%s",buf);


	}
	fclose(ptr);
	//=============获得无线网卡子网掩码，结束=============

	//=============获得无线网卡mac地址，开始=============
	memset(cmd,0,1024);
	sprintf(cmd, "sh %s/getmac.sh wlan0",USER_SCRIPT_DIRECTORY);

	memset(buf,0,1024);

	if((ptr = popen(cmd, "r"))==NULL)
	{

		sprintf(newbuf,"{\"ip\":\"%s\",\"netmask\":\"%s\",\"mac\":\"%s\",\"flag\":\"%s\",\"usedhcp\":\"%s\",\"essid\":\"%s\",\"password\":\"%s\",\"status\":\"%s\"}",line1,line2,line3,line4,line5,line6,line7,line8);

		*ret_len=strlen(newbuf);
		return newbuf;
	}

	memset(buf, 0, sizeof(buf));
	if((fgets(buf, sizeof(buf),ptr))!= NULL)//获取进程和子进程的总数
	{
		//myPtf("%s\n",buf);
		buf[strlen(buf)-1]='\0';
		sprintf(line3,"%s",buf);


	}
	fclose(ptr);
	//=============获得无线网卡mac地址，结束=============

	//=============获得无线网卡状态，开始=============
	if(icheckcount>0)
	{
		memset(cmd,0,1024);
		sprintf(cmd, "sh %s/getnetstatus.sh",USER_SCRIPT_DIRECTORY);

		memset(buf,0,1024);

		if((ptr = popen(cmd, "r"))==NULL)
		{

			sprintf(newbuf,"{\"ip\":\"%s\",\"netmask\":\"%s\",\"mac\":\"%s\",\"flag\":\"%s\",\"usedhcp\":\"%s\",\"essid\":\"%s\",\"password\":\"%s\",\"status\":\"%s\"}",line1,line2,line3,line4,line5,line6,line7,line8);

			*ret_len=strlen(newbuf);
			return newbuf;
		}

		memset(buf, 0, sizeof(buf));
		if((fgets(buf, sizeof(buf),ptr))!= NULL)//获取进程和子进程的总数
		{
			//myPtf("%s\n",buf);
			buf[strlen(buf)-1]='\0';
			sprintf(line4,"%s",buf);


		}
		fclose(ptr);
	}
	else
	{
		sprintf(line4,"%d",0);
	}

	//=============获得无线网卡无线网卡状态，结束=============

	//=============获得无线网卡IP地址分配方式，是否DHCP，开始=============
	memset(cmd,0,1024);
	sprintf(cmd, "sh %s/getdhcpflag.sh 2",USER_SCRIPT_DIRECTORY);

	memset(buf,0,1024);

	if((ptr = popen(cmd, "r"))==NULL)
	{

		sprintf(newbuf,"{\"ip\":\"%s\",\"netmask\":\"%s\",\"mac\":\"%s\",\"flag\":\"%s\",\"usedhcp\":\"%s\",\"essid\":\"%s\",\"password\":\"%s\",\"status\":\"%s\"}",line1,line2,line3,line4,line5,line6,line7,line8);

		*ret_len=strlen(newbuf);
		return newbuf;
	}

	memset(buf, 0, sizeof(buf));
	if((fgets(buf, sizeof(buf),ptr))!= NULL)//获取进程和子进程的总数
	{
		//myPtf("%s\n",buf);
		buf[strlen(buf)-1]='\0';
		sprintf(line5,"%s",buf);


	}
	fclose(ptr);
	//=============获得无线网卡IP地址分配方式，是否DHCP，结束=============

	//=============获得无线Essid，开始=============
	memset(cmd,0,1024);
	sprintf(cmd, "sh %s/getssid.sh",USER_SCRIPT_DIRECTORY);

	memset(buf,0,1024);

	if((ptr = popen(cmd, "r"))==NULL)
	{

		sprintf(newbuf,"{\"ip\":\"%s\",\"netmask\":\"%s\",\"mac\":\"%s\",\"flag\":\"%s\",\"usedhcp\":\"%s\",\"essid\":\"%s\",\"password\":\"%s\",\"status\":\"%s\"}",line1,line2,line3,line4,line5,line6,line7,line8);

		*ret_len=strlen(newbuf);
		return newbuf;
	}

	memset(buf, 0, sizeof(buf));
	if((fgets(buf, sizeof(buf),ptr))!= NULL)//获取进程和子进程的总数
	{
		//myPtf("%s\n",buf);
		buf[strlen(buf)-1]='\0';
		sprintf(line6,"%s",buf);


	}
	fclose(ptr);
	//=============获得无线Essid，开始=============


	//=============获得无线Essid密码，开始=============
	memset(cmd,0,1024);
	sprintf(cmd, "sh %s/getssidpwd.sh",USER_SCRIPT_DIRECTORY);

	memset(buf,0,1024);

	if((ptr = popen(cmd, "r"))==NULL)
	{

		sprintf(newbuf,"{\"ip\":\"%s\",\"netmask\":\"%s\",\"mac\":\"%s\",\"flag\":\"%s\",\"usedhcp\":\"%s\",\"essid\":\"%s\",\"password\":\"%s\",\"status\":\"%s\"}",line1,line2,line3,line4,line5,line6,line7,line8);

		*ret_len=strlen(newbuf);
		return newbuf;
	}

	memset(buf, 0, sizeof(buf));
	if((fgets(buf, sizeof(buf),ptr))!= NULL)//获取进程和子进程的总数
	{
		//myPtf("%s\n",buf);
		buf[strlen(buf)-1]='\0';
		sprintf(line7,"%s",buf);


	}
	fclose(ptr);
	//=============获得无线Essid密码，开始=============


	write_log_s("%s\n", "获得无线网卡的运行状态，开始");
	memset(cmd,0,1024);
	sprintf(cmd, "ifconfig wlan0 | grep RUNNING -c> %s/state.txt",USER_TEMP_DIRECTORY);
	system(cmd);
	char szfilename[200];
	memset(szfilename,0,200);
	sprintf(szfilename,"%s/state.txt",USER_TEMP_DIRECTORY);

	 FILE *fp3 = fopen(szfilename, "r");



	if(fp3==NULL)
	{//不存在
		sprintf(newbuf,"{\"ip\":\"%s\",\"netmask\":\"%s\",\"mac\":\"%s\",\"flag\":\"%s\",\"usedhcp\":\"%s\",\"essid\":\"%s\",\"password\":\"%s\",\"status\":\"%s\"}",line1,line2,line3,line4,line5,line6,line7,line8);

		*ret_len=strlen(newbuf);
		return newbuf;
	}//不存在

	if(!feof(fp3))
	{
		memset(buf,0,256);
		fgets(buf, sizeof(buf), fp3);
		buf[strlen(buf)-1]='\0';
		sprintf(line4,"%s",buf);
		//write_log_s("777777777:%s\n", buf);
	}
	fclose(fp3);
	memset(cmd,0,1024);
        sprintf(cmd, "rm -rf  %s/state.txt",USER_TEMP_DIRECTORY);
	system(cmd);

	//=============获得无线网卡的运行状态，开始=============
	memset(cmd,0,1024);
	sprintf(cmd, "sh %s/validwifi.sh",USER_SCRIPT_DIRECTORY);

	memset(buf,0,1024);

	if((ptr = popen(cmd, "r"))==NULL)
	{

		sprintf(newbuf,"{\"ip\":\"%s\",\"netmask\":\"%s\",\"mac\":\"%s\",\"flag\":\"%s\",\"usedhcp\":\"%s\",\"essid\":\"%s\",\"password\":\"%s\",\"status\":\"%s\"}",line1,line2,line3,line4,line5,line6,line7,line8);

		*ret_len=strlen(newbuf);
		return newbuf;
	}

	memset(buf, 0, sizeof(buf));
	if((fgets(buf, sizeof(buf),ptr))!= NULL)//获取进程和子进程的总数
	{
		//myPtf("%s\n",buf);
		buf[strlen(buf)-1]='\0';
		sprintf(line8,"%s",buf);


	}
	fclose(ptr);

	//=============获得无线Essid密码，开始=============
	*/
 /*
	memset(cmd,0,1024);
	sprintf(cmd,"rm -rf %s/temp.txt",USER_TEMP_DIRECTORY);
	system(cmd);

	memset(cmd,0,1024);
        sprintf(cmd, "sh %s/getipforwlan.sh >> /%s/temp.txt",USER_SCRIPT_DIRECTORY,USER_TEMP_DIRECTORY);
	system(cmd);


	memset(cmd,0,1024);
        sprintf(cmd, "sh %s/getnetmaskforwlan.sh >> /%s/temp.txt",USER_SCRIPT_DIRECTORY,USER_TEMP_DIRECTORY);
	system(cmd);


	memset(cmd,0,1024);
        sprintf(cmd, "sh %s/getmac.sh wlan0 >> /%s/temp.txt",USER_SCRIPT_DIRECTORY,USER_TEMP_DIRECTORY);
	system(cmd);

	memset(cmd,0,1024);
        sprintf(cmd, "sh %s/getnetstatus.sh>> /%s/temp.txt",USER_SCRIPT_DIRECTORY,USER_TEMP_DIRECTORY);
	system(cmd);

	memset(cmd,0,1024);
        sprintf(cmd, "sh %s/getdhcpflag.sh 2 >>/%s/temp.txt",USER_SCRIPT_DIRECTORY,USER_TEMP_DIRECTORY);
	system(cmd);

	memset(cmd,0,1024);
        sprintf(cmd, "sh %s/getssid.sh>>/%s/temp.txt",USER_SCRIPT_DIRECTORY,USER_TEMP_DIRECTORY);
	system(cmd);

	memset(cmd,0,1024);
        sprintf(cmd, "sh %s/getssidpwd.sh>>/%s/temp.txt",USER_SCRIPT_DIRECTORY,USER_TEMP_DIRECTORY);
	system(cmd);

	sprintf(line8,"%d",0);
	//memset(cmd,0,1024);
        //sprintf(cmd, "sh %s/validwifi.sh >>/%s/temp.txt",USER_SCRIPT_DIRECTORY,USER_TEMP_DIRECTORY);
	//system(cmd);


	FILE * fp3;

	char szfilename[200];
	memset(szfilename,0,200);
	sprintf(szfilename,"%s/temp.txt",USER_TEMP_DIRECTORY);


	fp3=fopen(szfilename,"r");

	if(fp3==NULL)
	{
	 //不存在
		write_log_s("%s\n", "打开文件temp.txt错误");
		sprintf(newbuf,"{\"ip\":\"%s\",\"netmask\":\"%s\",\"mac\":\"%s\",\"flag\":\"%s\",\"usedhcp\":\"%s\",\"essid\":\"%s\",\"password\":\"%s\",\"status\":\"%s\"}",line1,line2,line3,line4,line5,line6,line7,line8);
		*ret_len=strlen(newbuf);

		write_log_s("%s\n", "离开处理函数fn_dealCmd_GetWifiStatus");
		return newbuf;
	}
 	fclose(fp3);


	//打开文件:
	FILE *fp = fopen(szfilename, "r");
	if(fp == NULL)
	{
		fclose(fp);
//		myPtf("打开文件错误");
		write_log_s("%s\n", "temp.txt错误");
		sprintf(newbuf,"{\"ip\":\"%s\",\"netmask\":\"%s\",\"mac\":\"%s\",\"flag\":\"%s\",\"usedhcp\":\"%s\",\"essid\":\"%s\",\"password\":\"%s\",\"status\":\"%s\"}",line1,line2,line3,line4,line5,line6,line7,line8);
		*ret_len=strlen(newbuf);


		write_log_s("%s\n", "离开处理函数fn_dealCmd_GetWifiStatus");
		return newbuf;
	}



	int len;


	while(!feof(fp))

	{
		memset(line1,0,256);
		fgets(line1, sizeof(line1), fp);
		line1[strlen(line1)-1]='\0';

		memset(line2,0,256);
		fgets(line2, sizeof(line2), fp);
		line2[strlen(line2)-1]='\0';


		memset(line3,0,256);
		fgets(line3, sizeof(line3), fp);
		line3[strlen(line3)-1]='\0';

		memset(line4,0,256);
		fgets(line4, sizeof(line4), fp);
		line4[strlen(line4)-1]='\0';

		memset(line5,0,256);
		fgets(line5, sizeof(line5), fp);
		line5[strlen(line5)-1]='\0';

		memset(line6,0,256);
		fgets(line6, sizeof(line6), fp);
		line6[strlen(line6)-1]='\0';

		memset(line7,0,256);
		fgets(line7, sizeof(line7), fp);
		line7[strlen(line7)-1]='\0';

		//memset(line8,0,256);
		//fgets(line8, sizeof(line8), fp);
		//line8[strlen(line8)-1]='\0';



		break;
	}



	fclose(fp);
	memset(cmd,0,1024);
        sprintf(cmd, "rm -rf  /%s/temp.txt",USER_TEMP_DIRECTORY);
	system(cmd);

	sprintf(line8,"%d",0);
	sprintf(newbuf,"{\"ip\":\"%s\",\"netmask\":\"%s\",\"mac\":\"%s\",\"flag\":\"%s\",\"usedhcp\":\"%s\",\"essid\":\"%s\",\"password\":\"%s\",\"status\":\"%s\"}",line1,line2,line3,line4,line5,line6,line7,line8);

	*ret_len=strlen(newbuf);
	write_log_s("要发送数据：%s\n", newbuf);

	write_log_s("%s\n", "离开处理函数fn_dealCmd_GetWifiStatus");
	return newbuf;*/


}

//命令处理函数，设置GPRS参数
char * fn_dealCmd_SetGPRSSetting(char * recv_buf,int recv_len,int * ret_len)
{
	write_log_s("%s\n", "进入处理函数fn_dealCmd_SetGPRSSetting");
	write_log_s("%s\n", "收到数据%s,长度%d",recv_buf,recv_len);
	char *recv_data=(char *)malloc(recv_len+1);
    	memset(recv_data,0,recv_len+1);
    	memcpy(recv_data,recv_buf,recv_len);

	char *newbuf;
	newbuf=(char *)malloc(1024);

	write_log_s("%s,%s,%d\n", "收到数据",recv_data,recv_len);
	//开始对收到的json进行解释
	//{"gprsswitch":"1"}


	cJSON* cjson = cJSON_Parse(recv_data);
	if(cjson == NULL){

		write_log_s("%s\n","json pack into cjson error...");

		memset(newbuf,0,1024);
		sprintf(newbuf,"%s","{\"result\",0}");
		free(recv_data);
		recv_data=NULL;
		*ret_len=strlen(newbuf);
		return newbuf;
	}

	write_log_s("%s\n","json pack into cjson ok...");



	int iflag = (int)cJSON_GetObjectItem(cjson,"flag")->valueint;//获取键值内容
	if(iflag==1)
	{
		write_log_s("%s\n","打开GPRS");
	}
	else
	{
		write_log_s("%s\n","GPRS关闭");
	}
	write_log_s("设置开关%d\n", iflag);
	write_log_s("%s\n", "离开处理函数fn_dealCmd_SetGPRSSetting");

	memset(newbuf,0,1024);
	sprintf(newbuf,"%s","{\"result\",1}");
	free(recv_data);
	recv_data=NULL;
	*ret_len=strlen(newbuf);
	return newbuf;


}


//命令处理函数，获得GPRS参数
char * fn_dealCmd_GetGPRSSetting(char * recv_buf,int recv_len,int * ret_len)
{
	write_log_s("%s\n", "进入处理函数fn_dealCmd_GetGPRSSetting");
	char cmd[1024];

	memset(cmd,0,1024);
  	sprintf(cmd, "rm -rf /%s/gprstemp.txt",USER_SCRIPT_DIRECTORY);
	system(cmd);

	char *newbuf;
	newbuf=(char *)malloc(1024);

	//获得网络连接属性
	memset(cmd,0,1024);
  	sprintf(cmd, "sh %s/getgprsstatus.sh >> /%s/gprstemp.txt",USER_SCRIPT_DIRECTORY,USER_TEMP_DIRECTORY);
	system(cmd);


	//获得IP地址
	memset(cmd,0,1024);
  	sprintf(cmd, "sh %s/getgprsip.sh >> /%s/gprstemp.txt",USER_SCRIPT_DIRECTORY,USER_TEMP_DIRECTORY);
	system(cmd);

	char line1 [256];
	char line2 [256];




	char szfilename[200];
	memset(szfilename,0,200);
	sprintf(szfilename,"%s/gprstemp.txt",USER_TEMP_DIRECTORY);


		//打开文件:
	FILE *fp = fopen(szfilename, "r");
	if(fp == NULL)
	{
		fclose(fp);
		//		myPtf("打开文件错误");
		write_log_s("open %s error\n", "gprstemp.txt");
		sprintf(newbuf,"{\"ip\":\"%s\",\"status\":\"%s\"}","","");
		*ret_len=strlen(newbuf);


		write_log_s("%s\n", "离开处理函数fn_dealCmd_GetWifiStatus");

		return newbuf;
	}

	while(!feof(fp))

	{
		memset(line1,0,256);
		fgets(line1, sizeof(line1), fp);
		line1[strlen(line1)-1]='\0';

		memset(line2,0,256);
		fgets(line2, sizeof(line2), fp);
		line2[strlen(line2)-1]='\0';
		myPtf("line1=%s\n",line1);
		myPtf("line2=%s\n",line2);
		break;
	}
	fclose(fp);
	sprintf(newbuf,"{\"status\":\"%s\",\"ip\":\"%s\"}",line1,line2);
	*ret_len=strlen(newbuf);
	write_log_s("grps:%s\n", newbuf);
	memset(cmd,0,1024);
  	sprintf(cmd, "rm -rf /%s/gprstemp.txt",USER_TEMP_DIRECTORY);

	system(cmd);

	write_log_s("%s\n", "离开处理函数fn_dealCmd_GetGPRSSetting");
	return newbuf;

}


//命令处理函数，设置服务器参数
char * fn_dealCmd_SetServerSetting(char * recv_buf,int recv_len,int * ret_len)
{
	write_log_s("%s\n", "进入处理函数fn_dealCmd_SetServerSetting");


	char *recv_data=(char *)malloc(recv_len+1);
    	memset(recv_data,0,recv_len+1);
    	memcpy(recv_data,recv_buf,recv_len);

	write_log_s("%s,%s,%d\n", "收到数据",recv_data,recv_len);
	//开始对收到的json进行解释
	//{"ipaddress":"10.100.2.2","port":"7777","ipaddress2":"10.100.2.3","port2":"6666"}


	cJSON* cjson = cJSON_Parse(recv_data);
	if(cjson == NULL){

		write_log_s("%s\n","json pack into cjson error...");
	}

	write_log_s("%s\n","json pack into cjson ok...");



	char* test_1_string = (char*)cJSON_GetObjectItem(cjson,"ipaddress")->valuestring;//获取键值内容

	char* test_2_string = (char*)cJSON_GetObjectItem(cjson,"port")->valuestring;//获取键值内容
	char* test_3_string = (char*)cJSON_GetObjectItem(cjson,"ipaddress2")->valuestring;//获取键值内容
	char* test_4_string = (char*)cJSON_GetObjectItem(cjson,"port2")->valuestring;//获取键值内容
	write_log_s("%s,%s,%s,%s\n",test_1_string,test_2_string,test_3_string,test_4_string);

	char cmd[1024];
	memset(cmd,0,1024);
	sprintf(cmd,"sh %s/changeserverip.sh 1 %s \n",USER_SCRIPT_DIRECTORY,test_1_string);
	system(cmd);

	memset(cmd,0,1024);
	sprintf(cmd,"sh %s/changeserverport.sh 1 %s \n",USER_SCRIPT_DIRECTORY,test_2_string);
	system(cmd);

	memset(cmd,0,1024);
	sprintf(cmd,"sh %s/changeserverip.sh 2 %s \n",USER_SCRIPT_DIRECTORY,test_3_string);
	system(cmd);

	memset(cmd,0,1024);
	sprintf(cmd,"sh %s/changeserverport.sh 2 %s \n",USER_SCRIPT_DIRECTORY,test_4_string);
	system(cmd);


	//delete cjson
	cJSON_Delete(cjson);

	write_log_s("%s\n", "离开处理函数fn_dealCmd_SetServerSetting");
	char *newbuf;
	newbuf=(char *)malloc(32);
	memset(newbuf,0,32);
	sprintf(newbuf,"%d",1025);
	*ret_len=strlen(newbuf);
	free(recv_data);
	recv_data=NULL;
	return newbuf;
}


//命令处理函数，获得服务器参数
char * fn_dealCmd_GetServerSetting(char * recv_buf,int recv_len,int * ret_len)
{
	write_log_s("%s\n", "进入处理函数fn_dealCmd_GetServerSetting");


	char *newbuf=NULL;
	newbuf=(char *)malloc(1024);
	memset(newbuf,0,1024);

	//sprintf(newbuf,"{\"ipaddress\":\"%s\",\"port\":\"%s\",\"ipaddress2\":\"%s\",\"port2\":\"%s\"}","10.1.70.24","1095","10.1.80.42","6035");
	//*ret_len=strlen(newbuf);
	//write_log_s("最后结果:%s,%d,哈哈哈\n", newbuf,*ret_len);
	//write_log_s("%s\n", "离开处理函数fn_dealCmd_GetServerSetting");



	//return newbuf;

	char szfilename1[MAX_PATH];
	memset(szfilename1,0,MAX_PATH);
	sprintf(szfilename1,"%s/nktyserver.conf",USER_SCRIPT_DIRECTORY);
	//myPtf("文件名：%s\n",szfilename1);
	char line1[256];//IP地址
	char line2[256];//端口


	char line3[256];//IP地址
	char line4[256];//端口

	memset(line1,0,256);
	memset(line2,0,256);

	memset(line3,0,256);
	memset(line4,0,256);

	int i=0;
	ser_conn_t myserconn[MAX_SERVER_NUM];
	write_log_s("服务器个数%d，%s\n",MAX_SERVER_NUM,szfilename1);
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
		sprintf(myserconn[i].SerIP,"%s",serveripPtr);
		port=fn_GetIniKeyInt(szsername,"port",szfilename1);
		myserconn[i].SerPort=port;
		write_log_s("server%d,%s,%d\n",(i+1),myserconn[i].SerIP,myserconn[i].SerPort );
	}

	sprintf(line1,"%s",myserconn[0].SerIP);
	sprintf(line2,"%d",myserconn[0].SerPort);

	sprintf(line3,"%s",myserconn[1].SerIP);
	sprintf(line4,"%d",myserconn[1].SerPort);


	sprintf(newbuf,"{\"ipaddress\":\"%s\",\"port\":\"%s\",\"ipaddress2\":\"%s\",\"port2\":\"%s\"}",line1,line2,line3,line4);
	*ret_len=strlen(newbuf);
	write_log_s("%s,%d\n", "离开处理函数fn_dealCmd_GetServerSetting",*ret_len);



	return newbuf;




}

//命令处理函数，获得运行版本号
char * fn_dealCmd_GetVersion(char * recv_buf,int recv_len,int * ret_len)
{
	write_log_s("%s\n", "进入处理函数fn_dealCmd_GetVersion");



	char *recv_data=(char *)malloc(recv_len+1);
    	memset(recv_data,0,recv_len+1);
    	memcpy(recv_data,recv_buf,recv_len);

	write_log_s("%s,%s\n", "==收到版本数据==",recv_data);
	cJSON* cjson = cJSON_Parse(recv_data);



	char *newbuf=NULL;
	newbuf=(char *)malloc(sizeof(char *)*1024);
	memset(newbuf,0,sizeof(char *)*1024);
	sprintf(newbuf,"{\"version\":\"%s\"}",POSMAIN_VERSION);

	*ret_len=strlen(newbuf);
	if(cjson == NULL){
		if(NULL!=recv_data)
		{
			free(recv_data);
			recv_data==NULL;
		}
		return newbuf;
	}
	char* ui_version = (char*)cJSON_GetObjectItem(cjson,"ui_version")->valuestring;//获取键值内容
	char* consumeser_version = (char*)cJSON_GetObjectItem(cjson,"consume_version")->valuestring;//获取键值内容
	char* secscreen_version = (char*)cJSON_GetObjectItem(cjson,"secscreen_version")->valuestring;//获取键值内容
	char* scanqr_version = (char*)cJSON_GetObjectItem(cjson,"scanqr_version")->valuestring;//获取键值内容


	write_log_s("pos320main:%s\n", POSMAIN_VERSION);
	write_log_s("consumeser:%s\n", consumeser_version);
	write_log_s("scanqr:%s\n", scanqr_version);
	write_log_s("secscreen:%s\n", secscreen_version);
	write_log_s("terminalproject:%s\n", ui_version);
	memset(newbuf,0,sizeof(char *)*1024);
	sprintf(newbuf,"{\"version\":\"%s\"}",POSMAIN_VERSION);

	if(NULL!=recv_data )

	{
		free(recv_data);/*释放指针pointer*/
		recv_data = NULL;/*释放指针后一定别忘了把它置为NULL，不然就成了野指针，乱指一气，很危险很暴力*/
	}

	//delete cjson
	cJSON_Delete(cjson);



	*ret_len=strlen(newbuf);

	write_log_s("%s\n", "离开处理函数fn_dealCmd_GetVersion");

	return newbuf;
}

//命令处理函数，获得各个进程运行版本号，此命令已取消
char * fn_dealCmd_UpdateAllProcessStatus(char * recv_buf,int recv_len,int * ret_len)
{
	write_log_s("%s\n", "进入处理函数fn_dealCmd_UpdateALlProcessStatus");

	char *recv_data=(char *)malloc(recv_len+1);
    	memset(recv_data,0,recv_len+1);
    	memcpy(recv_data,recv_buf,recv_len);




	char bufftime1[256];
	memset(bufftime1,0,256);

	time_t time_log = time(NULL);
	struct tm* tm_log = localtime(&time_log);
 	sprintf(bufftime1,"%04d-%02d-%02d %02d:%02d:%02d",tm_log->tm_year + 1900, tm_log->tm_mon + 1, tm_log->tm_mday,tm_log->tm_hour,
		tm_log->tm_min, tm_log->tm_sec);
	bufftime1[20]='\0';

	write_log_s("==收到数据:%s\n", "",recv_data);

	cJSON* cjson = cJSON_Parse(recv_data);
	char* consumesertime = (char*)cJSON_GetObjectItem(cjson,"consumeser")->valuestring;//获取键值内容
	char* scanqrsertime = (char*)cJSON_GetObjectItem(cjson,"scanqrser")->valuestring;//获取键值内容
	char* secscreentime = (char*)cJSON_GetObjectItem(cjson,"secscreen")->valuestring;//获取键值内容
	char* terminalProjecttime = (char*)cJSON_GetObjectItem(cjson,"terminalProject")->valuestring;//获取键值内容

	//保存版本信息、更新时间
	/*sprintf(g_versiontimelist[0].processname,"%s","terminalProject");
	sprintf(g_versiontimelist[0].updatetime,"%s",terminalProjecttime);

	sprintf(g_versiontimelist[1].processname,"%s","consumeser");
	sprintf(g_versiontimelist[1].updatetime,"%s",consumesertime);


	sprintf(g_versiontimelist[2].processname,"%s","secscreen");
	sprintf(g_versiontimelist[2].updatetime,"%s",secscreentime);


	sprintf(g_versiontimelist[3].processname,"%s","scanqrser");
	sprintf(g_versiontimelist[3].updatetime,"%s",scanqrsertime);

	if(AUTOKILL_IFTIMEOUT==0)
	{
		char *newbuf;
		newbuf=(char *)malloc(1024);
		sprintf(newbuf,"{\"processname\":\"Pos320Main\",\"status\":\"%s\",\"version\":\"%s\"}",bufftime1,POSMAIN_VERSION);
		*ret_len=strlen(newbuf);
		write_log_s("%s\n", "离开处理函数fn_dealCmd_GetVersion");
		return newbuf;
	}


	long longnow=GetTick(bufftime1);

	long long1=GetTick(consumesertime);
	long long2=GetTick(scanqrsertime);



	long long3=GetTick(secscreentime);

	long long4=GetTick(terminalProjecttime);

	char cmd[1024];
	memset(cmd,0,1024);


	double fTime1=difftime(longnow,long1);

	myPtf("比较两个时间戳:%ld,%ld，差值%f\n",long1,longnow,fTime1);
	//int days = (int)(fTime1/(3600*24));
	//int hours =(int)((fTime1-days*3600*24)/3600);
	//int minitues=(int)((fTime1-days*3600*24-hours*3600)/60);
	//int seconds=(int)(fTime1-days*3600*24-hours*3600-minitues*60);
	//myPtf("差值%d天%d小时%d分钟%d秒\n",days,hours,minitues,seconds);
	//计算时间差的秒数
	int isec1=(int)(fTime1);
	myPtf("差值%d秒\n",isec1);
	if(isec1>RESTART_DELAY)
	{//超过5秒
		myPtf("%s\n","1需要关闭和重启此进程consumeser");
		memset(cmd,0,1024);
		sprintf(cmd,"sh startsingle.sh consumeser");
		system(cmd);
	}//超过5秒

	double fTime2=difftime(longnow,long2);

	myPtf("比较两个时间戳:%ld,%ld，差值%f\n",long2,longnow,fTime2);
	//int days = (int)(fTime1/(3600*24));
	//int hours =(int)((fTime1-days*3600*24)/3600);
	//int minitues=(int)((fTime1-days*3600*24-hours*3600)/60);
	//int seconds=(int)(fTime1-days*3600*24-hours*3600-minitues*60);
	//myPtf("差值%d天%d小时%d分钟%d秒\n",days,hours,minitues,seconds);
	//计算时间差的分钟数
	int isec2=(int)(fTime2);
	myPtf("差值%d秒\n",isec2);
	if(isec2>RESTART_DELAY)
	{//超过5分钟
		myPtf("%s\n","2需要关闭和重启此进程scanqrser");
		memset(cmd,0,1024);
		sprintf(cmd,"sh startsingle.sh scanqrser");
		system(cmd);
	}//超过5分钟

	double fTime3=difftime(longnow,long3);

	myPtf("比较两个时间戳:%ld,%ld，差值%f\n",long3,longnow,fTime3);
	//int days = (int)(fTime1/(3600*24));
	//int hours =(int)((fTime1-days*3600*24)/3600);
	//int minitues=(int)((fTime1-days*3600*24-hours*3600)/60);
	//int seconds=(int)(fTime1-days*3600*24-hours*3600-minitues*60);
	//myPtf("差值%d天%d小时%d分钟%d秒\n",days,hours,minitues,seconds);
	//计算时间差的分钟数
	int isec3=(int)(fTime3);
	myPtf("差值%d秒\n",isec3);
	if(isec3>RESTART_DELAY)
	{//超过超时时间
		myPtf("%s\n","3需要关闭和重启此进程secscreen");
		memset(cmd,0,1024);
		sprintf(cmd,"sh startsingle.sh secscreen");
		system(cmd);
	}//超过超时时间



	double fTime4=difftime(longnow,long4);

	myPtf("比较两个时间戳:%ld,%ld，差值%f\n",long4,longnow,fTime4);
	//int days = (int)(fTime1/(3600*24));
	//int hours =(int)((fTime1-days*3600*24)/3600);
	//int minitues=(int)((fTime1-days*3600*24-hours*3600)/60);
	//int seconds=(int)(fTime1-days*3600*24-hours*3600-minitues*60);
	//myPtf("差值%d天%d小时%d分钟%d秒\n",days,hours,minitues,seconds);
	//计算时间差的分钟数
	int isec4=(int)(fTime4);
	myPtf("差值%d秒\n",isec4);
	if(isec4>RESTART_DELAY)
	{//超过超时时间
		myPtf("%s\n","4需要关闭和重启此进程terminalProject");
		memset(cmd,0,1024);
		sprintf(cmd,"sh startsingle.sh terminalProject");
		system(cmd);
	}//超过超时时间

*/
	char *newbuf;
	newbuf=(char *)malloc(1024);
	sprintf(newbuf,"{\"processname\":\"Pos320Main\",\"status\":\"%s\",\"version\":\"%s\"}",bufftime1,POSMAIN_VERSION);
	*ret_len=strlen(newbuf);
	write_log_s("%s\n", "离开处理函数fn_dealCmd_GetVersion");
	return newbuf;

}


//命令处理命令 保存所有参数
char * fn_dealCmd_SAVEALL_SETTING(char * recv_buf,int recv_len,int * ret_len)
{
	write_log_s("%s\n", "进入处理函数fn_dealCmd_SAVEALL_SETTING");

	char *recv_data=(char *)malloc(recv_len+1);
    	memset(recv_data,0,recv_len+1);
    	memcpy(recv_data,recv_buf,recv_len);

	char *newbuf;
	newbuf=(char *)malloc(1024);
	int iflag=0;

	//nkty#1#1#0#10.1.70.41#10.1.70.254#255.255.255.0#114.114.114.114#8.8.8.8#nkty
	write_log_s("receive:%s，datalen:%d\n",recv_buf, *ret_len);

	cJSON* cjson = cJSON_Parse(recv_data);
	char *qrstr = (char*)cJSON_GetObjectItem(cjson,"qrstr")->valuestring;//获取键值内容

	char sztmp1[5];
	char sztmp2[5];
	sztmp1[0]=qrstr[0];
	sztmp1[1]=qrstr[1];
	sztmp1[2]=qrstr[2];
	sztmp1[3]=qrstr[3];
	sztmp1[4]='\0';
	int ilen=strlen(qrstr);
	sztmp2[0]=qrstr[ilen-4];
	sztmp2[1]=qrstr[ilen-3];
	sztmp2[2]=qrstr[ilen-2];
	sztmp2[3]=qrstr[ilen-1];
	sztmp2[4]='\0';

	write_log_s("检验字符串%s,%s\n", sztmp1,sztmp2);
	if(strncmp(sztmp1,"nkty",4)!=0)
	{
		memset(newbuf,0,1024);

		sprintf(newbuf,"%d",0);
		write_log_s("二维码无效\n");
		*ret_len=strlen(newbuf);
		write_log_s("%s\n", "离开处理函数fn_dealCmd_SAVEALL_SETTING");
		return newbuf;
	}
	if(strncmp(sztmp1,sztmp1,4)!=0)
	{
		memset(newbuf,0,1024);

		sprintf(newbuf,"%d",0);
		write_log_s("二维码无效\n");
		*ret_len=strlen(newbuf);
		write_log_s("%s\n", "离开处理函数fn_dealCmd_SAVEALL_SETTING");
		return newbuf;
	}
	write_log_s("qstr:%s\n", qrstr);
	int count1=0;
   	char *p;




	//测试是是是
	//==========对数据进行校验开始==========
	char str[2048];
	memset(str,0,2048);
	char str2[2048];
	memset(str2,0,2048);


 	char s2[] = "\f";
   	char s3[] = "_";


	sprintf(str2,"%s",qrstr);

	write_log_s("str21:%s\n", str2);

	replaceStr(str2, s2, s3);

	write_log_s("str22:%s\n", str2);


	sprintf(str,"%s",str2);

    //modified by yangtao 2019-05-06
	//char delims[] = "#";//分隔符
	char delims[] = ";";//分隔符
	char *result = NULL;
	result = strtok( str, delims );
	int k=0;
	while( result != NULL ) {

	    myPtf( "result is %d,%s\n", k,result );

	    result = strtok( NULL, delims );
		k=k+1;
	}

	int count=k;
	setting_t mysetting[count];
	memset(str,0,2048);
	sprintf(str,"%s",str2);
	result=NULL;
	result = strtok( str, delims );
	k=0;
	while( result != NULL ) {
	    myPtf("%d,%s\n",k,result);
	    sprintf(mysetting[k].buffer,"%s",result);
	    result = strtok( NULL, delims );
		k=k+1;
	}
	int i=0;
	char szbuffer[2048];
	memset(szbuffer,0,2048);
	sprintf(szbuffer,"%s",str2);
	myPtf("szbuffer1:%s\n",szbuffer);
	int ichecklen=strlen(szbuffer);
	szbuffer[ichecklen-13]='n';
	szbuffer[ichecklen-12]='k';
	szbuffer[ichecklen-11]='t';
	szbuffer[ichecklen-10]='y';
	szbuffer[ichecklen-9]='\0';
	myPtf("szbuffer2:%s\n",szbuffer);
	int itest=0;

	char *tmp2=NULL;
	tmp2=malloc(2048);
	itest=Compute_string_md5(szbuffer,tmp2);
	//myPtf("666666:%s,%s\n",szbuffer,tmp2);
	char szcmpbuf[9];
	memset(szcmpbuf,0,9);
	for(i=0;i<8;i++)
	{
		szcmpbuf[i]=*(tmp2+i);
	}
	free(tmp2);
	//myPtf("7777:%s,%s\n",szcmpbuf,mysetting[count-2].buffer);
	if(strcmp(szcmpbuf,mysetting[count-2].buffer)!=0)
	{

		memset(newbuf,0,1024);

		sprintf(newbuf,"%d",-1*iflag);
		write_log_s("MD5不匹配：%s,%s\n",szcmpbuf,mysetting[count-2].buffer);
		*ret_len=strlen(newbuf);
		write_log_s("%s\n", "离开处理函数fn_dealCmd_SAVEALL_SETTING");
		return newbuf;
	}
	write_log_s("MD5匹配：%s,%s\n",szcmpbuf,mysetting[count-2].buffer);
	//==========对数据进行校验完成==========




/*

	//对数据进行校验
	char delims[] = "#";//分隔符
	char *result = NULL;
	result = strtok( str, delims );
	int k=0;
	while( result != NULL ) {

	    myPtf( "result is %d,%s\n", k,result );
	    write_log_s("result is %d,%s\n", k,result);
	    result = strtok( NULL, delims );
		k=k+1;
	}

	int count=k;
	setting_t mysetting[count];
	memset(str,0,2048);
	sprintf(str,"%s",tmp);
	result = strtok( str, delims );
	k=0;
	while( result != NULL ) {
	    sprintf(mysetting[k].buffer,"%s",result);
	    result = strtok( NULL, delims );
		k=k+1;
	}
	free(tmp);

	myPtf("校验值：%s,%s\n",mysetting[0].buffer,mysetting[count-1].buffer);
 	write_log_s("校验值：%s,%s\n",mysetting[0].buffer,mysetting[count-1].buffer);
	char szbuf[10];
	sprintf(szbuf,"%s","nkty");
	szbuf[4]='\0';
	if(strncmp(mysetting[0].buffer,mysetting[count-1].buffer,4)==0)
	{
		if(strncmp(mysetting[0].buffer,szbuf,4)==0)
		{
			myPtf("第一个和最后一个相同,是nkty\n");
			write_log_s("第一个和最后一个相同,是nkty\n");
		}
		else
		{
			myPtf("第一个和最后一个相同,不是nkty\n");
			write_log_s("第一个和最后一个相同,不是nkty\n");
			memset(newbuf,0,1024);
			sprintf(newbuf,"%d",iflag*-1);
			*ret_len=strlen(newbuf);
			write_log_s("%s\n", "离开处理函数fn_dealCmd_SAVEALL_SETTING");
			return newbuf;

		}
	}
	else
	{
		myPtf("第一个和最后一个不相同\n");
		memset(newbuf,0,1024);
		sprintf(newbuf,"%d",iflag*-1);
		*ret_len=strlen(newbuf);
		write_log_s("%s\n", "离开处理函数fn_dealCmd_SAVEALL_SETTING");
		return newbuf;

	}

	char *s1=strstr(str,"#");
	if(s1==NULL)
	{
		myPtf("不包含#\n");
		memset(newbuf,0,1024);
		sprintf(newbuf,"%d",iflag*-1);
		*ret_len=strlen(newbuf);
		write_log_s("%s\n", "离开处理函数fn_dealCmd_SAVEALL_SETTING");
		return newbuf;
	}

*/

	for(k=0;k<count;k++)
	{
		myPtf("%d,%s\n",k,mysetting[k].buffer);
		//write_log_s("分解数据%d,%s\n",k,mysetting[k].buffer);
	}
	iflag=atoi(mysetting[1].buffer);
	myPtf("数据类型%d\n",iflag);


	char cmd[1024];
	char buf[1024];
	int icheckflag=0;
	if(iflag==1)
	{//是设置本机IP
		//1#1#0#110.1.70.90#10.1.70.254#255.255.255.0#114.114.114.114#8.8.8.8
		//设置本机#有线开关#dhcp#本机ip#网关#掩码#dns1#dns2
		write_log_s("设置有线%d,%s,开始\n",iflag,qrstr);
		int iusewired=atoi(mysetting[2].buffer);
		int iusedhcp=atoi(mysetting[3].buffer);
		if(iusedhcp==1)
		{//启用DHCP，禁用静态IP
			memset(cmd,0,1024);
			sprintf(cmd,"sh %s/startdhcp.sh 1",USER_SCRIPT_DIRECTORY);
			system(cmd);
		}//启用DHCP，禁用静态IP
		else
		{//不启用dhcp，启用静态IP
			if(iusewired==1)
			{//启用有线
				memset(cmd,0,1024);
				sprintf(cmd,"sh %s/startwired.sh",USER_SCRIPT_DIRECTORY);
				system(cmd);
			}//启用有线
			memset(cmd,0,1024);
			sprintf(cmd,"sh %s/startstatic.sh",USER_SCRIPT_DIRECTORY);
			system(cmd);

			memset(cmd,0,1024);
			sprintf(cmd,"sh %s/changeip.sh %s",USER_SCRIPT_DIRECTORY,mysetting[4].buffer);
			system(cmd);


			memset(cmd,0,1024);
			sprintf(cmd,"sh %s/changenetmask.sh %s",USER_SCRIPT_DIRECTORY,mysetting[6].buffer);
			system(cmd);

			memset(cmd,0,1024);
			sprintf(cmd,"sh %s/changegateway.sh %s",USER_SCRIPT_DIRECTORY,mysetting[5].buffer);
			system(cmd);


			memset(cmd,0,1024);
			sprintf(cmd,"sh %s/changedns.sh %s %s",USER_SCRIPT_DIRECTORY,mysetting[7].buffer,mysetting[8].buffer);
			system(cmd);

		}//不启用dhcp，启用静态IP
		//设置网络类型
		memset(cmd,0,1024);
		sprintf(cmd,"echo \"1\">%s/nettype",USER_HOME_DIRECTORY);
		system(cmd);

		//判断有无无线网卡
		memset(cmd,0,1024);
		sprintf(cmd,"ifconfig | grep wlan0 -c");
		system(cmd);
		memset(buf,0,1024);
		GetCommandReturnValue(&cmd[0],&buf[0]);
		buf[strlen(buf)]='\0';
		icheckflag=atoi(buf);
		write_log_s("has wlan0：%s,%d\n",buf,icheckflag);
		//判断无线网卡是否存在
		if (icheckflag>0)
		{//有无线网卡
			memset(cmd,0,1024);
			sprintf(cmd,"ifdown wlan0 ");
			system(cmd);
		}//有无线网卡
		//判断有无4G网卡
		memset(cmd,0,1024);
		sprintf(cmd,"ifconfig | grep wwan0 -c");
		system(cmd);
		memset(buf,0,1024);
		GetCommandReturnValue(&cmd[0],&buf[0]);

		buf[strlen(buf)]='\0';
		icheckflag=atoi(buf);
		write_log_s("has 4G：%s,%d\n",buf,icheckflag);

		//判断4G网卡是否存在
		if (icheckflag>0)
		{//有4G网卡
			memset(cmd,0,1024);
			sprintf(cmd,"ifdown wwan0 ");
			system(cmd);

		}//有4G网卡

		memset(cmd,0,1024);
		sprintf(cmd,"ifdown eth0 && ifup eth0");
		system(cmd);

		write_log_s("设置有线%d,%s,结束\n",iflag,qrstr);
			//memset(cmd,0,1024);
			//sprintf(cmd,"reboot");
			//system(cmd);
	}//是设置本机IP
	else if(iflag==2)
	{//是设置无线
		write_log_s("设置无线%d,%s,开始\n",iflag,qrstr);
		//nkty#2#1#FSAT002929#1234567890#nkty
		mysetting[2].buffer[strlen(mysetting[2].buffer)]='\0';
		int iusewireless=atoi(mysetting[2].buffer);
		write_log_s("无线状态%d\n",iusewireless);
		//设置无线#wifi名#密码
		memset(cmd,0,1024);
		sprintf(cmd,"sh %s/changessid.sh %s\n",USER_SCRIPT_DIRECTORY,mysetting[3].buffer);
		system(cmd);
		memset(cmd,0,1024);
		sprintf(cmd,"sh %s/changessidpwd.sh %s\n",USER_SCRIPT_DIRECTORY,mysetting[4].buffer);
		system(cmd);
		if(iusewireless==1)
		{
			memset(cmd,0,1024);
			sprintf(cmd,"sh %s/startwireless.sh\n",USER_SCRIPT_DIRECTORY);
			system(cmd);
			//此处需要先判断有无无线网卡，如果没有无线网卡，需要重新启动
			char buf[1024];
			FILE *ptr = NULL;
			memset(cmd,0,1024);
			//================检查是否存在无线网卡设备，开始================
			sprintf(cmd,"ifconfig -a | grep wlan0 -c");
			memset(buf,0,1024);

			if((ptr = popen(cmd, "r"))==NULL)
			{

				sprintf(newbuf,"%d",-1);
				*ret_len=strlen(newbuf);
				return newbuf;
			}

			memset(buf, 0, sizeof(buf));
			if((fgets(buf, sizeof(buf),ptr))!= NULL)//获取进程和子进程的总数
			{
				//myPtf("%s\n",buf);


			}
			fclose(ptr);
			buf[strlen(buf)]='\0';
			int icheckcount=atoi(buf);
			write_log_s("无线网卡是否存在：%d\n", icheckcount);
			//================检查是否存在无线网卡设备，结束================
			if(icheckcount<=0)
			{//没有无线网卡，执行重启
				memset(cmd,0,1024);
				sprintf(cmd,"shutdown -r now");
				//sprintf(cmd,"sh %s/checkwlan.sh %s",USER_SCRIPT_DIRECTORY);
				system(cmd);
				sprintf(newbuf,"%d",-1);
				*ret_len=strlen(newbuf);
				return newbuf;
			}//没有无线网卡
			//有无线网卡，判断无线网卡是否已经启动
			memset(cmd,0,1024);
			sprintf(cmd,"ifconfig wlan0 | grep RUNNING -c");
			memset(buf,0,1024);

			if((ptr = popen(cmd, "r"))==NULL)
			{

				sprintf(newbuf,"%d",-1);
				*ret_len=strlen(newbuf);
				return newbuf;
			}

			memset(buf, 0, sizeof(buf));
			if((fgets(buf, sizeof(buf),ptr))!= NULL)//获取进程和子进程的总数
			{
				//myPtf("%s\n",buf);


			}
			fclose(ptr);
			buf[strlen(buf)]='\0';
			icheckcount=atoi(buf);
			write_log_s("无线网卡是否已经启动：%d\n", icheckcount);
			if(icheckcount<=0)
			{//无线网卡没有启动
				memset(cmd,0,1024);
				sprintf(cmd,"ifup wlan0 ");
				system(cmd);
			}//无线网卡没有启动

			//关闭有线网卡
			memset(cmd,0,1024);
			sprintf(cmd,"ifdown eth0 ");
			system(cmd);

			//判断有无4G网卡
			memset(cmd,0,1024);
			sprintf(cmd,"ifconfig | grep wwan0 -c");
			system(cmd);
			memset(buf,0,1024);
			GetCommandReturnValue(&cmd[0],&buf[0]);

			buf[strlen(buf)]='\0';
			icheckflag=atoi(buf);
			write_log_s("has 4G：%s,%d\n",buf,icheckflag);

			//判断4G网卡是否存在
			if (icheckflag>0)
			{//有4G网卡
				memset(cmd,0,1024);
				sprintf(cmd,"ifdown wwan0 ");
				system(cmd);

			}//有4G网卡


		}
		//设置网络类型
		memset(cmd,0,1024);
		sprintf(cmd,"echo \"2\">%s/nettype",USER_HOME_DIRECTORY);
		system(cmd);
		write_log_s("设置无线%d,%s,结束\n",iflag,qrstr);
		memset(cmd,0,1024);
		sprintf(cmd,"shutdown -r now");
		system(cmd);

	}//是设置无线
	else if(iflag==3)
	{//是设置通讯服务器IP地址以及端口
		write_log_s("设置服务器参数%d,%s,开始\n",iflag,qrstr);
		//3#10.1.70.13#9093#10.1.70.13#9093
		//设置服务器信息#服务器ip#端口号#备用服务器ip#备用端口号
		char cmd[1024];
		memset(cmd,0,1024);
		sprintf(cmd,"sh %s/changeserverip.sh 1 %s \n",USER_SCRIPT_DIRECTORY,mysetting[2].buffer);
		system(cmd);

		memset(cmd,0,1024);
		sprintf(cmd,"sh %s/changeserverport.sh 1 %s \n",USER_SCRIPT_DIRECTORY,mysetting[3].buffer);
		system(cmd);

		memset(cmd,0,1024);
		sprintf(cmd,"sh %s/changeserverip.sh 2 %s \n",USER_SCRIPT_DIRECTORY,mysetting[4].buffer);
		system(cmd);

		memset(cmd,0,1024);
		sprintf(cmd,"sh %s/changeserverport.sh 2 %s \n",USER_SCRIPT_DIRECTORY,mysetting[5].buffer);
		system(cmd);
		write_log_s("设置服务器参数，结束\n");
		//memset(cmd,0,1024);
		//sprintf(cmd,"reboot");
		//system(cmd);

	}//是设置通讯服务器IP地址以及端口
	else if(iflag==4)
	{//是设置4G网卡
		write_log_s("设置4G网卡,开始\n",iflag,qrstr);
		//设置网络类型
		memset(cmd,0,1024);
		sprintf(cmd,"echo \"3\">%s/nettype",USER_HOME_DIRECTORY);
		system(cmd);
		memset(cmd,0,1024);
		sprintf(cmd,"sh %s/start4G.sh\n",USER_SCRIPT_DIRECTORY);
		system(cmd);
		write_log_s("设置4G网卡,结束\n");
	}//是设置4G网卡
	free(recv_data);
	recv_data=NULL;

	//delete cjson
	cJSON_Delete(cjson);
	memset(newbuf,0,1024);
	sprintf(newbuf,"%d",iflag);
	*ret_len=strlen(newbuf);
	write_log_s("%s\n", "离开处理函数fn_dealCmd_SAVEALL_SETTING");
	return newbuf;

}
/*
调用例子：
int itest=0;
char sztemp1[]="nktyisnkty";
char *tmp2=NULL;
tmp2=malloc(2048);
itest=Compute_string_md5(sztemp1,tmp2);
myPtf("%s,%s\n",sztemp1,tmp2);
*/
//计算字符串的MD5
int Compute_string_md5(char *string_ori, char *md5_str)
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




