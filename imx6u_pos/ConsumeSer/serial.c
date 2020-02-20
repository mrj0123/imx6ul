#include "a.h"
#include "serial.h"

static speed_t getBaudrate(int baudrate)
{
	switch(baudrate) {
		case 50:
			return B50;
		case 75:
			return B75;
		case 110:
			return B110;
		case 134:
			return B134;
		case 150:
			return B150;
		case 200:
			return B200;
		case 300:
			return B300;
		case 600:
			return B600;
		case 1200:
			return B1200;
		case 1800:
			return B1800;
		case 2400:
			return B2400;
		case 4800:
			return B4800;
		case 9600:
			return B9600;
		case 19200:
			return B19200;
		case 38400:
			return B38400;
		case 57600:
			return B57600;
		case 115200:
			return B115200;
		case 230400:
			return B230400;
		case 460800:
			return B460800;
		case 500000:
			return B500000;
		case 576000:
			return B576000;
		case 921600:
			return B921600;
		case 1000000:
			return B1000000;
		case 1152000:
			return B1152000;
		case 1500000:
			return B1500000;
		case 2000000:
			return B2000000;
		case 2500000:
			return B2500000;
		case 3000000:
			return B3000000;
		case 3500000:
			return B3500000;
		case 4000000:
			return B4000000;
		default:
			break;
	}
	return B115200;
}
int set_parity(int fd, struct termios *opt, com_param *param)
{
	opt->c_cflag |= (CLOCAL | CREAD);//一般必设置的标志

	switch(param->databits)//设置数据位数
	{
		case 7:
			opt->c_cflag &= ~CSIZE;
			opt->c_cflag |= CS7;
			break;
		case 8:
			opt->c_cflag &= ~CSIZE;
			opt->c_cflag |= CS8;
			break;
		default:
			fprintf(stderr, "Unsupported data size.\n");
			return -1;
	}

	switch(param->parity)//设置校验位
	{
		case 'n':
		case 'N':
			opt->c_cflag &= ~PARENB;//清除校验位
			opt->c_iflag &= ~INPCK;//enable parity checking
			break;
		case 'o':
		case 'O':
			opt->c_cflag |= PARENB;//enable parity
			opt->c_cflag |= PARODD;//奇校验
			opt->c_iflag |= INPCK;//disable parity checking
			break;
		case 'e':
		case 'E':
			opt->c_cflag |= PARENB;//enable parity
			opt->c_cflag &= ~PARODD;//偶校验
			opt->c_iflag |= INPCK;//disable pairty checking
			break;
		case 's':
		case 'S':
			opt->c_cflag &= ~PARENB;//清除校验位
			opt->c_cflag &= ~CSTOPB;//
			opt->c_iflag |= INPCK;//disable pairty checking
			break;
		default:
			fprintf(stderr, "Unsupported parity.\n");
			return -1;
	}

	switch(param->stopbits)//设置停止位
	{
		case 1:
			opt->c_cflag &= ~CSTOPB;
			break;
		case 2:
			opt->c_cflag |= CSTOPB;
			break;
		default:
			fprintf(stderr, "Unsupported stopbits.\n");
			return -1;
	}

	opt->c_cflag |= (CLOCAL | CREAD);

	opt->c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);

	opt->c_oflag &= ~OPOST;
	opt->c_oflag &= ~(ONLCR | OCRNL);//添加的

	opt->c_iflag &= ~(ICRNL | INLCR);
	opt->c_iflag &= ~(IXON | IXOFF | IXANY);//添加的

	return 0;
}

/*
 * 正确返回fd
 */
int serial_init(const char *dev_name, com_param *param)
{
	struct termios options;
	speed_t c_baud_rate = getBaudrate(param->baud);
	int fd = open(dev_name, O_RDWR | O_NOCTTY | O_NDELAY);
	if (fd < 0) {
        perror("serial_init open error");
    	return fd;
	}
	tcgetattr(fd, &options);
	cfmakeraw(&options);
	cfsetospeed(&options, c_baud_rate);//设置波特率
	/**3. 设置新属性，TCSANOW：所有改变立即生效*/
	set_parity(fd, &options, param);
	tcflush(fd, TCIFLUSH);//溢出数据可以接收，但不读
	tcsetattr(fd, TCSANOW, &options);
	return fd;
}

/*
 * 关闭设备
 */
int serial_exit(int fd)
{
	close(fd);
	return 0;
}

/*
 * 发数据，返回发送的字节数
 */
int serial_send(int fd, const char *send_buffer, size_t len)
{
	int ret;
	ret = write(fd, send_buffer, len);
	return ret;
}

/*
 * 带超时的接收函数，超时返回0，错误返回-1，正确返回接收到的数据
 */
static inline int _recvn(int fd, char *data, size_t len, struct timeval tm)
{
	int ret;
	int recv_n = 0;
	struct timeval *ptm = &tm;
	fd_set fs_read;
	FD_ZERO(&fs_read);
	FD_SET(fd, &fs_read);

	if (tm.tv_sec == 0 && tm.tv_usec == 0) {
		ptm = NULL;
	}
	ret = select(fd + 1, &fs_read, NULL, NULL, ptm);
	if (ret < 0) {
		perror("serail select ");
		return -1;
	}
	if (ret == 0) {
	    //myPtf("serail recv time out");
	    return 0;
	}
	if (FD_ISSET(fd, &fs_read)) {
	    recv_n = read(fd, data, len);
	}
	return recv_n;
}
/*
 * 收数据，返回接受的字节数
 */
int serial_recv(int fd, char *recv_buffer, size_t len, int timeout)
{
	int nleft, nread;
    struct timeval tv_timeout;
	tv_timeout.tv_sec  = timeout / 1000;
	tv_timeout.tv_usec = (timeout % 1000) * 1000; // 1s

    nleft = len;
	while (nleft > 0) {
		if ( (nread = _recvn(fd, recv_buffer, nleft, tv_timeout)) < 0) {
			return (-1);
		} else if (nread == 0) {
			break;				/* EOF */
		}
		nleft -= nread;
		recv_buffer += nread;
	}
	if (nleft) {
		//myPtf("serial recv not enough");
	}
	return len - nleft;
}

/*
int serial_rs485_mode(int fd, int on, u32 gpio_pin)
{
	struct serial_rs485 rs485conf;

	if (ioctl(fd, TIOCGRS485, &rs485conf) < 0) {
		pr_err("Error: TIOCGRS485 ioctl not supported.\n");
		return -1;
	}

	if (on) {
		rs485conf.flags |= SER_RS485_ENABLED | (1 << 5);
		rs485conf.delay_rts_before_send = 0;
		rs485conf.delay_rts_after_send = 0;
		rs485conf.padding[0] = gpio_pin;
		ioctl(fd, TIOCSRS485, &rs485conf);
	} else {
		rs485conf.flags &= ~SER_RS485_ENABLED;
		ioctl(fd, TIOCSRS485, &rs485conf);
	}
	return 0;
}
*/

int serial_clear_rx(int fd)
{
	return tcflush(fd, TCIFLUSH);
}
