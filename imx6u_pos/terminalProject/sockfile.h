#ifndef SOCKFILE_H
#define SOCKFILE_H
#include	<sys/socket.h>	/* basic socket definitions */
#include	<netinet/in.h>	/* sockaddr_in{} and other Internet defns */
#include	<sys/un.h>		/* for Unix domain sockets */

#define UNIXSTR_PATH_S "afunix_test_s"
#define UNIXSTR_PATH_C "afunix_test_c"

int unsock_client_sendto(int sockfd, void *buf, size_t len, char *name);

int unsock_server_init(char *name);
int unsock_server_sendto(int sockfd, struct sockaddr *dest_addr, socklen_t *addrlen, void *buf, size_t len);
int unsock_server_recvfrom(int sockfd, struct sockaddr *src_addr, socklen_t *addrlen, void *buf, size_t len, int timeout);

#endif

