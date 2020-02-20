#ifndef GETNETSETTINGTHREAD_H
#define GETNETSETTINGTHREAD_H


//调用线程查看数据
int fn_StartNetInfoThread(int infoType);

//获取线程
void *fn_getNetInfoThread(void *args);


#endif // GETNETSETTINGTHREAD_H
