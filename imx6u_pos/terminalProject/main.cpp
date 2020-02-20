#include "mainwindow.h"
#include <QApplication>
char* getPidFromStr(const char *str)
{
    printf("process name is %s\n",str);
    static char sPID[8] = {0};
    int tmp = 0;
    int pos1 = 0;
    int pos2 = 0;
    int i = 0;
    int j = 0;

    for (i=0; i<(int)strlen(str); i++) {
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
    if(NULL==(fstream=popen("ps -e -o pid,comm | grep terminalProject | grep -v PID | grep -v grep", "r")))
    {
        fprintf(stderr,"execute command failed: %s", strerror(errno));
        return -1;
    }
    while(NULL!=fgets(buff, sizeof(buff), fstream)) {
        char * oldPID = getPidFromStr(buff);
        if ( strcmp(sCurrPid, oldPID) != 0 ) {
            myPtf("程序已经运行，PID=%s\n", oldPID);
            ret = 1;
        }
    }
    pclose(fstream);

    return ret;
}
int main(int argc, char *argv[])
{
    int retRun= isRunning();
    if(retRun==1){
        return 0;
    }
    QApplication a(argc, argv);
    a.setAutoSipEnabled(false);
    MainWindow w;
    w.show();

    return a.exec();
}
