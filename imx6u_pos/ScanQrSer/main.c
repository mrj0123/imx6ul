#include "a.h"
#include "afunix_udp.h"
#include "dispose.h"
#include "dealcmd.h"

#include <math.h>


#define MAX_PATH 260
#define CONF_FILE_PATH	"Config.ini"
#define GPIO_NUM 23

#define SERIAL_DEVICE "/dev/ttymxc1"

//////////////////////////////////

char* getPidFromStr(const char *str)
{
    myPtf("process name is %s\n",str);
    static char sPID[8] = {0};
    int tmp = 0;
    int pos1 = 0;
    int pos2 = 0;
    int i = 0;
    int j = 0;

    for (i=0; i<strlen(str); i++) {
        if ( (tmp==0) && (str[i]>='0' && str[i]<='9') ) {
            tmp = 1;
            pos1 = i;
        }
        if ( (tmp==1) && (str[i]<'0' || str[i]>'9') ) {
            pos2 = i;
            break;
        }
    }
    for (j=0,i=pos1; i<pos2; i++,j++) {
        sPID[j] = str[i];
    }
    myPtf("oldPid=%s\n",sPID);
    return sPID;
}


int isRunning()
{
    int ret = 0;
    char sCurrPid[16] = {0};
    sprintf(sCurrPid, "%d", getpid());
    myPtf("sCurrPid is %s\n",sCurrPid);
    FILE *fstream=NULL;
    char buff[1024] = {0};
    if(NULL==(fstream=popen("ps -e -o pid,comm | grep scanqrser | grep -v PID | grep -v grep", "r")))
    {
        fprintf(stderr,"execute command failed: %s", strerror(errno));
        return -1;
    }
    while(NULL!=fgets(buff, sizeof(buff), fstream)) {
        char *oldPID = getPidFromStr(buff);
        if ( strcmp(sCurrPid, oldPID) != 0 ) {
            myPtf("程序已经运行，PID=%s\n", oldPID);
            ret = 1;
        }
    }
    pclose(fstream);

    return ret;
}
//启动主程序
int main()
{
    myPtf("scanqrser version=%s\n",SCANQRSERVERSION);
    if (isRunning()==1)
    {
        myPtf("scanqrser is running!\n");
        return 0;
    }
    fn_v4l2_init();
    //启动服务
    int serverfd = s_init_net(SCANQR_PATH_S);
    if (serverfd<=0)
    {
         myPtf("serverfd init error:%d",serverfd);
         return -1;
    }
    fn_init_dispose();
    //循环接收命令
    fn_dispose(serverfd);
    fn_destroy_dispose();
    //关闭网络
    close_net(serverfd,SCANQR_PATH_S);
    return 0;
}
