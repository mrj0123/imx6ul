#ifndef AFUNIX_UDP_H
#define AFUNIX_UDP_H

#include "a.h"


typedef unsigned long long u64;
typedef signed long long   s64;
typedef unsigned int	   u32;
typedef signed int	       s32;
typedef unsigned short	   u16;
typedef signed short	   s16;
typedef unsigned char	    u8;
typedef signed char	        s8;

//通讯包结构
typedef struct _Cmd_Head {
    int cmd;
    int length;
    int tsp;//时间戳，当天时间不含日期，精确到0.1毫秒
} cmd_head_t;

//客户端操作
int c_init_net(char cli_path[108]);
//服务端操作
int s_init_net(char ser_path[108]);
//接收数据, 客户端
char * c_recv_packet(cmd_head_t * pHead,int fd,struct sockaddr_un * src_addr,socklen_t * addrlen);
//接收数据, 服務器端
char * s_recv_packet(cmd_head_t * pHead,int fd,struct sockaddr_un * src_addr,socklen_t * addrlen);
//char * recv_once_packet(cmd_head_t * pHead,int fd,struct sockaddr_un * src_addr,socklen_t * addrlen);
//服务端发送数据包，需要目的地址。服务器端目的地址是recv_packet返回的src_addr
int s_send_packet(int fd, int cmd, int tsp, u8 *buf, size_t n ,struct sockaddr_un * dst_addr,socklen_t * addrlen);
//客户端发送数据包，需要服务器段路径作为目的地址
int c_send_packet(int fd, int cmd, int tsp, u8 *buf, size_t n ,char ser_path[108]);
//关闭链接,客户端关闭时path填c_init_net中的cli_path，服务器端关闭时填s_init_net中的ser_path
int close_net(int fd,char path[108]);

#endif // AFUNIX_UDP_H
