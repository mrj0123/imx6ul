#include "common.h"
#include "qmessagebox.h"


const char *CASTFILE_CMD = "cat /proc/bus/input/devices > /usr/local/nkty/temp/deviceslog1.txt" ;
const char *DEVICE_LIST_FILE = "/usr/local/nkty/temp/deviceslog1.txt";

int getNowTime()
{
   struct timeval tv;
   gettimeofday(&tv,NULL);
   //myPtf("tv.tv_sec=%ld,tv.tv_usec=%ld\n",tv.tv_sec,tv.tv_usec);
   //return ((long long int)tv.tv_sec) * 1000 + ((long long int)tv.tv_usec) / 1000;
   return (tv.tv_sec%86400)*10000 + tv.tv_usec/100;
}

common::common()
{

}

void common::getNetworkConnet()
{
    //网络连接状态--初始化函数
    QJsonObject jsonsend;
    //发送数据
    QJsonObject jsonrecv;
    int Cmd = NETWORK_CONNECT_STATUS_GET;
    char retJson[1024];
    memset(retJson,0,1024);
    jsonrecv = scmd_network_common->send_Command(&Cmd,jsonsend,retJson);

    if(Cmd != 10000+NETWORK_CONNECT_STATUS_GET){
        //数据获取失败，可以尝试重连
        /*QMessageBox message(QMessageBox::NoIcon,"提示","发送数据失败!,ret是"+QString::number(Cmd));
        message.exec();*/

    }else{
        /*QString strShow = "";
        strShow.append(retJson);
        QMessageBox message(QMessageBox::NoIcon,"提示","json:"+strShow);
        message.exec();*/
        networkStruct_common =new networkStruct();
        networkStruct_common->netState=jsonrecv.value("netState").toString();
        networkStruct_common->networkType=jsonrecv.value("networkType").toString();
        networkStruct_common->powerrange=jsonrecv.value("powerrange").toString();
        networkStruct_common->serverState=jsonrecv.value("serverState").toString();
    }
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
    const char *itemPos[DEV_ITEMLEN][2] =
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
        myPtf("i=%d,DEV_BUS=%d,info=%s",i,DEV_BUS,deviceInfo[DEV_BUS]);
        myPtf("i=%d,DEV_VENDOR=%d,info=%s",i,DEV_VENDOR,deviceInfo[DEV_VENDOR]);
        myPtf("i=%d,DEV_PORDUCT=%d,info=%s",i,DEV_PORDUCT,deviceInfo[DEV_PORDUCT]);
        myPtf("i=%d,DEV_VERSION=%d,info=%s",i,DEV_VERSION,deviceInfo[DEV_VERSION]);
        myPtf("i=%d,DEV_NAME=%d,info=%s",i,DEV_NAME,deviceInfo[DEV_NAME]);
        myPtf("i=%d,DEV_PHYS=%d,info=%s",i,DEV_PHYS,deviceInfo[DEV_PHYS]);
        myPtf("i=%d,DEV_SYSFS=%d,info=%s",i,DEV_SYSFS,deviceInfo[DEV_SYSFS]);
        myPtf("i=%d,DEV_HANDLERS=%d,info=%s\n",i,DEV_HANDLERS,deviceInfo[DEV_HANDLERS]);
    }
    myPtf("GetKBList return\n");
    *pKBNumber = keybdcount;
    return (char *)deviceList;
}
