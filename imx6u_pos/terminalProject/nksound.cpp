#include "nksound.h"
#include <unistd.h>
#include <QMediaPlayer>

//定义扫码锁
static pthread_mutex_t sdmutex;

//声音变量
QString nkSoundFile;
QMediaPlayer * nkPlayers;
//扫码线程ID
pthread_t soundthreadid;

//显示线程内处理的真实业务逻辑
void *fn_SoundInThread(void *args)
{
    while(1)
    {
        if (nkSoundFile.isEmpty())
        {
            usleep(100);
        }
        else
        {
            nkPlayers->setMedia(QMediaContent(QUrl::fromLocalFile(nkSoundFile)));
            nkPlayers->play();
            pthread_mutex_lock(&sdmutex);
            nkSoundFile.clear();
            pthread_mutex_unlock(&sdmutex);
        }
    }
    return NULL;
}

//初始化声音线程
int fn_StartSoundThread()
{
    nkPlayers =  new QMediaPlayer;
    //初始化锁
    pthread_mutex_init(&sdmutex, NULL);   //默认属性初始化锁
    //启动线程
    int ret=pthread_create(&soundthreadid,NULL,fn_SoundInThread,NULL);
    return ret;
}
//结束声音线程
int fn_EndSoundThread()
{
    //释放控件
    delete nkPlayers;
    //回收锁
    pthread_join(soundthreadid,NULL);
    pthread_mutex_destroy(&sdmutex);
    return 0;
}


//开始发声
int fn_PlaySound(QString soundFile)
{
    pthread_mutex_lock(&sdmutex);
    nkSoundFile = soundFile;
    pthread_mutex_unlock(&sdmutex);
    return 0;
}
//停止发声
int fn_StopSound()
{
    /*
    pthread_mutex_lock(&sdmutex);
    nkPlayers->stop();
    pthread_mutex_unlock(&sdmutex);
    */
    return 0;
}
