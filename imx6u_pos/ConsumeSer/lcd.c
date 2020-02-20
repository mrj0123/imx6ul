#include "lcd.h"
#include "i2c-dev.h"
#include <math.h>
#include <time.h>

static int file;
#define LCD_I2CBUS 1
#define LCD_ADDRESS 0x38
#define BELLON
#define BELLOFF

#define MVERSION 80802;

#define lcd_write(daddr, len, block) i2c_smbus_write_i2c_block_data(file, daddr, len, block)
#define Delay_ms(ms) usleep(ms * 1000)

BYTE     LcdDisBuf[16];
const BYTE  NumberTable[25] ={0x5f,0x03,0x3e,0x2f,0x63,0x6d,0x7d,0x07,0x7f,0x6f,//LCD��ʾ���0-9
							  0x77,0x79,0x5c,0x3b,0x7c,0x74,					//A-F
							  0X5D,0x73,0x58,0x76,0x5B,0x31,0x20,0x24,0x2c};	//G H L P U n - �� ��
//extern BYTE	CommIndt;
WORD 	VerNo = 0x256;
union{WORD   lng;BYTE sht[2];} MachNoDis = {
	.lng = 36
};

int CharToInt(char hex)
{
    if (hex>='0' && hex <='9')
        return hex - '0';
    if (hex>='A' && hex <= 'F')
        return hex-'A'+10;
    if(hex>='a' && hex <= 'f')
        return hex-'a'+10;
    return 0;
}

/*****************************************************************************************/
/*****************************************************************************************/
WORD Hex_bcd(BYTE HexData)
{
	 WORD  BcdData;
	 BYTE temp;

	 temp=HexData%100;
	 BcdData=((WORD)HexData)/100<<8;
	 BcdData=BcdData|temp/10<<4;
	 BcdData=BcdData|temp%10;
	 return BcdData;
}

static int open_i2c_dev(int i2cbus, char *filename, size_t size, int quiet)
{
	int file;

	snprintf(filename, size, "/dev/i2c/%d", i2cbus);
	filename[size - 1] = '\0';
	file = open(filename, O_RDWR);

	if (file < 0 && (errno == ENOENT || errno == ENOTDIR)) {
		sprintf(filename, "/dev/i2c-%d", i2cbus);
		file = open(filename, O_RDWR);
	}

	if (file < 0 && !quiet) {
		if (errno == ENOENT) {
			fprintf(stderr, "Error: Could not open file "
				"`/dev/i2c-%d' or `/dev/i2c/%d': %s\n",
				i2cbus, i2cbus, strerror(ENOENT));
		} else {
			fprintf(stderr, "Error: Could not open file "
				"`%s': %s\n", filename, strerror(errno));
			if (errno == EACCES)
				fprintf(stderr, "Run as root?\n");
		}
	}

	return file;
}

static int set_slave_addr(int file, int address, int force)
{
	/* With force, let the user read from/write to the registers
	   even when a driver is also running */
	if (ioctl(file, force ? I2C_SLAVE_FORCE : I2C_SLAVE, address) < 0) {
		fprintf(stderr,
			"Error: Could not set address to 0x%02x: %s\n",
			address, strerror(errno));
		return -errno;
	}

	return 0;
}


/*
 * init i2c bus, and lcd init
 */
int lcd_module_init()
{
	char filename[32];
	file = open_i2c_dev(LCD_I2CBUS, filename, sizeof(filename), 0);
	if (file < 0) {
		exit(1);
	}

	set_slave_addr(file, LCD_ADDRESS, 1);
	u8 block[1] = {};
	lcd_write(0xc8, 0, block);

	return 0;
}

/*
 * clear lcd
 */
int lcd_clear(void)
{
	u8 block[1] = {};
	lcd_write(0x50, 0, block);
	return 0;
}
/***************************************************************************
 * �������ƣ�BYTE lcd_bytes_display(BYTE DisStartAddr,BYTE *DataStartPtr,BYTE DataNum,BYTE LcdSlaveAddr)
 * �������ܣ������ֽ�������ʾ�����������йص�ַ�����Ȼ��˳��д�����ݣ�
 *          PCF8576���Զ����е�ַ�ۼӣ�1��4�༶��������2�ۼӣ�����ʾ
 * ִ��������
 * �������룺��1��DisStartAddr��ʾ����ʼ��ַ���˴�����Ϊż����
 *          ��2��*DataStartPtrͨ��Ϊ��ʾ������׵�ַ��3��DataNum��Ҫ��ʾ���ֽ���
 *          ��4��LcdSlaveAddr ѡ�����Ա����û���
 * ���������(1)0 ����д�� ��2����0 д��ʧ��
 ***************************************************************************/
int lcd_bytes_display(BYTE DisStartAddr, BYTE *DataStartPtr, BYTE DataNum, BYTE LcdSlaveAddr)
{
	u8 block[DataNum + 1];
	block[0] = DisStartAddr;
	memcpy(block + 1, DataStartPtr, DataNum);
	lcd_write(LcdSlaveAddr, sizeof(block), block);
   	return 0;
}

/***************************************************************************
 * �������ƣ�BYTE lcd_byte_display(BYTE DisAddr,BYTE DisData,BYTE LcdSlaveAddr)
 * �������ܣ����ֽ�������ʾ�����������йص�ַ�����Ȼ�����뽫��ʾ���ݣ�
 * ִ��������
 * �������룺��1��DisAddr����ʾ���ݵĵ�ַ���˴�����Ϊż����
 *          ��2��DisData��ʾ������	��3��LcdSlaveAddrѡ�����Ա����û���
 * ���������(1)0 ����д�� ��2����0 д��ʧ��
 ***************************************************************************/
int lcd_byte_display(BYTE DisAddr,BYTE DisData,BYTE LcdSlaveAddr)
{
	u8 block[2] = {DisAddr, DisData};
	lcd_write(LcdSlaveAddr, sizeof(block), block);
	return 0;
}

/************************************************************************************************
 *�������ƣ�void Remind_info_display(BYTE Command,BYTE DisData)
 *�������ܣ�������ʾ��Ϣ��ʾ����
 *ִ��������
 *�������룺��1��command,��ʾ���ͣ�2��DisData��ʾ������
 *���������(�ޣ�
 *�������ڣ�2007/03/20
 *�����ˣ�  Shaoping
 *��ע��    ��1��OPENBIT,CLOSEBIT�У� DisData��4λ����ʾλ������λ����ʾ������
 ************************************************************************************************/
void Remind_info_display(BYTE Command,BYTE DisData)
{
  	BYTE temp,tt,flag=0;
  	WORD tempword;

  	switch(Command)
  	{
 		case OPENSECOND:  									 	//
        	lcd_byte_display(4,0x80,USERLCD);	 			 	//����Ա����û���LCDͬʱ��ʾ
  			lcd_byte_display(4,0x80,OPERLCD);
        	break;
	   	case CLOSESECOND:  								  	 	//
	        lcd_byte_display(4,0,USERLCD);	 				 	//����Ա����û���LCDͬʱ��ʾ
	  		lcd_byte_display(4,0,OPERLCD);
	        break;
	   	case TRADEREADY:  								  	 	//
	        lcd_byte_display(28,0x84,USERLCD);	 			 	//����Ա����û���LCDͬʱ��ʾ�����͡���ſ���
	  		lcd_byte_display(28,0x84,OPERLCD);
	        break;
	   	case TRADEAGAIN:  								  	 	//
	        lcd_byte_display(28,0x88,USERLCD);	 			 	//����Ա����û���LCDͬʱ��ʾ�����͡��طſ���
	  		lcd_byte_display(28,0x88,OPERLCD);
	        break;
	    case SCOLLDIS:											//��ʼ������ʾ���˴���ʾ��1λ
			for(temp=0;temp<7;temp++) {LcdDisBuf[temp]=0;}		//DisData,��3BITΪ��ʾλ�Σ���5BITΪ��ʾ����
	        LcdDisBuf[((DisData&0xE0)>>5)]=NumberTable[DisData&0x1f];
	        lcd_bytes_display(0,LcdDisBuf,7,USERLCD);	        //����Ա����û���LCDͬʱ��ʾ	         //
	  		lcd_bytes_display(0,LcdDisBuf,7,OPERLCD);
	        break;
	    case ERRORNUM:		   									//��ʾ������,�����Ų���BCDת������ʾ��Ϊ16����
	        LcdDisBuf[0]=0x7C;									//E
	        LcdDisBuf[1]=0x30;									//r
	        LcdDisBuf[2]=0x30;									//r
	        LcdDisBuf[3]=0x20;									//-
	        LcdDisBuf[4]=0x00;
	        LcdDisBuf[5]=0x00;
	        LcdDisBuf[6]=0x00;
	        LcdDisBuf[7]=0x00;
	        LcdDisBuf[8]=0x00;
	        temp=DisData&0xf0;
	        temp>>=4;
	        LcdDisBuf[10]=NumberTable[temp];
	        LcdDisBuf[9]= NumberTable[DisData&0x0f];
	        LcdDisBuf[11]=0x00;
	        LcdDisBuf[12]=0x00;
	        LcdDisBuf[13]=0x00;
	        LcdDisBuf[14]=0x00;
	        LcdDisBuf[15]=0x00;
	  		lcd_bytes_display(0,LcdDisBuf,16,USERLCD); 			//����Ա����û���LCDͬʱ��ʾ�����
	  		lcd_bytes_display(0,LcdDisBuf,16,OPERLCD);
			BELLON;Delay_ms(600);
//			while(!PinRc500||PinNoGd)	{ClrWdt();}
			BELLOFF;
	        break;
	   	case OPERERROR:										 	//��������
	        LcdDisBuf[0]=0x7C;									//E
	        LcdDisBuf[1]=0x30;									//r
	        LcdDisBuf[2]=0x30;									//r
	        LcdDisBuf[3]=0x39;									//0
	        LcdDisBuf[4]=0xB0;									//���������r
			for(temp=5;temp<16;temp++) LcdDisBuf[temp]=0;		//
	  		lcd_bytes_display(0,LcdDisBuf,16,USERLCD); 		 	//����Ա����û���
	  		lcd_bytes_display(0,LcdDisBuf,16,OPERLCD);
			BELLON;Delay_ms(600);BELLOFF;
	        break;
	    case OPENBIT:											 //��ʾĳһλ������ʾ����ֻ����0��F��������ʾ�硰��ſ�������ʾ��Ϣ
	        temp=(DisData&0xf0)>>4;								 //��4BIT����ʾ��λ��
	        LcdDisBuf[temp]=NumberTable[DisData&0x0f];
	        lcd_byte_display(temp+temp,LcdDisBuf[temp],USERLCD); //����Ա����û���LCDͬʱ��ʾ
	  		lcd_byte_display(temp+temp,LcdDisBuf[temp],OPERLCD);
	        break;
	    case CLOSEBIT:											 //�ر�ĳһλ��ʾ����OPENBIT ����γ���˸��Ч��
			temp=(DisData&0xf0)>>4;								 //��4BIT����ʾ��λ��
	        temp+=temp;
	        lcd_byte_display(temp,0,USERLCD);	 				 //����Ա����û���LCDͬʱ��ʾ
	  		lcd_byte_display(temp,0,OPERLCD);
	        break;
	    case PSWDINPUT:											 //������ʾ�������룺��ʾ��			��
			if(DisData==0)										 //2008��6��26��!=0ʱ�������һ����ʾ
				{for(temp=0;temp<7;temp++)  LcdDisBuf[temp]=0;}
	        LcdDisBuf[3]|=0x80;									 //�������롱
	        for(temp=7;temp<16;temp++) LcdDisBuf[temp]=0;
	        lcd_bytes_display(0,LcdDisBuf,16,USERLCD);	         //����Ա����û���LCDͬʱ��ʾ���п���ʾ����
	  		lcd_bytes_display(0,LcdDisBuf,16,OPERLCD);
		    break;
	    case PSWDBIT:											 //��ʾ"H",DisData�˴�����ʾ��λ��ֻ�е�12��13λ�ǡ��ס��֣���ʾ����
	        if(DisData==12)
		    {
			 	LcdDisBuf[12]=0x33;
	  		 	LcdDisBuf[13]=0x20;
		     	lcd_bytes_display(24,&LcdDisBuf[12],2,USERLCD); //����Ա����û���LCDͬʱ��ʾ���п���ʾ����
	  		 	lcd_bytes_display(24,&LcdDisBuf[12],2,OPERLCD);
		    }
			else												//����DisData�ṩ����ʾλ�Σ�����Ӧλ��ʾH
			{
	         	lcd_byte_display(DisData<<1,0x73,USERLCD); 		//����Ա����û���LCDͬʱ��ʾ
	  		 	lcd_byte_display(DisData<<1,0x73,OPERLCD);
			}
	        break;
	    case EXCDHOUR:		   	        						//��ʾ������δ����Сʱ��
	        LcdDisBuf[0]=0x7C;									//E
	        LcdDisBuf[1]=0x38;									//c
	        LcdDisBuf[2]=0x3b;									//d
	        LcdDisBuf[3]=0x20;									//-
	        LcdDisBuf[4]=0x00;
	        LcdDisBuf[5]=0x00;
	        LcdDisBuf[6]=0x00;
	        LcdDisBuf[7]=0x00;
	        LcdDisBuf[8]=0x00;
	        tempword=Hex_bcd(DisData);							//HEX��ת��BCD����ʾ
	  	    temp=(BYTE)((tempword&0x0f00)>>8);
	  		LcdDisBuf[11]=NumberTable[temp];
	  		temp=(BYTE)((tempword&0x00f0)>>4);
	  		LcdDisBuf[10]=NumberTable[temp];
	  		temp=(BYTE)(tempword&0x000f);
	  		LcdDisBuf[9]=NumberTable[temp];
	        LcdDisBuf[12]=0x00;
	        LcdDisBuf[13]=0x00;
	        LcdDisBuf[14]=0x00;
	        LcdDisBuf[15]=0x00;
	  		lcd_bytes_display(0,LcdDisBuf,16,USERLCD);    		//����Ա����û���LCDͬʱ��ʾ
	  		lcd_bytes_display(0,LcdDisBuf,16,OPERLCD);
	        break;
	     case NUMADVER:									     	//��ʾ�����źͻ�������汾�ţ�
	  	    LcdDisBuf[0]=0x00;
	        LcdDisBuf[1]=0x31;								 	//"n"
			LcdDisBuf[2]=NumberTable[MachNoDis.sht[1]&0x0f];	//MachNoDis.lngר���ڻ�������ʾ����RealMachNo��BCD��
			LcdDisBuf[3]=NumberTable[MachNoDis.sht[0]>>4];
			LcdDisBuf[4]=NumberTable[MachNoDis.sht[0]&0x0f];
	  		LcdDisBuf[5]=0;
	  		LcdDisBuf[6]=0;
			LcdDisBuf[11]=0;
			LcdDisBuf[10]=NumberTable[(BYTE)((VerNo>>12)&0x0f)]; //�汾���ǹ̶���2�ֽ���ʾ�룬����Ϊ0-9
			LcdDisBuf[9]=NumberTable[(BYTE)((VerNo>>8)&0x0f)];
			LcdDisBuf[8]=NumberTable[(BYTE)((VerNo>>4)&0x0f)];
			LcdDisBuf[7]=NumberTable[(BYTE)(VerNo&0x0f)];
	        LcdDisBuf[12]=0x84;							 	 	//"X"
	  		LcdDisBuf[13]=0x05;
	  		LcdDisBuf[14]=0;
	  		LcdDisBuf[15]=0;
	  		lcd_bytes_display(0,LcdDisBuf,16,USERLCD);	     	//ͬʱ�ڲ���Ա����û�����ʾ
	  		lcd_bytes_display(0,LcdDisBuf,16,OPERLCD);
	        break;
	     case MACHSTAT:											//��������ʱʹ�ã����������ʾ���ס���Ȼ��ȫ����ʾ
	        for(temp=0;temp<16;temp++)   LcdDisBuf[temp]=0;
	        lcd_bytes_display(0,LcdDisBuf,16,USERLCD);	        //ͬʱ�������Lcd���п���ʾ����
	        lcd_bytes_display(0,LcdDisBuf,16,OPERLCD);
	        LcdDisBuf[0]=0x20;  							    //��ʼ��ε���"�ס���
	        lcd_byte_display(26,LcdDisBuf[0],USERLCD);
	        lcd_byte_display(26,LcdDisBuf[0],OPERLCD);
	        Delay_ms(100);
	        LcdDisBuf[1]=0x04;
	        lcd_byte_display(24,LcdDisBuf[1],USERLCD);
	        lcd_byte_display(24,LcdDisBuf[1],OPERLCD);
	        Delay_ms(100);
	        LcdDisBuf[1]+=0x02;
	        lcd_byte_display(24,LcdDisBuf[1],USERLCD);
	        lcd_byte_display(24,LcdDisBuf[1],OPERLCD);
	        Delay_ms(100);
	        LcdDisBuf[1]+=0x80;
	        lcd_byte_display(24,LcdDisBuf[1],USERLCD);
	        lcd_byte_display(24,LcdDisBuf[1],OPERLCD);
	        Delay_ms(100);
	        LcdDisBuf[1]+=0x01;
	        lcd_byte_display(24,LcdDisBuf[1],USERLCD);
	        lcd_byte_display(24,LcdDisBuf[1],OPERLCD);
	        Delay_ms(100);
	        LcdDisBuf[0]+=0x01;
	        lcd_byte_display(26,LcdDisBuf[0],USERLCD);
	        lcd_byte_display(26,LcdDisBuf[0],OPERLCD);
	        Delay_ms(100);
	        LcdDisBuf[0]+=0x10;
	        lcd_byte_display(26,LcdDisBuf[0],USERLCD);
	        lcd_byte_display(26,LcdDisBuf[0],OPERLCD);
	        Delay_ms(100);
	        LcdDisBuf[0]+=0x04;
	        lcd_byte_display(26,LcdDisBuf[0],USERLCD);
	        lcd_byte_display(26,LcdDisBuf[0],OPERLCD);
	        Delay_ms(100);
	        LcdDisBuf[0]=0xff;
	        LcdDisBuf[1]=0xff;
	        lcd_byte_display(24,LcdDisBuf[1],USERLCD);
	        lcd_byte_display(24,LcdDisBuf[1],OPERLCD);
	        lcd_byte_display(26,LcdDisBuf[0],USERLCD);
	        lcd_byte_display(26,LcdDisBuf[0],OPERLCD);
	        Delay_ms(200);										 	//���ס��������ʾ����
	        for(temp=0;temp<16;temp++) {LcdDisBuf[temp]=0xff;}
	        lcd_bytes_display(0,LcdDisBuf,16,USERLCD);	         	//����Ա����û���LCDͬʱ��ʾ���п���ʾ����
	  		lcd_bytes_display(0,LcdDisBuf,16,OPERLCD);
	        break;
	     case FULLSCRN:												//ȫ����ʾ����ÿ��λ��ʾ��ͬ�����ݣ�ע��DisDataֱ�Ӿ�����ʾ��
	        for(temp=0;temp<16;temp++) {LcdDisBuf[temp]=DisData;}	//��ȫ����ʾ��ʹ����������ʾ
	        lcd_bytes_display(0,LcdDisBuf,16,USERLCD);	         	//����Ա����û���LCDͬʱ��ʾ	         //
	  		lcd_bytes_display(0,LcdDisBuf,16,OPERLCD);
	        break;
	     case BADERROR:												//���ش���״̬��ʾ����ֹ��ʾȫ0��ȫF,ע��DisData������ʾ��
	        DisData=NumberTable[DisData&0x1F];						//DisData<0x1f��
	        for(temp=0;temp<7;temp++)  {LcdDisBuf[temp]=DisData;} 	//�ֶδ���ʾ����ȫ1
	        for(temp=7;temp<16;temp++) {LcdDisBuf[temp]=0;} 		//����ʾ
	        lcd_bytes_display(0,LcdDisBuf,16,USERLCD);	         	//����Ա����û���LCDͬʱ��ʾ	         //
	  		lcd_bytes_display(0,LcdDisBuf,16,OPERLCD);
		    //CLOSEINT;
			temp=1;
			//while(temp)		{ClrWdt();}								//ֻ�忴�Ź�����ֹ����
	        break;
	     case RADDIS:												//����״��ʾ����3Bits����ʾλ������5Bits����ʾ����
			tt=(DisData&0xE0)>>5;
			tt++;
			for(temp=0;temp<tt;temp++) {LcdDisBuf[temp]=NumberTable[DisData&0x1F];}
	   		for(temp=tt;temp<7;temp++) {LcdDisBuf[temp]=0;}
	        lcd_bytes_display(0,LcdDisBuf,7,USERLCD);	         	//����Ա����û���LCDͬʱ��ʾ	         //
	  		lcd_bytes_display(0,LcdDisBuf,7,OPERLCD);
	 		break;
	    default:break;
  	}//switch
}

//������ʾ�汾��
void Version_display()
{
    int i;
    unsigned char Version[]="b0001";
    unsigned char SerialNo[]="12345ff";
    memset(LcdDisBuf,0,16);
    for (i=0;i<7;i++)
    {
        LcdDisBuf[i]=NumberTable[CharToInt(SerialNo[i])];
    }
    /////////////////////////////////////////////////////////////
    for (i=0;i<5;i++)
    {
        //LcdDisBuf[11-i]=NumberTable[Version/(unsigned long)(pow(10,4-i)+0.01)%10];
        LcdDisBuf[11-i]=NumberTable[CharToInt(Version[i])];
    }
    /////////////////////////////////////////////////////////////
    if(1){LcdDisBuf[12]|=0x87;LcdDisBuf[13]|=0x35;}		//û��ͨѶ�ź�,��ʾ���ס���
    //if(0){LcdDisBuf[12]|=0x78;LcdDisBuf[13]|=0xCA;}	//�л�ѹ��ˮ����ʾ���ڡ�
    //LcdDisBuf[15]=0x40;
    LcdDisBuf[14]=0;		  												//����ʾ����������������ſ�����NKTY"
    LcdDisBuf[15]=0x40;

    lcd_bytes_display(0,LcdDisBuf,16,USERLCD);	     	     //ͬʱ�ڲ���Ա����û�����ʾ
    lcd_bytes_display(0,LcdDisBuf,16,OPERLCD);  		     //
}

void SerialNo_display()
{

}

//��ʾ��ǰʱ�䣬��Ϊ��������ʾ:����Ϊ˫��������ʾ:
void DateTime_display()
{
    //��ȡ��ǰʱ��
    time_t timer;//time_t����long int ����
    struct tm *tblock;
    timer = time(NULL);
    tblock = localtime(&timer);
    memset(LcdDisBuf,0,16);
    //myPtf("Local time is: %02d-%02d %02d:%02d\n", tblock->tm_mon+1, tblock->tm_mday, tblock->tm_hour, tblock->tm_min);
    ////////////////////////////////////////////////
    //��ʾʱ��
    LcdDisBuf[0]=NumberTable[tblock->tm_hour/10];
    LcdDisBuf[1]=NumberTable[tblock->tm_hour%10];
    LcdDisBuf[2]=(tblock->tm_sec%2)<<7;
    LcdDisBuf[3]=NumberTable[tblock->tm_min/10];
    LcdDisBuf[4]=NumberTable[tblock->tm_min%10];
    ////////////////////////////////////////////////
    //��ʾ����
    LcdDisBuf[11]=NumberTable[(tblock->tm_mon+1)/10];
    LcdDisBuf[10]=NumberTable[(tblock->tm_mon+1)%10];
    LcdDisBuf[9]=0x20;
    LcdDisBuf[8]=NumberTable[tblock->tm_mday/10];
    LcdDisBuf[7]=NumberTable[tblock->tm_mday%10];
    LcdDisBuf[15]=0x40; //ͼ��

    //ͬʱ�ڲ���Ա����û�����ʾ
    lcd_bytes_display(0,LcdDisBuf,16,USERLCD);
    lcd_bytes_display(0,LcdDisBuf,16,OPERLCD);
}
//��ʾ������
void InputMoney_display(int InputMoney)
{
    /*
    memset(LcdDisBuf,0,16);
    //��ʾPAUSE
    LcdDisBuf[0]=0x76;
    LcdDisBuf[1]=0x77;
    LcdDisBuf[2]=0x5B;
    LcdDisBuf[3]=0x6d;
    LcdDisBuf[4]=0x7c;
    */
    //��ȡ��ǰʱ��
    time_t timer;//time_t����long int ����
    struct tm *tblock;
    timer = time(NULL);
    tblock = localtime(&timer);
    memset(LcdDisBuf,0,16);
    //myPtf("Local time is: %02d-%02d %02d:%02d\n", tblock->tm_mon+1, tblock->tm_mday, tblock->tm_hour, tblock->tm_min);
    ////////////////////////////////////////////////
    //��ʾʱ��
    LcdDisBuf[0]=NumberTable[tblock->tm_hour/10];
    LcdDisBuf[1]=NumberTable[tblock->tm_hour%10];
    LcdDisBuf[2]=(tblock->tm_sec%2)<<7;
    LcdDisBuf[3]=NumberTable[tblock->tm_min/10];
    LcdDisBuf[4]=NumberTable[tblock->tm_min%10];
    ////////////////////////////////////////////////
    //��ʾ���
    InputMoney%=100000;
    int len;
    if (InputMoney==0)
    {
        len=1;
    }
    else
    {
        len=(int)(log10(InputMoney)+1);
    }
    for (int i=0;i<len;i++)
    {
        //LcdDisBuf[11-i]=NumberTable[Version/(unsigned long)(pow(10,4-i)+0.01)%10];
        LcdDisBuf[len+6-i]=NumberTable[InputMoney/(unsigned long)(pow(10,len-1-i)+0.01)%10];
    }
    //�������С��1Ԫ����Ҫ��Ԫλ��0
    if (InputMoney<100)
    {
        LcdDisBuf[9]=NumberTable[0];
        //�������С��1�ǣ���Ҫ�ڽ�λ��0
        if (InputMoney<10)
        {
            LcdDisBuf[8]=NumberTable[0];
        }
    }
    LcdDisBuf[8]|=0x80;

    LcdDisBuf[14]=0x80; //���
    LcdDisBuf[15]=0x40; //ͼ��
    //ͬʱ�ڲ���Ա����û�����ʾ
    lcd_bytes_display(0,LcdDisBuf,16,USERLCD);
    lcd_bytes_display(0,LcdDisBuf,16,OPERLCD);
}

//��0�ȴ�ˢ�� ZeroPos��ʾ��ʾ��0��λ�ã�0~4��
void WaitForCard_display(int InputMoney,int ZeroPos)
{
    memset(LcdDisBuf,0,16);
    //�ҵ���ʾ0��λ��
    LcdDisBuf[ZeroPos]=NumberTable[0];
    ////////////////////////////////////////////////
    //��ʾ���
    InputMoney%=100000;
    int len;
    if (InputMoney==0)
    {
        len=1;
    }
    else
    {
        len=(int)(log10(InputMoney)+1);
    }
    for (int i=0;i<len;i++)
    {
        //LcdDisBuf[11-i]=NumberTable[Version/(unsigned long)(pow(10,4-i)+0.01)%10];
        LcdDisBuf[len+6-i]=NumberTable[InputMoney/(unsigned long)(pow(10,len-1-i)+0.01)%10];
    }
    //�������С��1Ԫ����Ҫ��Ԫλ��0
    if (InputMoney<100)
    {
        LcdDisBuf[9]=NumberTable[0];
        //�������С��1�ǣ���Ҫ�ڽ�λ��0
        if (InputMoney<10)
        {
            LcdDisBuf[8]=NumberTable[0];
        }
    }
    LcdDisBuf[8]|=0x80;


    LcdDisBuf[14]=0x80|0x04; //���+��ſ�
    LcdDisBuf[15]=0x40; //ͼ��
    //ͬʱ�ڲ���Ա����û�����ʾ
    lcd_bytes_display(0,LcdDisBuf,16,USERLCD);
    lcd_bytes_display(0,LcdDisBuf,16,OPERLCD);
}

//���ѳɹ�����һ����ʾRemainMoney��ʾ���ڶ���ConsumeMoney��ʾ���Ѷ�
void ConsumeSuccess_display(int RemainMoney,int ConsumeMoney)
{
    memset(LcdDisBuf,0,16);
    //��ʾ���
    RemainMoney%=10000000;
    int len;
    if (RemainMoney==0)
    {
        len=1;
    }
    else
    {
        len=(int)(log10(RemainMoney)+1);
    }
    for (int i=0;i<len;i++)
    {
        //LcdDisBuf[11-i]=NumberTable[Version/(unsigned long)(pow(10,4-i)+0.01)%10];
        LcdDisBuf[7-len+i]=NumberTable[RemainMoney/(unsigned long)(pow(10,len-1-i)+0.01)%10];
    }
    //�������С��1Ԫ����Ҫ��Ԫλ��0
    if (RemainMoney<100)
    {
        LcdDisBuf[4]=NumberTable[0];
        //�������С��1�ǣ���Ҫ�ڽ�λ��0
        if (RemainMoney<10)
        {
            LcdDisBuf[5]=NumberTable[0];
        }
    }
    LcdDisBuf[5]|=0x80;

    ////////////////////////////////////////////////
    //��ʾ���Ѷ�
    ConsumeMoney%=100000;
    if (ConsumeMoney==0)
    {
        len=1;
    }
    else
    {
        len=(int)(log10(ConsumeMoney)+1);
    }
    for (int i=0;i<len;i++)
    {
        //LcdDisBuf[11-i]=NumberTable[Version/(unsigned long)(pow(10,4-i)+0.01)%10];
        LcdDisBuf[len+6-i]=NumberTable[ConsumeMoney/(unsigned long)(pow(10,len-1-i)+0.01)%10];
    }
    //�������С��1Ԫ����Ҫ��Ԫλ��0
    if (ConsumeMoney<100)
    {
        LcdDisBuf[9]=NumberTable[0];
        //�������С��1�ǣ���Ҫ�ڽ�λ��0
        if (ConsumeMoney<10)
        {
            LcdDisBuf[8]=NumberTable[0];
        }
    }
    LcdDisBuf[8]|=0x80;


    LcdDisBuf[14]=0x80|0x40|0x01; //���+���+���˿�
    LcdDisBuf[15]=0x40; //ͼ��
    //ͬʱ�ڲ���Ա����û�����ʾ
    lcd_bytes_display(0,LcdDisBuf,16,USERLCD);
    lcd_bytes_display(0,LcdDisBuf,16,OPERLCD);
}

//����ʧ�ܣ�������ʾ
void ErrorCode_display(int Errorcode)
{
    memset(LcdDisBuf,0,16);
    //��ʾ������
    /*
    LcdDisBuf[0]=0x7c; //E
    LcdDisBuf[1]=0x30; //r
    LcdDisBuf[2]=0x30; //r
    LcdDisBuf[3]=0x39; //o
    LcdDisBuf[4]=0x30; //r
    */
    LcdDisBuf[1]=0x7c; //E
    LcdDisBuf[2]=0x30; //r
    LcdDisBuf[3]=0x30; //r
    LcdDisBuf[4]=0x20; //-
    //ErrorCode
    LcdDisBuf[5]=NumberTable[Errorcode/10];
    LcdDisBuf[6]=NumberTable[Errorcode%10];
    /////////////////////////////////////////////////////
    //LcdDisBuf[14]=0x80|0x40|0x01; //���+���+���˿�
    LcdDisBuf[15]=0x40; //ͼ��
    //ͬʱ�ڲ���Ա����û�����ʾ
    lcd_bytes_display(0,LcdDisBuf,16,USERLCD);
    lcd_bytes_display(0,LcdDisBuf,16,OPERLCD);
}










/************************************************************************************************
 *�������ƣ�void Multi_word_display(BYTE Command,DWORD DisDword)
 *�������ܣ��๦����Ϣ��ʾ������ʾʱ�䡢���������
 *ִ��������
 *�������룺
 *���������(�ޣ�
 *�������ڣ�2007/03/20
 *�����ˣ�  Shaoping
 *��ע��    ���ֽڵ���ʾ��ϢDisDword���������ʾ���ݵĲ�ͬ������ֽڵĺ���Ҳ��ͬ��ע������
 ************************************************************************************************/
void Multi_word_display(BYTE Command,DWORD DisDword)
{
  	BYTE temp,tt,flag=0;
  	WORD tempword;
  	union{DWORD dword;BYTE byte[4];}tempdword;

  	//tempdword.dword=DisDword;
  	tempdword.dword=5001;
  	switch(Command)
  	{
	    case NORMALDS:  									 		 //ϵͳ����ʱ������ʾ����ʾ��ǰʱ��Ͳ�����ʽ��DisDwordΪBCD���ʽ
/*
	        LcdDisBuf[4] =NumberTable[tempdword.byte[0]&0x0f];  	 //DisDword��null(1Byte)+opermode(1Byte)+hour(1Byte)+minute(1Byte),
			LcdDisBuf[3] =NumberTable[(tempdword.byte[0]&0xf0)>>4];
	        LcdDisBuf[2] =0x80;										 //����ð�ţ�������ʾ
			LcdDisBuf[1] =NumberTable[tempdword.byte[1]&0x0f];
			LcdDisBuf[0] =NumberTable[(tempdword.byte[1]&0xf0)>>4];
			LcdDisBuf[5] =0x74;										 //"F"
			LcdDisBuf[6] =NumberTable[tempdword.byte[2]&0x0f];		 //������ʽ��ʾ
	        LcdDisBuf[10]=0x31;								 		 //"n"
			LcdDisBuf[9]=NumberTable[MachNoDis.sht[1]&0x0f];
			LcdDisBuf[8]=NumberTable[MachNoDis.sht[0]>>4];
			LcdDisBuf[7]=NumberTable[MachNoDis.sht[0]&0x0f];
*/
			LcdDisBuf[0] =NumberTable[(tempdword.byte[1]&0xf0)>>4];
			myPtf("LcdDisBuf[0]=%02X,(tempdword.byte[1]&0xf0)>>4=%02X\n",LcdDisBuf[0],(tempdword.byte[1]&0xf0)>>4);
			LcdDisBuf[1] =NumberTable[tempdword.byte[1]&0x0f];
			myPtf("LcdDisBuf[1]=%02X,tempdword.byte[1]&0x0f=%02X\n",LcdDisBuf[1],tempdword.byte[1]&0x0f);
	        LcdDisBuf[2] =0x80;										 //����ð�ţ�������ʾ
			LcdDisBuf[3] =NumberTable[(tempdword.byte[0]&0xf0)>>4];
			myPtf("LcdDisBuf[3]=%02X,(tempdword.byte[0]&0xf0)>>4=%02X\n",LcdDisBuf[3],(tempdword.byte[0]&0xf0)>>4);
	        LcdDisBuf[4] =NumberTable[tempdword.byte[0]&0x0f];  	 //DisDword��null(1Byte)+opermode(1Byte)+hour(1Byte)+minute(1Byte),
			myPtf("LcdDisBuf[4]=%02X,tempdword.byte[0]&0x0f=%02X\n",LcdDisBuf[4],tempdword.byte[0]&0x0f);
			LcdDisBuf[5] =0x74|0x80;										 //"F"
			LcdDisBuf[6] =NumberTable[tempdword.byte[2]&0x0f];		 //������ʽ��ʾ
			myPtf("LcdDisBuf[6]=%02X,tempdword.byte[2]&0x0f=%02X\n",LcdDisBuf[6],tempdword.byte[2]&0x0f);
			/////////////////////////////////////////////////////////////
			LcdDisBuf[7]=NumberTable[MachNoDis.sht[0]&0x0f];        //����
			myPtf("LcdDisBuf[7]=%02X,MachNoDis.sht[0]&0x0f=%02X\n",LcdDisBuf[7],MachNoDis.sht[0]&0x0f);
			LcdDisBuf[8]=NumberTable[MachNoDis.sht[0]>>4]|0x80;          //���ŵ�һ���ַ�
			myPtf("LcdDisBuf[8]=%02X,MachNoDis.sht[0]>>4=%02X\n",LcdDisBuf[8],MachNoDis.sht[0]>>4);
			LcdDisBuf[9]=NumberTable[MachNoDis.sht[1]&0x0f];
			myPtf("LcdDisBuf[9]=%02X,MachNoDis.sht[1]&0x0f=%02X\n",LcdDisBuf[9],MachNoDis.sht[1]&0x0f);
	        LcdDisBuf[10]=0x31;								 		 //"n"

	        for(temp=11;temp<15;temp++) LcdDisBuf[temp]=0;
//			if(CommIndt==0) 	 {LcdDisBuf[12]|=0x87;LcdDisBuf[13]|=0x35;}		//û��ͨѶ�ź�,��ʾ���ס���
			if(1){LcdDisBuf[12]|=0xFF;LcdDisBuf[13]|=0xFF;}		//û��ͨѶ�ź�,��ʾ���ס���
			if(0){LcdDisBuf[12]|=0x78;LcdDisBuf[13]|=0xCA;}		//�л�ѹ��ˮ����ʾ���ڡ�
			//LcdDisBuf[15]=0x40;
            LcdDisBuf[14]=0xC1;		  												//����ʾ����������������ſ�����NKTY"
			LcdDisBuf[15]=DisDword;
			//LcdDisBuf[15]=0x40;

	  		lcd_bytes_display(0,LcdDisBuf,16,USERLCD);	     	     //ͬʱ�ڲ���Ա����û�����ʾ
	  		lcd_bytes_display(0,LcdDisBuf,16,OPERLCD);  		     //
	        break;
	    case INPUTMONEY:															//��һ����ʾ����ڶ���
			for(temp=0;temp<16;temp++) LcdDisBuf[temp]=0;							//DisDword��number(Bit7-Bit4)+DisData(28bits),BCD���ʽ
	        temp=tempdword.byte[3]&0x70;					 		 				//
	        if(temp==0x70) LcdDisBuf[0]=NumberTable[tempdword.byte[3]&0x0f]; 		//number=7,����λ��ʼ��ʾ,
	        if(temp>=0x60) LcdDisBuf[1]=NumberTable[(tempdword.byte[2]&0xf0)>>4];   //ǧ
			if(temp>=0x50) LcdDisBuf[2]=NumberTable[tempdword.byte[2]&0x0f];		//��
			if(temp>=0x40) LcdDisBuf[3]=NumberTable[(tempdword.byte[1]&0xf0)>>4];	//ʮ
			if(temp>=0x30) LcdDisBuf[4]=NumberTable[tempdword.byte[1]&0x0f];		//��
	        LcdDisBuf[5] =NumberTable[(tempdword.byte[0]&0xf0)>>4]|0x80; 			//ʮ��λ��ͬʱĬ��Ҫ����С����
			LcdDisBuf[6] =NumberTable[tempdword.byte[0]&0x0f];						//�ٷ�λ
	        if((tempdword.byte[3]&0x80)==0x80) LcdDisBuf[5]&=0x7f;					//��ʾ��������99999��99,����ʾС����
	  	    lcd_bytes_display(0,LcdDisBuf,16,USERLCD);	     	 					//ͬʱ�ڲ���Ա����û�����ʾ
	  		lcd_bytes_display(0,LcdDisBuf,16,OPERLCD);
	        break;
	    case REMDIS:																//���Ѻ������ʾ����һ����ʾ�������ڶ���
			for(temp=0;temp<7;temp++) LcdDisBuf[temp]=0;							//DisDword��number(Bit7-Bit4)+DisData(28bits),BCD���ʽ
	        temp=tempdword.byte[3]&0x70;					 		 				//
	        if(temp==0x70) LcdDisBuf[0]=NumberTable[tempdword.byte[3]&0x0f]; 		//number=7,����λ��ʼ��ʾ,
	        if(temp>=0x60) LcdDisBuf[1]=NumberTable[(tempdword.byte[2]&0xf0)>>4];   //ǧ
			if(temp>=0x50) LcdDisBuf[2]=NumberTable[tempdword.byte[2]&0x0f];		//��
			if(temp>=0x40) LcdDisBuf[3]=NumberTable[(tempdword.byte[1]&0xf0)>>4];	//ʮ
			if(temp>=0x30) LcdDisBuf[4]=NumberTable[tempdword.byte[1]&0x0f];		//��
	        LcdDisBuf[5] =NumberTable[(tempdword.byte[0]&0xf0)>>4]|0x80; 			//ʮ��λ��ͬʱĬ��Ҫ����С����
			LcdDisBuf[6] =NumberTable[tempdword.byte[0]&0x0f];						//�ٷ�λ
	        if((tempdword.byte[3]&0x80)==0x80) LcdDisBuf[5]&=0x7f;					//��ʾ��������99999��99,����ʾС����
	  		lcd_bytes_display(0,LcdDisBuf,7,USERLCD);	     	 					//ͬʱ�ڲ���Ա����û�����ʾ
	  		lcd_bytes_display(0,LcdDisBuf,7,OPERLCD);
	        break;
	    case CSMDIS:																//����ǰ������ȷ�Ϻ����Ѻ�ʵ�����ѽ����ʾ���ڶ�����ʾ
			for(temp=7;temp<12;temp++) LcdDisBuf[temp]=0;							//DisDword��number(Bit7-Bit4)+DisData(28bits),BCD���ʽ
	        temp=tempdword.byte[3]&0x70;
	        if((tempdword.byte[3]&0x80)!=0x80)				 		 				//Ҫ��ʾ�Ľ��<1000.00
	        {
	           if(temp==0x50) LcdDisBuf[11]=NumberTable[tempdword.byte[2]&0x0f]; 	//number=5,�Ӱ�λ��ʼ��ʾ,
	           if(temp>=0x40) LcdDisBuf[10]=NumberTable[(tempdword.byte[1]&0xf0)>>4];		//ʮ
			   if(temp>=0x30) LcdDisBuf[9]=NumberTable[tempdword.byte[1]&0x0f];		//��
	           LcdDisBuf[8] =NumberTable[(tempdword.byte[0]&0xf0)>>4]|0x80; 		//ʮ��λ��ͬʱĬ��Ҫ����С����
			   LcdDisBuf[7] =NumberTable[tempdword.byte[0]&0x0f];					//�ٷ�λ
			}
		    else																	//Ҫ��ʾ�Ľ��>=1000����<=99999
		    {
			   if(temp==0x50) LcdDisBuf[11]=NumberTable[tempdword.byte[3]&0x0f]; 	//��λ
	           if(temp>=0x40) LcdDisBuf[10]=NumberTable[(tempdword.byte[2]&0xf0)>>4];  		//ǧλ
			   LcdDisBuf[9] =NumberTable[tempdword.byte[2]&0x0f];					//��
	           LcdDisBuf[8] =NumberTable[(tempdword.byte[1]&0xf0)>>4]; 				//ʮ��û��С����
			   LcdDisBuf[7] =NumberTable[tempdword.byte[1]&0x0f];					//��λ
	        }
			LcdDisBuf[12]=0;
			LcdDisBuf[13]=0;
			LcdDisBuf[14]=0xC1;														//����ʾ����������������ſ�����NKTY"
			LcdDisBuf[15]=0x40;
	  		lcd_bytes_display(14,&LcdDisBuf[7],9,USERLCD);	     	 				//ͬʱ�ڲ���Ա����û�����ʾ
	  		lcd_bytes_display(14,&LcdDisBuf[7],9,OPERLCD);
	        break;
	    case GROUPINPUT1:															//���鷽ʽ�£�ֻ�ܵ�һ����ʾ�������ڶ���
			for(temp=0;temp<7;temp++) LcdDisBuf[temp]=0;							//DisDword��groupnum(BYTE3)+number(BYTE2:Bit7-Bit4)+DisData(20bits),BCD���ʽ
	        LcdDisBuf[0]=NumberTable[tempdword.byte[3]&0x0f]; 						//��ʾ���
	        LcdDisBuf[1]=0x20;   													//������
	        temp=tempdword.byte[2]&0x70;
			if(temp==0x50) LcdDisBuf[2]=NumberTable[tempdword.byte[2]&0x0f];		//��
			if(temp>=0x40) LcdDisBuf[3]=NumberTable[(tempdword.byte[1]&0xf0)>>4];	//ʮ
			if(temp>=0x30) LcdDisBuf[4]=NumberTable[tempdword.byte[1]&0x0f];		//��
	        LcdDisBuf[5] =NumberTable[(tempdword.byte[0]&0xf0)>>4]|0x80; 			//ʮ��λ��ͬʱĬ��Ҫ����С����
			LcdDisBuf[6] =NumberTable[tempdword.byte[0]&0x0f];						//�ٷ�λ
	  		lcd_bytes_display(0,LcdDisBuf,7,USERLCD);	     	 					//ͬʱ�ڲ���Ա����û�����ʾ
	  		lcd_bytes_display(0,LcdDisBuf,7,OPERLCD);
	        break;
	    case GROUPINPUT2:															//���鷽ʽ�£��ڶ�����ʾ��ǰ�������ܽ��
			for(temp=7;temp<16;temp++) LcdDisBuf[temp]=0;							//DisDword��number(Bit7-Bit4)+DisData(28bits),BCD���ʽ
	        temp=tempdword.byte[3]&0x70;
	        if((tempdword.byte[3]&0x80)!=0x80)				 		 					//Ҫ��ʾ�Ľ��<1000.00
	        {
	           if(temp==0x50) LcdDisBuf[11]=NumberTable[tempdword.byte[2]&0x0f]; 		//number=5,�Ӱ�λ��ʼ��ʾ,
	           if(temp>=0x40) LcdDisBuf[10]=NumberTable[(tempdword.byte[1]&0xf0)>>4];	//ʮ
			   if(temp>=0x30) LcdDisBuf[9]=NumberTable[tempdword.byte[1]&0x0f];			//��
	           LcdDisBuf[8] =NumberTable[(tempdword.byte[0]&0xf0)>>4]|0x80; 			//ʮ��λ��ͬʱĬ��Ҫ����С����
			   LcdDisBuf[7] =NumberTable[tempdword.byte[0]&0x0f];						//�ٷ�λ
			}
		    else																		//Ҫ��ʾ�Ľ��>=1000����<=99999
		    {
			   if(temp==0x50) LcdDisBuf[11]=NumberTable[tempdword.byte[3]&0x0f]; 		//��λ
	           if(temp>=0x40) LcdDisBuf[10]=NumberTable[(tempdword.byte[2]&0xf0)>>4];  	//ǧλ
			   LcdDisBuf[9] =NumberTable[tempdword.byte[2]&0x0f];						//��
	           LcdDisBuf[8] =NumberTable[(tempdword.byte[1]&0xf0)>>4]; 					//ʮ��û��С����
			   LcdDisBuf[7] =NumberTable[tempdword.byte[1]&0x0f];						//��λ
	        }
	  		lcd_bytes_display(14,&LcdDisBuf[7],9,USERLCD);	     	 					//ͬʱ�ڲ���Ա����û�����ʾ
	  		lcd_bytes_display(14,&LcdDisBuf[7],9,OPERLCD);
	        break;
	    case SIMPLESET:  						//��Ʒ�ַ�ʽ�µ������ý�����˳�����,DisDword��unitprice(3Bytes),BCD���ʽ
	        LcdDisBuf[0]=0x74;					//F
	        LcdDisBuf[1]=0x03;					//1
	        LcdDisBuf[2]=0x20;					//-
	        LcdDisBuf[3]=0x00;					//SPACE
	        LcdDisBuf[4]=0x00;
	        LcdDisBuf[5]=0x00;
	        LcdDisBuf[6]=0x00;
	        LcdDisBuf[7] =NumberTable[tempdword.byte[0]&0x0f];
			LcdDisBuf[8] =NumberTable[(tempdword.byte[0]&0xf0)>>4]|0x80;
			LcdDisBuf[9] =NumberTable[tempdword.byte[1]&0x0f];
			LcdDisBuf[10]=NumberTable[(tempdword.byte[1]&0xf0)>>4];
			LcdDisBuf[11]=NumberTable[tempdword.byte[2]&0x0f];
	        LcdDisBuf[12]=0;
	  		LcdDisBuf[13]=0;
	        LcdDisBuf[14]=0;
	  		LcdDisBuf[15]=0;
	  		lcd_bytes_display(0,LcdDisBuf,16,USERLCD);	    	//ͬʱ�ڲ���Ա����û�����ʾ
	  		lcd_bytes_display(0,LcdDisBuf,16,OPERLCD);
	        break;
	    case MULTISET:  						//��Ʒ�ַ�ʽ�µ������ý�����棬DisDword��unitprice(3Bytes),BCD���ʽ
	        LcdDisBuf[0]=0x74;					//F
	        LcdDisBuf[1]=0x3e;					//2
	        LcdDisBuf[2]=0x20;					//-
			LcdDisBuf[3]=0x5f;					//0				//ֻ��ʾ��0����������
	        LcdDisBuf[4]=0x00;					//SPACE
	        LcdDisBuf[5]=0x00;
	        LcdDisBuf[6]=0x00;
	        LcdDisBuf[7] =NumberTable[tempdword.byte[0]&0x0f];
			LcdDisBuf[8] =NumberTable[(tempdword.byte[0]&0xf0)>>4]|0x80;
			LcdDisBuf[9] =NumberTable[tempdword.byte[1]&0x0f];
			LcdDisBuf[10]=NumberTable[(tempdword.byte[1]&0xf0)>>4];
			LcdDisBuf[11]=NumberTable[tempdword.byte[2]&0x0f];
	        LcdDisBuf[12]=0;
	  		LcdDisBuf[13]=0;
	        LcdDisBuf[14]=0;
	  		LcdDisBuf[15]=0;
	  		lcd_bytes_display(0,LcdDisBuf,16,USERLCD);	     		//ͬʱ�ڲ���Ա����û�����ʾ
	  		lcd_bytes_display(0,LcdDisBuf,16,OPERLCD);
	        break;
	    case ALOWSET:  												//������׼��ʾ��ͬ��ݷ�ʽ����ʾ
	        LcdDisBuf[0]=0x74;										//F
	        LcdDisBuf[1]=0x2F;										//3
	        LcdDisBuf[2]=0x20;										//-
			LcdDisBuf[3]=0x5f;										//ֻ��ʾ��0����������
	        LcdDisBuf[4]=0x00;										//SPACE
	        LcdDisBuf[5]=0x00;
	        LcdDisBuf[6]=0x00;
	        LcdDisBuf[7] =NumberTable[tempdword.byte[0]&0x0f];
			LcdDisBuf[8] =NumberTable[(tempdword.byte[0]&0xf0)>>4]|0x80;
			LcdDisBuf[9] =NumberTable[tempdword.byte[1]&0x0f];
			LcdDisBuf[10]=NumberTable[(tempdword.byte[1]&0xf0)>>4];
			LcdDisBuf[11]=NumberTable[tempdword.byte[2]&0x0f];
	        LcdDisBuf[12]=0;
	  		LcdDisBuf[13]=0;
	        LcdDisBuf[14]=0;
	  		LcdDisBuf[15]=0;
	  		lcd_bytes_display(0,LcdDisBuf,16,USERLCD);	     		 //ͬʱ�ڲ���Ա����û�����ʾ
	  		lcd_bytes_display(0,LcdDisBuf,16,OPERLCD);
	        break;
	    case OPENPRICE:  									 		 //��Ʒ�ֻ��Ʒ�ַ�ʽ�µ���������˸����1����������5��λ
	        LcdDisBuf[7] =NumberTable[tempdword.byte[0]&0x0f];  	 //DisDword��unitprice(3Bytes),BCD���ʽ
			LcdDisBuf[8] =NumberTable[(tempdword.byte[0]&0xf0)>>4]|0x80;
			LcdDisBuf[9] =NumberTable[tempdword.byte[1]&0x0f];
			LcdDisBuf[10]=NumberTable[(tempdword.byte[1]&0xf0)>>4];
			LcdDisBuf[11]=NumberTable[tempdword.byte[2]&0x0f];
	  		lcd_bytes_display(14,&LcdDisBuf[7],5,USERLCD);	     	 //ͬʱ�ڲ���Ա����û�����ʾ
	  		lcd_bytes_display(14,&LcdDisBuf[7],5,OPERLCD);  		 //14:��LCD��7λ��ʼ
	        break;
	    case CLOSEPRICE:  									  		 //��Ʒ�ֻ��Ʒ�ַ�ʽ�µ���������˸����2����δ����λ��ʹ��˸
	        temp=tempdword.byte[3]&0x7f;				 			 //DisDword��keynum(1Byte)+unitprice(3Bytes),BCD���ʽ
	        for(tt=5;tt>temp;tt--) {LcdDisBuf[12-tt]=0;} 			 //����keynum��ֵ������Ӧλ��ʾ��5��keynum)����keynum=2�����LCD�ϵ�7����8λ�͵�9λ����ʾ��keynum=3,��ص�7�͵�8λ��ʾ
	        if((tempdword.byte[3]&0x80)==0x80) LcdDisBuf[8]|=0x80; 	 //keynum���λ��С������ʾ����λ����1����ʾС����
	        else LcdDisBuf[8]&=0x7f;
	   		lcd_bytes_display(14,&LcdDisBuf[7],5-temp,USERLCD);	     //ͬʱ�ڲ���Ա����û�����ʾ
	  		lcd_bytes_display(14,&LcdDisBuf[7],5-temp,OPERLCD);  	 //14:��LCD��7λ��ʼ
	        break;
	    default:	break;
  	}//switch
}
/************************************************************************************************/


