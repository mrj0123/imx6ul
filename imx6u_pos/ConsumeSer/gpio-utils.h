
#define SYSFS_GPIO_DIR "/sys/class/gpio"
#define MAX_BUF 64

int gpio_export(unsigned int gpio); //建立设备，如果没有再内核中建立，则需要在这里建立
int gpio_unexport(unsigned int gpio);   //因为本系统不涉及安全退出，所以可以不用
int gpio_set_dir(unsigned int gpio, const char* dir);   //设置方向，输入"in"/输出“out”
int gpio_set_value(unsigned int gpio, unsigned int value);  //设置值，写操作，配合out
int gpio_get_value(unsigned int gpio, unsigned int *value); //获取值，读操作，配合in
int gpio_set_edge(unsigned int gpio, const char *edge);     //设置上升沿或下降沿触发
int gpio_fd_open(unsigned int gpio, unsigned int dir);      //打开设备
int gpio_fd_close(int fd);                                  //关闭设备

// Analog in
#define ADC_BUF 1024
#define SYSFS_AIN_DIR "/sys/devices/ocp.2/helper.11"
int ain_get_value(unsigned int ain, unsigned int *value);
