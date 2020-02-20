#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "sockfile.h"


int unsock_client_sendto(int sockfd, void *buf, size_t len, char *name)
{
	struct sockaddr_un dest_addr;
	memset(&dest_addr, 0, sizeof(dest_addr));
	dest_addr.sun_family = AF_UNIX;
	strcpy(dest_addr.sun_path, name);
	return sendto(sockfd, buf, len, 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
}

int unsock_server_init(char *name)
{
	int sockfd;
	struct sockaddr_un servaddr;
	sockfd = socket(AF_UNIX, SOCK_DGRAM, 0);
	if (sockfd < 0) {
		perror("unsock_server_init socket error\n");
		return -1;
	}
	unlink(name);
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sun_family = AF_UNIX;
	strcpy(servaddr.sun_path, name);

	if (bind(sockfd, (struct sockaddr *)&servaddr,
		sizeof(servaddr)) < 0) {
		perror("unsock_server_init bind error\n");
		close(sockfd);
		return -1;
	}

	return sockfd;
}

int unsock_server_sendto(int sockfd, struct sockaddr *dest_addr, socklen_t *addrlen, void *buf, size_t len)
{
	return sendto(sockfd, buf, len, 0, dest_addr, *addrlen);
}

int unsock_server_recvfrom(int sockfd, struct sockaddr *src_addr, socklen_t *addrlen, void *buf, size_t len, int timeout)
{
	struct timeval tv = {
		.tv_sec = timeout / 1000,
		.tv_usec = (timeout % 1000) * 1000,
	};
	int ret;
	fd_set fs_read;
	FD_ZERO(&fs_read);
	FD_SET(sockfd, &fs_read);

	ret = select(sockfd + 1, &fs_read, NULL, NULL, &tv);
	//myPtf("recvfrom select ret=%d\n",ret);
	if (ret < 0) {
		perror("select ");
		exit(2);
	}
	if (ret == 0) {
		return -1;
	}
	ret = recvfrom(sockfd, buf, len, 0, src_addr, addrlen);
	if (ret < 0) {
		perror("unsock_server_recvfrom ");
		return -1;
	}
	return ret;
}


