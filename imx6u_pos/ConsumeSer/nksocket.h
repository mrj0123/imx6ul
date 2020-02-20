#ifndef NKSOCKET_H
#define NKSOCKET_H

#include "a.h"
#include "wrap.h"

#define MAX_SERVER_NUM 2

typedef struct ServerConn{
    char SerIP[16];
    int SerPort;
}ser_conn_t;


void handle_sig(int signle);
int init_socket(ser_conn_t server_conn[MAX_SERVER_NUM]);
int con_socket();
int realinit_socket();
int send_socket(char * buff,int len);
int recv_socket(char * buff,int len);
int close_socket();


#endif // NKSOCKET_H
