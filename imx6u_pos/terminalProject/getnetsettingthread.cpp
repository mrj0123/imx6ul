#include "pthread.h"
#include "stdio.h"

#include "getnetsetting.h"
#include "getnetsettingthread.h"


void *fn_getNetInfoThread(void *args)
{
    int * infoType = (int *)args;
    printf("infoType:%d,%d,args=%d\n",*infoType,*(int *)args,(int)args);
    switch(*infoType)
    {
    case 1://GETSERVERSETTING
        getserversetting();
        break;
    case 2://GETWIREDSETTING
        getwiredsetting();
        break;
    case 3://GETWIRELESSSETTING
        getwirelesssetting();
        break;
    case 4://GETGPRSSETTING
        getGPRSsetting();
        break;
    default:
        break;
    }
    delete(infoType);
    return (void *)NULL;
}

int fn_StartNetInfoThread(int infoType)
{
    pthread_t nithreadid;
    int * type = new int;
    *type = infoType;
    printf("thread fn_getNetInfoThread is ready to start!%d\n",infoType);
    int ret=pthread_create(&nithreadid,NULL,fn_getNetInfoThread,(void *)type);
    /*if (ret > 0)
    {
        printf("pthread_create fn_getNetInfoThread is success!\n");
        pthread_join(nithreadid,NULL);
        printf("thread fn_getNetInfoThread is over!\n");
    }
    else
    {
        printf("pthread_create fn_getNetInfoThread is error!\n");
    }*/
    return ret;
}

