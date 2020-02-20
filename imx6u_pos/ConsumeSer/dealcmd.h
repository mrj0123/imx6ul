#ifndef DEALCMD_H
#define DEALCMD_H

char * fn_dealCmd_GetTermInfo(char *recv_buf,int recv_len,int * ret_len,int *send_cmd);
char * fn_dealCmd_GetConsumeInfo(char *recv_buf,int recv_len,int * ret_len,int *send_cmd);
char * fn_dealCmd_ReqConsume(char *recv_buf,int recv_len,int * ret_len,int *send_cmd);
char * fn_dealCmd_QueConsumeRet(char *recv_buf,int recv_len,int * ret_len,int *send_cmd);
char * fn_dealCmd_CancelConsume(char *recv_buf,int recv_len,int * ret_len,int *send_cmd);
char * fn_dealCmd_GetFlowTatal(char *recv_buf,int recv_len,int * ret_len,int *send_cmd);
char * fn_dealCmd_GetFlowInfo(char *recv_buf,int recv_len,int * ret_len,int *send_cmd);
char * fn_dealCmd_GetVersion(char *recv_buf,int recv_len,int * ret_len,int *send_cmd);
char * fn_dealCmd_GetScanQrSerVersion(char *recv_buf,int recv_len,int * ret_len,int *send_cmd);
char * fn_dealCmd_GetAccountInfo(char *recv_buf,int recv_len,int * ret_len,int *send_cmd);
char * fn_dealCmd_GetQrcode(char *recv_buf,int recv_len,int * ret_len,int *send_cmd);
char * fn_dealCmd_CancelFlow(char *recv_buf,int recv_len,int * ret_len,int *send_cmd);

char * fn_dealCmd_Echo(char *recv_buf,int recv_len,int * ret_len,int *send_cmd);
//脱机消费
char * fn_dealCmd_ReqOfflineConsume(char *recv_buf,int recv_len,int * ret_len,int *send_cmd);
//脱机流水上传
char * fn_dealCmd_ReqUploadFlow(char *recv_buf,int recv_len,int * ret_len,int *send_cmd);
//查询离线流水数量
char * fn_dealCmd_GetOfflineFlowNum(char *recv_buf,int recv_len,int * ret_len,int *send_cmd);
//下载用户名单列表
char * fn_dedlCmd_DownloadUserList(char *recv_buf,int recv_len,int * ret_len,int *send_cmd);
//下载名单到第几包
char * fn_dedlCmd_GetUserListPackNum(char *recv_buf,int recv_len,int * ret_len,int *send_cmd);

int fn_GetServerTime(int *send_cmd);    //更新时间
//初始化程序
int fn_dealCmd_init();
int fn_dealCmd_destroy();

#endif // DEALCMD_H
