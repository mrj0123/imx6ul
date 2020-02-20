#include "getnetsetting.h"

#include <stdio.h>
#include <stdlib.h>


#include <sys/types.h>
#include <sys/stat.h>


#include <unistd.h>
#include <fcntl.h>

#include "inifile.h"
#include "common.h"
 
#include <time.h>
#include <ctype.h>

#include <unistd.h>
#include <string.h>
#include <stdlib.h>


//获得服务器参数存放到全局变量g_server_setting 二维数组
void getserversetting(void)
{
	printf("====server setting====\n");
	
	char szcmd[1024];	
	char myresult[1024];
	int ret=0;
	//=============================获得第一个服务器的IP======================
	memset(szcmd,0,1024);
	sprintf(szcmd, "/usr/local/nkty/script/getserverip.sh 1");
	memset(myresult,0,1024);
	ret=GetCommandReturnValue(&szcmd[0],&myresult[0]);
	sprintf(g_server_setting[0].serverip,myresult);
	//=============================获得第一个服务器的端口======================
	memset(szcmd,0,1024);
	sprintf(szcmd, "/usr/local/nkty/script/getserverport.sh 1");
	memset(myresult,0,1024);
	ret=GetCommandReturnValue(&szcmd[0],&myresult[0]);
	myresult[strlen(myresult)]='\0';
	g_server_setting[0].serverport=atoi(myresult);
	//=============================获得第二个服务器的IP======================

	memset(szcmd,0,1024);
	sprintf(szcmd, "/usr/local/nkty/script/getserverip.sh 2");
	memset(myresult,0,1024);
	ret=GetCommandReturnValue(&szcmd[0],&myresult[0]);
	sprintf(g_server_setting[1].serverip,myresult);
	//=============================获得第二个服务器的端口======================
	memset(szcmd,0,1024);
	sprintf(szcmd, "/usr/local/nkty/script/getserverport.sh 2");
	memset(myresult,0,1024);
	ret=GetCommandReturnValue(&szcmd[0],&myresult[0]);
	myresult[strlen(myresult)]='\0';
	g_server_setting[1].serverport=atoi(myresult);

	printf("server1IP:%s\n",g_server_setting[0].serverip);
	printf("server1POrt:%d\n",g_server_setting[0].serverport);
	printf("server2IP:%s\n",g_server_setting[1].serverip);
	printf("server2POrt:%d\n",g_server_setting[1].serverport);

}
//获得有线设置存放到全局变量g_wired_setting 结构变量
void getwiredsetting(void)
{
	printf("====get wired setting====\n");

	char szcmd[1024];	
	char myresult[1024];
	int ret=0;
	//=============================获得有线设置的IP======================
	memset(szcmd,0,1024);
	sprintf(szcmd, "/usr/local/nkty/script/getipaddress.sh");
	memset(myresult,0,1024);
	ret=GetCommandReturnValue(&szcmd[0],&myresult[0]);
	sprintf(g_wired_setting.ipaddress,myresult);
	//=============================获得有线设置的子网掩码======================
	memset(szcmd,0,1024);
	sprintf(szcmd, "/usr/local/nkty/script/getnetmask.sh");
	memset(myresult,0,1024);
	ret=GetCommandReturnValue(&szcmd[0],&myresult[0]);
	sprintf(g_wired_setting.netmask,myresult);
	//=============================获得有线设置的网关======================
	memset(szcmd,0,1024);
	sprintf(szcmd, "/usr/local/nkty/script/getgateway.sh");
	memset(myresult,0,1024);
	ret=GetCommandReturnValue(&szcmd[0],&myresult[0]);
	sprintf(g_wired_setting.gateway,myresult);

	//=============================获得有线设置的mac地址======================
	memset(szcmd,0,1024);
	sprintf(szcmd, "/usr/local/nkty/script/getmac.sh eth0");
	memset(myresult,0,1024);
	ret=GetCommandReturnValue(&szcmd[0],&myresult[0]);
	sprintf(g_wired_setting.mac,myresult);

	//=============================获得有线设置的netstatus======================
	memset(szcmd,0,1024);
	sprintf(szcmd, "/usr/local/nkty/script/getnetstatus.sh");
	memset(myresult,0,1024);
	ret=GetCommandReturnValue(&szcmd[0],&myresult[0]);
	g_wired_setting.netstatus=atoi(myresult);
	
	//=============================获得有线设置的dhcpflag======================
	memset(szcmd,0,1024);
	sprintf(szcmd, "/usr/local/nkty/script/getnetstatus.sh");
	memset(myresult,0,1024);
	ret=GetCommandReturnValue(&szcmd[0],&myresult[0]);
	g_wired_setting.dhcpflag=atoi(myresult);

	//=============================获得有线设置的dns1======================
	memset(szcmd,0,1024);
	sprintf(szcmd, "sed -n '/dns-nameservers/'p /etc/network/interfaces | awk '{print $2}'");
	memset(myresult,0,1024);
	ret=GetCommandReturnValue(&szcmd[0],&myresult[0]);
	sprintf(g_wired_setting.dns1,"%s",myresult);

	//=============================获得有线设置的dns2======================
	memset(szcmd,0,1024);
	sprintf(szcmd, "sed -n '/dns-nameservers/'p /etc/network/interfaces | awk '{print $3}'");
	memset(myresult,0,1024);
	ret=GetCommandReturnValue(&szcmd[0],&myresult[0]);
	sprintf(g_wired_setting.dns2,"%s",myresult);


	printf("IP:%s\n",g_wired_setting.ipaddress);
	printf("netmask:%s\n",g_wired_setting.netmask);
	printf("gateway:%s\n",g_wired_setting.gateway);
	printf("mac:%s\n",g_wired_setting.mac);
	printf("netstatus:%d\n",g_wired_setting.netstatus);
	printf("dhcpflag:%d\n",g_wired_setting.dhcpflag);
	printf("dns1:%s\n",g_wired_setting.dns1);
	printf("dns2:%s\n",g_wired_setting.dns2);


	
}
//获得无线设置存放到全局变量g_wireless_setting 结构变量
void getwirelesssetting(void)
{
	printf("====get wirelesssetting====\n");

	char szcmd[1024];	
	char myresult[1024];
	int ret=0;
	//=============================获得无线设置的ESSID======================
	memset(szcmd,0,1024);
	sprintf(szcmd, "/usr/local/nkty/script/getssid.sh 1");
	memset(myresult,0,1024);
	ret=GetCommandReturnValue(&szcmd[0],&myresult[0]);
	sprintf(g_wireless_setting.essidname,"%s",myresult);
	//=============================获得无线设置的密码======================
	memset(szcmd,0,1024);
	sprintf(szcmd, "/usr/local/nkty/script/getssidpwd.sh");
	memset(myresult,0,1024);
	ret=GetCommandReturnValue(&szcmd[0],&myresult[0]);
	sprintf(g_wireless_setting.password,"%s",myresult);	
	//=============================获得有线设置的状态======================

	memset(szcmd,0,1024);
	sprintf(szcmd, "/usr/local/nkty/script/getnetstatus.sh");
	memset(myresult,0,1024);
	ret=GetCommandReturnValue(&szcmd[0],&myresult[0]);
	g_wireless_setting.netstatus=atoi(myresult);


	printf("essid:%s\n",g_wireless_setting.essidname);
	printf("password:%s\n",g_wireless_setting.password);
	printf("netstatus:%d\n",g_wireless_setting.netstatus);

}
//获得4G设置
void getGPRSsetting(void)
{
	printf("====get 4g setting====\n");
	char szcmd[1024];	
	char myresult[1024];
	int ret=0;
	int checkflag=0;
	//清空初始值
	g_gprs_setting.nettype=0;
	g_gprs_setting.status=0;
	memset(g_gprs_setting.ipaddress,0,16);

	//=============================获得网络类型======================
	memset(szcmd,0,1024);
	sprintf(szcmd, "cat /usr/local/nkty/nettype");
	memset(myresult,0,1024);
	ret=GetCommandReturnValue(&szcmd[0],&myresult[0]);
	myresult[strlen(myresult)]='\0';
	if(strlen(myresult)>2)
	{
		checkflag=0;
	}
	else
	{
		checkflag=atoi(myresult);
	}
	g_gprs_setting.nettype=checkflag;
	//=============================判断有无4G网卡======================
	memset(szcmd,0,1024);
	sprintf(szcmd, "ifconfig | grep wwan0 -c");
	memset(myresult,0,1024);
	ret=GetCommandReturnValue(&szcmd[0],&myresult[0]);
	myresult[strlen(myresult)]='\0';
	checkflag=atoi(myresult);
	if(checkflag>0)
	{//有4G网卡
		memset(szcmd,0,1024);
		//=============================判断4G网卡是否运行状态======================
		sprintf(szcmd, "ifconfig wwan0 | grep RUNNING -c");
		memset(myresult,0,1024);
		ret=GetCommandReturnValue(&szcmd[0],&myresult[0]);
		myresult[strlen(myresult)]='\0';
		checkflag=atoi(myresult);
		if(checkflag>0)
		{//4G网卡正在运行
			g_gprs_setting.status=checkflag;
			sprintf(szcmd, "ifconfig wwan0 | grep 'inet addr'  | awk -F \" \" '{print $2'}  | awk -F \":\" \"{print $2}\"");
			memset(myresult,0,1024);
			ret=GetCommandReturnValue(&szcmd[0],&myresult[0]);
			myresult[strlen(myresult)]='\0';
			memset(g_gprs_setting.ipaddress,0,16);
			sprintf(g_gprs_setting.ipaddress,"%s",myresult);
		}//4G网卡正在运行
	}//有4G网卡
}
