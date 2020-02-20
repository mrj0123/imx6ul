#include "a.h"

void perr_exit(const char *s)
{
    perror(s);
    myPtf("perror error %s",s);
    //exit(1);
}

int Accept(int fd, struct sockaddr *sa, socklen_t *salenptr)
{
    int n;
again:
    if ((n = accept(fd, sa, salenptr)) < 0) {
        if ((errno == ECONNABORTED) || (errno == EINTR)) {
            goto again;
        } else {
            perr_exit("accept error");
        }
    }
    return n;
}

void Bind(int fd, const struct sockaddr *sa, socklen_t salen)
{
    if(bind(fd, sa, salen) < 0){
        perr_exit("bind error");
    }
}

int Connect(int fd, const struct sockaddr *sa, socklen_t salen)
{
    return connect(fd, sa, salen);
}



void Listen(int fd, int backlog)
{
    if(listen(fd, backlog) < 0){
        perr_exit("listen error");
    }
}

int Socket(int family, int type, int protocol)
{
    int n;
    if ((n = socket(family, type, protocol)) < 0) {
        perr_exit("socket error");
    }
    return n;
}


ssize_t Read(int fd, void *ptr, size_t nbytes)
{

	struct timeval tv = {
		.tv_sec = 5,
		.tv_usec = 0,
	};
	int ret;
	fd_set fs_read;
	FD_ZERO(&fs_read);
	FD_SET(fd, &fs_read);

	ret = select(fd + 1, &fs_read, NULL, NULL, &tv);
	if (ret < 0) {
		perror("select ");
		exit(2);
	}
	if (ret == 0) {
		return -1;
	}
    ssize_t n;
again:
    if((n = read(fd, ptr, nbytes)) == -1) {
        if(errno == EINTR) {
            goto again;
        } else {
            return -1;
        }
    }
    return n;
}

ssize_t Write(int fd, const void *ptr, size_t nbytes)
{
    ssize_t n;
    int loop=0;
again:
    myPtf("start write\n");
    if ((n = write(fd, ptr, nbytes)) == -1) {
        myPtf("write error\n");
        if (errno == EINTR) {
            myPtf("write error eintr\n");
            if (loop++>5)
            {
                return -2;
            }
            goto again;
        } else {
            myPtf("write error -1\n");
            return -1;
        }
    }
    myPtf("write success\n");
    return n;
}

ssize_t Readn(int fd, void *vptr, size_t n)
{
    size_t nleft;
    ssize_t nread;
    char *ptr;

    ptr = vptr;
    nleft = n;
    while(nleft > 0) {
        if( (nread = read(fd, ptr, nleft)) < 0) {
            if( errno == EINTR ) {
                nread = 0;
            }else if( nread == 0 ) {
                break;
            }
        }

        nleft -= nread;
        ptr += nread;
    }
    return n - nleft;
}

ssize_t Writen(int fd, const void *vptr, size_t n)
{
    size_t nleft;
    ssize_t nwrittrn;
    const char *ptr;

    ptr = vptr;
    nleft = n;

    while(nleft > 0) {
        if (( nwrittrn = write(fd, ptr, nleft)) <= 0) {
            if (nwrittrn  < 0 && errno == EINTR) {
                nwrittrn = 0;
            } else {
                return -1;
            }
        }

        nleft -= nwrittrn;
        ptr += nwrittrn;
    }
    return n;
}

static ssize_t my_read(int fd, char *ptr) {
    static int read_cnt;
    static char *read_ptr;
    static char read_buf[100];

    if (read_cnt <= 0) {
    again:
        if((read_cnt = read(fd, read_buf, sizeof(read_buf))) < 0) {
            if (errno == EINTR) {
                goto again;
            }
            return -1;
        } else if(read_cnt == 0) {
            return 0;
        }
        read_ptr = read_buf;
    }
    read_cnt --;
    *ptr = *read_ptr++;
    return 1;
}


ssize_t Readline(int fd, void *vptr, size_t maxlen)
{
    ssize_t n, rc;
    char c, *ptr;

    ptr = vptr;
    for (n = 0; n < maxlen; n++) {
        rc = my_read(fd, &c);
        if (rc == 1) {
            *ptr++ = c;
            if (c == '\n') {
                break;
            }
        }else if(rc == 0) {
            *ptr = 0;
            return n-1;
        }else {
            return -1;
        }
    }

    *ptr = 0;
    return n;
}

void Close(int fd)
{
    if(close(fd) == -1) {
        perr_exit("close error");
    }
}

