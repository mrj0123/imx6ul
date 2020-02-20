#include "a.h"
#include <sys/time.h>

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

//0表示成功，-1表示失败
int Connect(int fd, const struct sockaddr *sa, socklen_t salen)
{

    return connect(fd, sa, salen);
    /*
    //首先将连接改为非阻塞
    unsigned long ul = 1;
    ioctl(fd, FIONBIO, &ul);
    //fcntl(fd,F_SETFL,fcntl(fd,F_GETFL,0)|O_NONBLOCK);
    int connected = connect(fd, sa, salen);
    int ret = -1;
    if (connected != 0 )
	{
	    //EINPROGRESS表示连接正在建立之中
		if(errno != EINPROGRESS)
			myPtf("connect error :%s\n",strerror(errno));
		else
		{
		    //设置超时时间
			struct timeval tm = {2, 0};
			fd_set wset,rset;
			FD_ZERO(&wset);
			FD_ZERO(&rset);
			FD_SET(fd,&wset);
			FD_SET(fd,&rset);
			long t1 = time(NULL);
			//select 等待超时
			int res = select(fd+1,&rset,&wset,NULL,&tm);
			long t2 = time(NULL);
			myPtf("interval time: %ld,res=%d\n", t2 - t1,res);
			if(res < 0)
			{
				myPtf("network error in connect\n");
			}
			else if(res == 0)
			{
				myPtf("connect time out\n");
			}
			else if (res > 0)
			{
			    //连接成功
				if(FD_ISSET(fd,&wset))
				{
				    //改回阻塞状态
					myPtf("connect succeed.\n");
					//fcntl(fd,F_SETFL,fcntl(fd,F_GETFL,0) & ~O_NONBLOCK);
					ul = 0;
					ioctl(fd, FIONBIO, &ul);
					ret = 0;
				}
				else
				{
				    //其它错误，暂不处理
					myPtf("other error when select:%s\n",strerror(errno));
				}
			}
		}
	}
    return ret;
    ///////////////////////////////////////////
    int error=-1, len;
    len = sizeof(int);
    struct timeval tm;
    fd_set set;
    unsigned long ul = 1;
    ioctl(fd, FIONBIO, &ul); //设置为非阻塞模式
    myPtf("set no block!\n");
    bool ret = false;
    int conret;
    int selret = 0;
    conret = connect(fd, sa, salen);
    myPtf("set block! conret=%d\n",conret);
    if(conret != 0)
    {
        tm.tv_sec = 2;
        tm.tv_usec = 0;
        FD_ZERO(&set);
        FD_SET(fd, &set);
        selret = select(fd+1, NULL, &set, NULL, &tm);
        if(selret > 0)
        {
            getsockopt(fd, SOL_SOCKET, SO_ERROR, &error, (socklen_t *)&len);
            int err = error;
            if(err == 0)
            {
                myPtf("connect and select ok!\n");
                ul = 0;
                ioctl(fd, FIONBIO, &ul); //设置为阻塞模式
                return 0;
            }
            else
            {
                //关闭之后再次连接
                close(fd);
                //再次连接
                if(connect(fd, sa, salen)==0)
                {
                    myPtf("reconnect ok!\n");
                    ul = 0;
                    ioctl(fd, FIONBIO, &ul); //设置为阻塞模式
                    return 0;
                }
                else
                {
                    myPtf("getsockopt err = %d!\n",err);
                    ul = 0;
                    ioctl(fd, FIONBIO, &ul); //设置为阻塞模式
                    return -1;
                }
            }
        }
        else
        {
            myPtf("select error! selret=%d\n",selret);
            ul = 0;
            ioctl(fd, FIONBIO, &ul); //设置为阻塞模式
            return connect(fd, sa, salen);
        }
    }
    myPtf("connect ok!\n");
    ul = 0;
    ioctl(fd, FIONBIO, &ul); //设置为阻塞模式

    return 0;
    */
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
    ///////////////////////
    struct timeval timeo;
    socklen_t len = sizeof(timeo);
    timeo.tv_sec = 3;

    setsockopt(n, SOL_SOCKET, SO_SNDTIMEO, &timeo, len);
    ///////////////////////
    return n;
}


ssize_t Read(int fd, void *ptr, size_t nbytes)
{

	struct timeval tv = {
		.tv_sec = 3,
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
    int err;
again:
    myPtf("start write\n");
    if ((n = write(fd, ptr, nbytes)) == -1) {
        err = errno;
        myPtf("write error n=%d,error=%d\n",n,err);
        if (err == EINTR) {
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

