
#define SYSFS_GPIO_DIR "/sys/class/gpio"
#define MAX_BUF 64

int gpio_export(unsigned int gpio); //�����豸�����û�����ں��н���������Ҫ�����ｨ��
int gpio_unexport(unsigned int gpio);   //��Ϊ��ϵͳ���漰��ȫ�˳������Կ��Բ���
int gpio_set_dir(unsigned int gpio, const char* dir);   //���÷�������"in"/�����out��
int gpio_set_value(unsigned int gpio, unsigned int value);  //����ֵ��д���������out
int gpio_get_value(unsigned int gpio, unsigned int *value); //��ȡֵ�������������in
int gpio_set_edge(unsigned int gpio, const char *edge);     //���������ػ��½��ش���
int gpio_fd_open(unsigned int gpio, unsigned int dir);      //���豸
int gpio_fd_close(int fd);                                  //�ر��豸

// Analog in
#define ADC_BUF 1024
#define SYSFS_AIN_DIR "/sys/devices/ocp.2/helper.11"
int ain_get_value(unsigned int ain, unsigned int *value);
