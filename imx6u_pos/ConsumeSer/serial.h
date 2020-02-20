#ifndef SERIAL_H
#define SERIAL_H

#define SERIAL_DEVICE "/dev/ttymxc1"

typedef struct _com_param {
	int baud;
	int databits;
	int stopbits;
	int parity;
} com_param;

int serial_init(const char *dev_name, com_param *param);
int serial_send(int fd, const char *send_buffer, size_t len);
int serial_recv(int fd, char *recv_buffer, size_t len, int timeout);
int serial_exit(int fd);
//int serial_rs485_mode(int fd, int on, u32 gpio_pin);
int serial_clear_rx(int fd);

#endif
