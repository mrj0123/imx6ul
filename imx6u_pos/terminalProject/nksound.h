#ifndef NKSOUND_H
#define NKSOUND_H

#include <QString>

//初始化声音线程
int fn_StartSoundThread();
//结束声音线程
int fn_EndSoundThread();
//开始发声
int fn_PlaySound(QString soundFile);
//停止发声
int fn_StopSound();

#endif // NKSOUND_H
