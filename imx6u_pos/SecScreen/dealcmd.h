#ifndef DEALCMD_H
#define DEALCMD_H

//显示文字
struct DisplayText
{
    int posX;
    int posY;
    int font;
    char text[48];
}distxt_t;

//显示内容
struct DisplayInfo
{
    //distxt_t * pDistxt; //文字内容
    int txtnum;         //文字条目数量
    char bgPic[128];    //背景图片路径名称
}dspinfo_t;

//显示照片，但不现实文字
char * fn_dealCmd_ShowPic(char * recv_buf,int recv_len,int * ret_len);
//显示照片，也显示文字
char * fn_dealCmd_ShowAll(char * recv_buf,int recv_len,int * ret_len);
//追加文字，不修改原来显示的背景和以前文字
char * fn_dealCmd_AppendTxt(char * recv_buf,int recv_len,int * ret_len);
char * fn_dealCmd_GetVersion(char * recv_buf,int recv_len,int * ret_len);

//int fn_SetDisplayBuff(char * recv_buf);
//初始化
int fn_dxfb_init();
//释放
void fn_dxfb_destory();
#endif // DEALCMD_H
