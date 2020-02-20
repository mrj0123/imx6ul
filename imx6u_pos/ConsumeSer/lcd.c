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
const BYTE  NumberTable[25] ={0x5f,0x03,0x3e,0x2f,0x63,0x6d,0x7d,0x07,0x7f,0x6f,//LCD显示码表：0-9
							  0x77,0x79,0x5c,0x3b,0x7c,0x74,					//A-F
							  0X5D,0x73,0x58,0x76,0x5B,0x31,0x20,0x24,0x2c};	//G H L P U n - 二 三
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
 * 函数名称：BYTE lcd_bytes_display(BYTE DisStartAddr,BYTE *DataStartPtr,BYTE DataNum,BYTE LcdSlaveAddr)
 * 函数功能：任意字节数据显示，首先输入有关地址和命令，然后按顺序写入数据，
 *          PCF8576会自动进行地址累加（1：4多级驱动则以2累加）并显示
 * 执行条件：
 * 函数输入：（1）DisStartAddr显示的起始地址（此处必须为偶数）
 *          （2）*DataStartPtr通常为显示数组的首地址（3）DataNum将要显示的字节数
 *          （4）LcdSlaveAddr 选择操作员面或用户面
 * 函数输出：(1)0 正常写入 （2）非0 写入失败
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
 * 函数名称：BYTE lcd_byte_display(BYTE DisAddr,BYTE DisData,BYTE LcdSlaveAddr)
 * 函数功能：单字节数据显示，首先输入有关地址和命令，然后输入将显示数据，
 * 执行条件：
 * 函数输入：（1）DisAddr将显示数据的地址（此处必须为偶数）
 *          （2）DisData显示的数据	（3）LcdSlaveAddr选择操作员面或用户面
 * 函数输出：(1)0 正常写入 （2）非0 写入失败
 ***************************************************************************/
int lcd_byte_display(BYTE DisAddr,BYTE DisData,BYTE LcdSlaveAddr)
{
	u8 block[2] = {DisAddr, DisData};
	lcd_write(LcdSlaveAddr, sizeof(block), block);
	return 0;
}

/************************************************************************************************
 *函数名称：void Remind_info_display(BYTE Command,BYTE DisData)
 *函数功能：机器提示信息显示程序
 *执行条件：
 *函数输入：（1）command,显示类型（2）DisData显示的数据
 *函数输出：(无）
 *创建日期：2007/03/20
 *创建人：  Shaoping
 *备注：    （1）OPENBIT,CLOSEBIT中， DisData高4位是显示位，低四位是显示的数据
 ************************************************************************************************/
void Remind_info_display(BYTE Command,BYTE DisData)
{
  	BYTE temp,tt,flag=0;
  	WORD tempword;

  	switch(Command)
  	{
 		case OPENSECOND:  									 	//
        	lcd_byte_display(4,0x80,USERLCD);	 			 	//操作员面和用户面LCD同时显示
  			lcd_byte_display(4,0x80,OPERLCD);
        	break;
	   	case CLOSESECOND:  								  	 	//
	        lcd_byte_display(4,0,USERLCD);	 				 	//操作员面和用户面LCD同时显示
	  		lcd_byte_display(4,0,OPERLCD);
	        break;
	   	case TRADEREADY:  								  	 	//
	        lcd_byte_display(28,0x84,USERLCD);	 			 	//操作员面和用户面LCD同时显示”金额“和”请放卡“
	  		lcd_byte_display(28,0x84,OPERLCD);
	        break;
	   	case TRADEAGAIN:  								  	 	//
	        lcd_byte_display(28,0x88,USERLCD);	 			 	//操作员面和用户面LCD同时显示”金额“和”重放卡“
	  		lcd_byte_display(28,0x88,OPERLCD);
	        break;
	    case SCOLLDIS:											//开始滚动显示，此处显示第1位
			for(temp=0;temp<7;temp++) {LcdDisBuf[temp]=0;}		//DisData,高3BIT为显示位次，低5BIT为显示数，
	        LcdDisBuf[((DisData&0xE0)>>5)]=NumberTable[DisData&0x1f];
	        lcd_bytes_display(0,LcdDisBuf,7,USERLCD);	        //操作员面和用户面LCD同时显示	         //
	  		lcd_bytes_display(0,LcdDisBuf,7,OPERLCD);
	        break;
	    case ERRORNUM:		   									//显示错误编号,错误编号不作BCD转换，显示的为16进制
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
	  		lcd_bytes_display(0,LcdDisBuf,16,USERLCD); 			//操作员面和用户面LCD同时显示错误号
	  		lcd_bytes_display(0,LcdDisBuf,16,OPERLCD);
			BELLON;Delay_ms(600);
//			while(!PinRc500||PinNoGd)	{ClrWdt();}
			BELLOFF;
	        break;
	   	case OPERERROR:										 	//操作错误
	        LcdDisBuf[0]=0x7C;									//E
	        LcdDisBuf[1]=0x30;									//r
	        LcdDisBuf[2]=0x30;									//r
	        LcdDisBuf[3]=0x39;									//0
	        LcdDisBuf[4]=0xB0;									//操作错误和r
			for(temp=5;temp<16;temp++) LcdDisBuf[temp]=0;		//
	  		lcd_bytes_display(0,LcdDisBuf,16,USERLCD); 		 	//操作员面和用户面
	  		lcd_bytes_display(0,LcdDisBuf,16,OPERLCD);
			BELLON;Delay_ms(600);BELLOFF;
	        break;
	    case OPENBIT:											 //显示某一位，但显示内容只能是0－F，不能显示如“请放卡”等提示信息
	        temp=(DisData&0xf0)>>4;								 //高4BIT是显示的位次
	        LcdDisBuf[temp]=NumberTable[DisData&0x0f];
	        lcd_byte_display(temp+temp,LcdDisBuf[temp],USERLCD); //操作员面和用户面LCD同时显示
	  		lcd_byte_display(temp+temp,LcdDisBuf[temp],OPERLCD);
	        break;
	    case CLOSEBIT:											 //关闭某一位显示，和OPENBIT 配合形成闪烁的效果
			temp=(DisData&0xf0)>>4;								 //高4BIT是显示的位次
	        temp+=temp;
	        lcd_byte_display(temp,0,USERLCD);	 				 //操作员面和用户面LCD同时显示
	  		lcd_byte_display(temp,0,OPERLCD);
	        break;
	    case PSWDINPUT:											 //用于提示密码输入：显示“			”
			if(DisData==0)										 //2008、6、26，!=0时不清除第一行显示
				{for(temp=0;temp<7;temp++)  LcdDisBuf[temp]=0;}
	        LcdDisBuf[3]|=0x80;									 //“输密码”
	        for(temp=7;temp<16;temp++) LcdDisBuf[temp]=0;
	        lcd_bytes_display(0,LcdDisBuf,16,USERLCD);	         //操作员面和用户面LCD同时显示所有可显示内容
	  		lcd_bytes_display(0,LcdDisBuf,16,OPERLCD);
		    break;
	    case PSWDBIT:											 //显示"H",DisData此处是显示的位，只有第12和13位是”米“字，显示特殊
	        if(DisData==12)
		    {
			 	LcdDisBuf[12]=0x33;
	  		 	LcdDisBuf[13]=0x20;
		     	lcd_bytes_display(24,&LcdDisBuf[12],2,USERLCD); //操作员面和用户面LCD同时显示所有可显示内容
	  		 	lcd_bytes_display(24,&LcdDisBuf[12],2,OPERLCD);
		    }
			else												//根据DisData提供的显示位次，在相应位显示H
			{
	         	lcd_byte_display(DisData<<1,0x73,USERLCD); 		//操作员面和用户面LCD同时显示
	  		 	lcd_byte_display(DisData<<1,0x73,OPERLCD);
			}
	        break;
	    case EXCDHOUR:		   	        						//显示黑名单未更新小时数
	        LcdDisBuf[0]=0x7C;									//E
	        LcdDisBuf[1]=0x38;									//c
	        LcdDisBuf[2]=0x3b;									//d
	        LcdDisBuf[3]=0x20;									//-
	        LcdDisBuf[4]=0x00;
	        LcdDisBuf[5]=0x00;
	        LcdDisBuf[6]=0x00;
	        LcdDisBuf[7]=0x00;
	        LcdDisBuf[8]=0x00;
	        tempword=Hex_bcd(DisData);							//HEX码转成BCD码显示
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
	  		lcd_bytes_display(0,LcdDisBuf,16,USERLCD);    		//操作员面和用户面LCD同时显示
	  		lcd_bytes_display(0,LcdDisBuf,16,OPERLCD);
	        break;
	     case NUMADVER:									     	//显示机器号和机器程序版本号，
	  	    LcdDisBuf[0]=0x00;
	        LcdDisBuf[1]=0x31;								 	//"n"
			LcdDisBuf[2]=NumberTable[MachNoDis.sht[1]&0x0f];	//MachNoDis.lng专用于机器号显示，是RealMachNo的BCD码
			LcdDisBuf[3]=NumberTable[MachNoDis.sht[0]>>4];
			LcdDisBuf[4]=NumberTable[MachNoDis.sht[0]&0x0f];
	  		LcdDisBuf[5]=0;
	  		LcdDisBuf[6]=0;
			LcdDisBuf[11]=0;
			LcdDisBuf[10]=NumberTable[(BYTE)((VerNo>>12)&0x0f)]; //版本号是固定的2字节显示码，建议为0-9
			LcdDisBuf[9]=NumberTable[(BYTE)((VerNo>>8)&0x0f)];
			LcdDisBuf[8]=NumberTable[(BYTE)((VerNo>>4)&0x0f)];
			LcdDisBuf[7]=NumberTable[(BYTE)(VerNo&0x0f)];
	        LcdDisBuf[12]=0x84;							 	 	//"X"
	  		LcdDisBuf[13]=0x05;
	  		LcdDisBuf[14]=0;
	  		LcdDisBuf[15]=0;
	  		lcd_bytes_display(0,LcdDisBuf,16,USERLCD);	     	//同时在操作员面和用户面显示
	  		lcd_bytes_display(0,LcdDisBuf,16,OPERLCD);
	        break;
	     case MACHSTAT:											//机器启动时使用，首先逐段显示”米“，然后全部显示
	        for(temp=0;temp<16;temp++)   LcdDisBuf[temp]=0;
	        lcd_bytes_display(0,LcdDisBuf,16,USERLCD);	        //同时清除两面Lcd所有可显示内容
	        lcd_bytes_display(0,LcdDisBuf,16,OPERLCD);
	        LcdDisBuf[0]=0x20;  							    //开始逐段点亮"米“字
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
	        Delay_ms(200);										 	//”米“字逐段显示结束
	        for(temp=0;temp<16;temp++) {LcdDisBuf[temp]=0xff;}
	        lcd_bytes_display(0,LcdDisBuf,16,USERLCD);	         	//操作员面和用户面LCD同时显示所有可显示内容
	  		lcd_bytes_display(0,LcdDisBuf,16,OPERLCD);
	        break;
	     case FULLSCRN:												//全屏显示，即每个位显示相同的数据，注意DisData直接就是显示码
	        for(temp=0;temp<16;temp++) {LcdDisBuf[temp]=DisData;}	//清全部显示或使所有内容显示
	        lcd_bytes_display(0,LcdDisBuf,16,USERLCD);	         	//操作员面和用户面LCD同时显示	         //
	  		lcd_bytes_display(0,LcdDisBuf,16,OPERLCD);
	        break;
	     case BADERROR:												//严重错误状态显示，静止显示全0至全F,注意DisData不是显示码
	        DisData=NumberTable[DisData&0x1F];						//DisData<0x1f，
	        for(temp=0;temp<7;temp++)  {LcdDisBuf[temp]=DisData;} 	//字段处显示，如全1
	        for(temp=7;temp<16;temp++) {LcdDisBuf[temp]=0;} 		//不显示
	        lcd_bytes_display(0,LcdDisBuf,16,USERLCD);	         	//操作员面和用户面LCD同时显示	         //
	  		lcd_bytes_display(0,LcdDisBuf,16,OPERLCD);
		    //CLOSEINT;
			temp=1;
			//while(temp)		{ClrWdt();}								//只清看门狗，防止重启
	        break;
	     case RADDIS:												//放射状显示，高3Bits是显示位数，低5Bits是显示内容
			tt=(DisData&0xE0)>>5;
			tt++;
			for(temp=0;temp<tt;temp++) {LcdDisBuf[temp]=NumberTable[DisData&0x1F];}
	   		for(temp=tt;temp<7;temp++) {LcdDisBuf[temp]=0;}
	        lcd_bytes_display(0,LcdDisBuf,7,USERLCD);	         	//操作员面和用户面LCD同时显示	         //
	  		lcd_bytes_display(0,LcdDisBuf,7,OPERLCD);
	 		break;
	    default:break;
  	}//switch
}

//开机显示版本号
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
    if(1){LcdDisBuf[12]|=0x87;LcdDisBuf[13]|=0x35;}		//没有通讯信号,显示“米”字
    //if(0){LcdDisBuf[12]|=0x78;LcdDisBuf[13]|=0xCA;}	//有积压流水，显示“口”
    //LcdDisBuf[15]=0x40;
    LcdDisBuf[14]=0;		  												//共显示：“金额”“现余额”“请放卡”“NKTY"
    LcdDisBuf[15]=0x40;

    lcd_bytes_display(0,LcdDisBuf,16,USERLCD);	     	     //同时在操作员面和用户面显示
    lcd_bytes_display(0,LcdDisBuf,16,OPERLCD);  		     //
}

void SerialNo_display()
{

}

//显示当前时间，秒为单数，显示:，秒为双数，不显示:
void DateTime_display()
{
    //获取当前时间
    time_t timer;//time_t就是long int 类型
    struct tm *tblock;
    timer = time(NULL);
    tblock = localtime(&timer);
    memset(LcdDisBuf,0,16);
    //myPtf("Local time is: %02d-%02d %02d:%02d\n", tblock->tm_mon+1, tblock->tm_mday, tblock->tm_hour, tblock->tm_min);
    ////////////////////////////////////////////////
    //显示时间
    LcdDisBuf[0]=NumberTable[tblock->tm_hour/10];
    LcdDisBuf[1]=NumberTable[tblock->tm_hour%10];
    LcdDisBuf[2]=(tblock->tm_sec%2)<<7;
    LcdDisBuf[3]=NumberTable[tblock->tm_min/10];
    LcdDisBuf[4]=NumberTable[tblock->tm_min%10];
    ////////////////////////////////////////////////
    //显示日期
    LcdDisBuf[11]=NumberTable[(tblock->tm_mon+1)/10];
    LcdDisBuf[10]=NumberTable[(tblock->tm_mon+1)%10];
    LcdDisBuf[9]=0x20;
    LcdDisBuf[8]=NumberTable[tblock->tm_mday/10];
    LcdDisBuf[7]=NumberTable[tblock->tm_mday%10];
    LcdDisBuf[15]=0x40; //图标

    //同时在操作员面和用户面显示
    lcd_bytes_display(0,LcdDisBuf,16,USERLCD);
    lcd_bytes_display(0,LcdDisBuf,16,OPERLCD);
}
//显示输入金额
void InputMoney_display(int InputMoney)
{
    /*
    memset(LcdDisBuf,0,16);
    //显示PAUSE
    LcdDisBuf[0]=0x76;
    LcdDisBuf[1]=0x77;
    LcdDisBuf[2]=0x5B;
    LcdDisBuf[3]=0x6d;
    LcdDisBuf[4]=0x7c;
    */
    //获取当前时间
    time_t timer;//time_t就是long int 类型
    struct tm *tblock;
    timer = time(NULL);
    tblock = localtime(&timer);
    memset(LcdDisBuf,0,16);
    //myPtf("Local time is: %02d-%02d %02d:%02d\n", tblock->tm_mon+1, tblock->tm_mday, tblock->tm_hour, tblock->tm_min);
    ////////////////////////////////////////////////
    //显示时间
    LcdDisBuf[0]=NumberTable[tblock->tm_hour/10];
    LcdDisBuf[1]=NumberTable[tblock->tm_hour%10];
    LcdDisBuf[2]=(tblock->tm_sec%2)<<7;
    LcdDisBuf[3]=NumberTable[tblock->tm_min/10];
    LcdDisBuf[4]=NumberTable[tblock->tm_min%10];
    ////////////////////////////////////////////////
    //显示金额
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
    //如果数据小于1元，需要在元位填0
    if (InputMoney<100)
    {
        LcdDisBuf[9]=NumberTable[0];
        //如果数据小于1角，需要在角位填0
        if (InputMoney<10)
        {
            LcdDisBuf[8]=NumberTable[0];
        }
    }
    LcdDisBuf[8]|=0x80;

    LcdDisBuf[14]=0x80; //金额
    LcdDisBuf[15]=0x40; //图标
    //同时在操作员面和用户面显示
    lcd_bytes_display(0,LcdDisBuf,16,USERLCD);
    lcd_bytes_display(0,LcdDisBuf,16,OPERLCD);
}

//滚0等待刷卡 ZeroPos表示显示滚0的位置（0~4）
void WaitForCard_display(int InputMoney,int ZeroPos)
{
    memset(LcdDisBuf,0,16);
    //找到显示0的位置
    LcdDisBuf[ZeroPos]=NumberTable[0];
    ////////////////////////////////////////////////
    //显示金额
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
    //如果数据小于1元，需要在元位填0
    if (InputMoney<100)
    {
        LcdDisBuf[9]=NumberTable[0];
        //如果数据小于1角，需要在角位填0
        if (InputMoney<10)
        {
            LcdDisBuf[8]=NumberTable[0];
        }
    }
    LcdDisBuf[8]|=0x80;


    LcdDisBuf[14]=0x80|0x04; //金额+请放卡
    LcdDisBuf[15]=0x40; //图标
    //同时在操作员面和用户面显示
    lcd_bytes_display(0,LcdDisBuf,16,USERLCD);
    lcd_bytes_display(0,LcdDisBuf,16,OPERLCD);
}

//消费成功，第一排显示RemainMoney表示余额，第二排ConsumeMoney表示消费额
void ConsumeSuccess_display(int RemainMoney,int ConsumeMoney)
{
    memset(LcdDisBuf,0,16);
    //显示余额
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
    //如果数据小于1元，需要在元位填0
    if (RemainMoney<100)
    {
        LcdDisBuf[4]=NumberTable[0];
        //如果数据小于1角，需要在角位填0
        if (RemainMoney<10)
        {
            LcdDisBuf[5]=NumberTable[0];
        }
    }
    LcdDisBuf[5]|=0x80;

    ////////////////////////////////////////////////
    //显示消费额
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
    //如果数据小于1元，需要在元位填0
    if (ConsumeMoney<100)
    {
        LcdDisBuf[9]=NumberTable[0];
        //如果数据小于1角，需要在角位填0
        if (ConsumeMoney<10)
        {
            LcdDisBuf[8]=NumberTable[0];
        }
    }
    LcdDisBuf[8]|=0x80;


    LcdDisBuf[14]=0x80|0x40|0x01; //金额+余额+请退卡
    LcdDisBuf[15]=0x40; //图标
    //同时在操作员面和用户面显示
    lcd_bytes_display(0,LcdDisBuf,16,USERLCD);
    lcd_bytes_display(0,LcdDisBuf,16,OPERLCD);
}

//消费失败，错误提示
void ErrorCode_display(int Errorcode)
{
    memset(LcdDisBuf,0,16);
    //显示错误码
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
    //LcdDisBuf[14]=0x80|0x40|0x01; //金额+余额+请退卡
    LcdDisBuf[15]=0x40; //图标
    //同时在操作员面和用户面显示
    lcd_bytes_display(0,LcdDisBuf,16,USERLCD);
    lcd_bytes_display(0,LcdDisBuf,16,OPERLCD);
}










/************************************************************************************************
 *函数名称：void Multi_word_display(BYTE Command,DWORD DisDword)
 *函数功能：多功能信息显示程序，显示时间、键盘输入等
 *执行条件：
 *函数输入：
 *函数输出：(无）
 *创建日期：2007/03/20
 *创建人：  Shaoping
 *备注：    四字节的显示信息DisDword，会根据显示内容的不同，其各字节的含义也不同，注意区分
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
	    case NORMALDS:  									 		 //系统空闲时正常显示，显示当前时间和操作方式，DisDword为BCD码格式
/*
	        LcdDisBuf[4] =NumberTable[tempdword.byte[0]&0x0f];  	 //DisDword＝null(1Byte)+opermode(1Byte)+hour(1Byte)+minute(1Byte),
			LcdDisBuf[3] =NumberTable[(tempdword.byte[0]&0xf0)>>4];
	        LcdDisBuf[2] =0x80;										 //点亮冒号，即秒显示
			LcdDisBuf[1] =NumberTable[tempdword.byte[1]&0x0f];
			LcdDisBuf[0] =NumberTable[(tempdword.byte[1]&0xf0)>>4];
			LcdDisBuf[5] =0x74;										 //"F"
			LcdDisBuf[6] =NumberTable[tempdword.byte[2]&0x0f];		 //操作方式显示
	        LcdDisBuf[10]=0x31;								 		 //"n"
			LcdDisBuf[9]=NumberTable[MachNoDis.sht[1]&0x0f];
			LcdDisBuf[8]=NumberTable[MachNoDis.sht[0]>>4];
			LcdDisBuf[7]=NumberTable[MachNoDis.sht[0]&0x0f];
*/
			LcdDisBuf[0] =NumberTable[(tempdword.byte[1]&0xf0)>>4];
			myPtf("LcdDisBuf[0]=%02X,(tempdword.byte[1]&0xf0)>>4=%02X\n",LcdDisBuf[0],(tempdword.byte[1]&0xf0)>>4);
			LcdDisBuf[1] =NumberTable[tempdword.byte[1]&0x0f];
			myPtf("LcdDisBuf[1]=%02X,tempdword.byte[1]&0x0f=%02X\n",LcdDisBuf[1],tempdword.byte[1]&0x0f);
	        LcdDisBuf[2] =0x80;										 //点亮冒号，即秒显示
			LcdDisBuf[3] =NumberTable[(tempdword.byte[0]&0xf0)>>4];
			myPtf("LcdDisBuf[3]=%02X,(tempdword.byte[0]&0xf0)>>4=%02X\n",LcdDisBuf[3],(tempdword.byte[0]&0xf0)>>4);
	        LcdDisBuf[4] =NumberTable[tempdword.byte[0]&0x0f];  	 //DisDword＝null(1Byte)+opermode(1Byte)+hour(1Byte)+minute(1Byte),
			myPtf("LcdDisBuf[4]=%02X,tempdword.byte[0]&0x0f=%02X\n",LcdDisBuf[4],tempdword.byte[0]&0x0f);
			LcdDisBuf[5] =0x74|0x80;										 //"F"
			LcdDisBuf[6] =NumberTable[tempdword.byte[2]&0x0f];		 //操作方式显示
			myPtf("LcdDisBuf[6]=%02X,tempdword.byte[2]&0x0f=%02X\n",LcdDisBuf[6],tempdword.byte[2]&0x0f);
			/////////////////////////////////////////////////////////////
			LcdDisBuf[7]=NumberTable[MachNoDis.sht[0]&0x0f];        //米字
			myPtf("LcdDisBuf[7]=%02X,MachNoDis.sht[0]&0x0f=%02X\n",LcdDisBuf[7],MachNoDis.sht[0]&0x0f);
			LcdDisBuf[8]=NumberTable[MachNoDis.sht[0]>>4]|0x80;          //二排第一个字符
			myPtf("LcdDisBuf[8]=%02X,MachNoDis.sht[0]>>4=%02X\n",LcdDisBuf[8],MachNoDis.sht[0]>>4);
			LcdDisBuf[9]=NumberTable[MachNoDis.sht[1]&0x0f];
			myPtf("LcdDisBuf[9]=%02X,MachNoDis.sht[1]&0x0f=%02X\n",LcdDisBuf[9],MachNoDis.sht[1]&0x0f);
	        LcdDisBuf[10]=0x31;								 		 //"n"

	        for(temp=11;temp<15;temp++) LcdDisBuf[temp]=0;
//			if(CommIndt==0) 	 {LcdDisBuf[12]|=0x87;LcdDisBuf[13]|=0x35;}		//没有通讯信号,显示“米”字
			if(1){LcdDisBuf[12]|=0xFF;LcdDisBuf[13]|=0xFF;}		//没有通讯信号,显示“米”字
			if(0){LcdDisBuf[12]|=0x78;LcdDisBuf[13]|=0xCA;}		//有积压流水，显示“口”
			//LcdDisBuf[15]=0x40;
            LcdDisBuf[14]=0xC1;		  												//共显示：“金额”“现余额”“请放卡”“NKTY"
			LcdDisBuf[15]=DisDword;
			//LcdDisBuf[15]=0x40;

	  		lcd_bytes_display(0,LcdDisBuf,16,USERLCD);	     	     //同时在操作员面和用户面显示
	  		lcd_bytes_display(0,LcdDisBuf,16,OPERLCD);  		     //
	        break;
	    case INPUTMONEY:															//第一行显示，清第二行
			for(temp=0;temp<16;temp++) LcdDisBuf[temp]=0;							//DisDword＝number(Bit7-Bit4)+DisData(28bits),BCD码格式
	        temp=tempdword.byte[3]&0x70;					 		 				//
	        if(temp==0x70) LcdDisBuf[0]=NumberTable[tempdword.byte[3]&0x0f]; 		//number=7,从万位开始显示,
	        if(temp>=0x60) LcdDisBuf[1]=NumberTable[(tempdword.byte[2]&0xf0)>>4];   //千
			if(temp>=0x50) LcdDisBuf[2]=NumberTable[tempdword.byte[2]&0x0f];		//百
			if(temp>=0x40) LcdDisBuf[3]=NumberTable[(tempdword.byte[1]&0xf0)>>4];	//十
			if(temp>=0x30) LcdDisBuf[4]=NumberTable[tempdword.byte[1]&0x0f];		//个
	        LcdDisBuf[5] =NumberTable[(tempdword.byte[0]&0xf0)>>4]|0x80; 			//十分位，同时默认要加上小数点
			LcdDisBuf[6] =NumberTable[tempdword.byte[0]&0x0f];						//百分位
	        if((tempdword.byte[3]&0x80)==0x80) LcdDisBuf[5]&=0x7f;					//显示的数超过99999。99,不显示小数点
	  	    lcd_bytes_display(0,LcdDisBuf,16,USERLCD);	     	 					//同时在操作员面和用户面显示
	  		lcd_bytes_display(0,LcdDisBuf,16,OPERLCD);
	        break;
	    case REMDIS:																//消费后余额显示，第一行显示，不动第二行
			for(temp=0;temp<7;temp++) LcdDisBuf[temp]=0;							//DisDword＝number(Bit7-Bit4)+DisData(28bits),BCD码格式
	        temp=tempdword.byte[3]&0x70;					 		 				//
	        if(temp==0x70) LcdDisBuf[0]=NumberTable[tempdword.byte[3]&0x0f]; 		//number=7,从万位开始显示,
	        if(temp>=0x60) LcdDisBuf[1]=NumberTable[(tempdword.byte[2]&0xf0)>>4];   //千
			if(temp>=0x50) LcdDisBuf[2]=NumberTable[tempdword.byte[2]&0x0f];		//百
			if(temp>=0x40) LcdDisBuf[3]=NumberTable[(tempdword.byte[1]&0xf0)>>4];	//十
			if(temp>=0x30) LcdDisBuf[4]=NumberTable[tempdword.byte[1]&0x0f];		//个
	        LcdDisBuf[5] =NumberTable[(tempdword.byte[0]&0xf0)>>4]|0x80; 			//十分位，同时默认要加上小数点
			LcdDisBuf[6] =NumberTable[tempdword.byte[0]&0x0f];						//百分位
	        if((tempdword.byte[3]&0x80)==0x80) LcdDisBuf[5]&=0x7f;					//显示的数超过99999。99,不显示小数点
	  		lcd_bytes_display(0,LcdDisBuf,7,USERLCD);	     	 					//同时在操作员面和用户面显示
	  		lcd_bytes_display(0,LcdDisBuf,7,OPERLCD);
	        break;
	    case CSMDIS:																//消费前输入金额确认和消费后实际消费金额显示，第二行显示
			for(temp=7;temp<12;temp++) LcdDisBuf[temp]=0;							//DisDword＝number(Bit7-Bit4)+DisData(28bits),BCD码格式
	        temp=tempdword.byte[3]&0x70;
	        if((tempdword.byte[3]&0x80)!=0x80)				 		 				//要显示的金额<1000.00
	        {
	           if(temp==0x50) LcdDisBuf[11]=NumberTable[tempdword.byte[2]&0x0f]; 	//number=5,从百位开始显示,
	           if(temp>=0x40) LcdDisBuf[10]=NumberTable[(tempdword.byte[1]&0xf0)>>4];		//十
			   if(temp>=0x30) LcdDisBuf[9]=NumberTable[tempdword.byte[1]&0x0f];		//个
	           LcdDisBuf[8] =NumberTable[(tempdword.byte[0]&0xf0)>>4]|0x80; 		//十分位，同时默认要加上小数点
			   LcdDisBuf[7] =NumberTable[tempdword.byte[0]&0x0f];					//百分位
			}
		    else																	//要显示的金额>=1000，但<=99999
		    {
			   if(temp==0x50) LcdDisBuf[11]=NumberTable[tempdword.byte[3]&0x0f]; 	//万位
	           if(temp>=0x40) LcdDisBuf[10]=NumberTable[(tempdword.byte[2]&0xf0)>>4];  		//千位
			   LcdDisBuf[9] =NumberTable[tempdword.byte[2]&0x0f];					//百
	           LcdDisBuf[8] =NumberTable[(tempdword.byte[1]&0xf0)>>4]; 				//十，没有小数点
			   LcdDisBuf[7] =NumberTable[tempdword.byte[1]&0x0f];					//个位
	        }
			LcdDisBuf[12]=0;
			LcdDisBuf[13]=0;
			LcdDisBuf[14]=0xC1;														//共显示：“金额”“现余额”“请放卡”“NKTY"
			LcdDisBuf[15]=0x40;
	  		lcd_bytes_display(14,&LcdDisBuf[7],9,USERLCD);	     	 				//同时在操作员面和用户面显示
	  		lcd_bytes_display(14,&LcdDisBuf[7],9,OPERLCD);
	        break;
	    case GROUPINPUT1:															//分组方式下，只管第一行显示，不动第二行
			for(temp=0;temp<7;temp++) LcdDisBuf[temp]=0;							//DisDword＝groupnum(BYTE3)+number(BYTE2:Bit7-Bit4)+DisData(20bits),BCD码格式
	        LcdDisBuf[0]=NumberTable[tempdword.byte[3]&0x0f]; 						//显示组号
	        LcdDisBuf[1]=0x20;   													//“－”
	        temp=tempdword.byte[2]&0x70;
			if(temp==0x50) LcdDisBuf[2]=NumberTable[tempdword.byte[2]&0x0f];		//百
			if(temp>=0x40) LcdDisBuf[3]=NumberTable[(tempdword.byte[1]&0xf0)>>4];	//十
			if(temp>=0x30) LcdDisBuf[4]=NumberTable[tempdword.byte[1]&0x0f];		//个
	        LcdDisBuf[5] =NumberTable[(tempdword.byte[0]&0xf0)>>4]|0x80; 			//十分位，同时默认要加上小数点
			LcdDisBuf[6] =NumberTable[tempdword.byte[0]&0x0f];						//百分位
	  		lcd_bytes_display(0,LcdDisBuf,7,USERLCD);	     	 					//同时在操作员面和用户面显示
	  		lcd_bytes_display(0,LcdDisBuf,7,OPERLCD);
	        break;
	    case GROUPINPUT2:															//分组方式下，第二行显示当前已输入总金额
			for(temp=7;temp<16;temp++) LcdDisBuf[temp]=0;							//DisDword＝number(Bit7-Bit4)+DisData(28bits),BCD码格式
	        temp=tempdword.byte[3]&0x70;
	        if((tempdword.byte[3]&0x80)!=0x80)				 		 					//要显示的金额<1000.00
	        {
	           if(temp==0x50) LcdDisBuf[11]=NumberTable[tempdword.byte[2]&0x0f]; 		//number=5,从百位开始显示,
	           if(temp>=0x40) LcdDisBuf[10]=NumberTable[(tempdword.byte[1]&0xf0)>>4];	//十
			   if(temp>=0x30) LcdDisBuf[9]=NumberTable[tempdword.byte[1]&0x0f];			//个
	           LcdDisBuf[8] =NumberTable[(tempdword.byte[0]&0xf0)>>4]|0x80; 			//十分位，同时默认要加上小数点
			   LcdDisBuf[7] =NumberTable[tempdword.byte[0]&0x0f];						//百分位
			}
		    else																		//要显示的金额>=1000，但<=99999
		    {
			   if(temp==0x50) LcdDisBuf[11]=NumberTable[tempdword.byte[3]&0x0f]; 		//万位
	           if(temp>=0x40) LcdDisBuf[10]=NumberTable[(tempdword.byte[2]&0xf0)>>4];  	//千位
			   LcdDisBuf[9] =NumberTable[tempdword.byte[2]&0x0f];						//百
	           LcdDisBuf[8] =NumberTable[(tempdword.byte[1]&0xf0)>>4]; 					//十，没有小数点
			   LcdDisBuf[7] =NumberTable[tempdword.byte[1]&0x0f];						//个位
	        }
	  		lcd_bytes_display(14,&LcdDisBuf[7],9,USERLCD);	     	 					//同时在操作员面和用户面显示
	  		lcd_bytes_display(14,&LcdDisBuf[7],9,OPERLCD);
	        break;
	    case SIMPLESET:  						//单品种方式下单价设置进入和退出界面,DisDword＝unitprice(3Bytes),BCD码格式
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
	  		lcd_bytes_display(0,LcdDisBuf,16,USERLCD);	    	//同时在操作员面和用户面显示
	  		lcd_bytes_display(0,LcdDisBuf,16,OPERLCD);
	        break;
	    case MULTISET:  						//多品种方式下单价设置进入界面，DisDword＝unitprice(3Bytes),BCD码格式
	        LcdDisBuf[0]=0x74;					//F
	        LcdDisBuf[1]=0x3e;					//2
	        LcdDisBuf[2]=0x20;					//-
			LcdDisBuf[3]=0x5f;					//0				//只显示“0”键代表金额
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
	  		lcd_bytes_display(0,LcdDisBuf,16,USERLCD);	     		//同时在操作员面和用户面显示
	  		lcd_bytes_display(0,LcdDisBuf,16,OPERLCD);
	        break;
	    case ALOWSET:  												//补贴标准显示，同快捷方式下显示
	        LcdDisBuf[0]=0x74;										//F
	        LcdDisBuf[1]=0x2F;										//3
	        LcdDisBuf[2]=0x20;										//-
			LcdDisBuf[3]=0x5f;										//只显示“0”键代表金额
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
	  		lcd_bytes_display(0,LcdDisBuf,16,USERLCD);	     		 //同时在操作员面和用户面显示
	  		lcd_bytes_display(0,LcdDisBuf,16,OPERLCD);
	        break;
	    case OPENPRICE:  									 		 //单品种或多品种方式下单价设置闪烁界面1：点亮所有5个位
	        LcdDisBuf[7] =NumberTable[tempdword.byte[0]&0x0f];  	 //DisDword＝unitprice(3Bytes),BCD码格式
			LcdDisBuf[8] =NumberTable[(tempdword.byte[0]&0xf0)>>4]|0x80;
			LcdDisBuf[9] =NumberTable[tempdword.byte[1]&0x0f];
			LcdDisBuf[10]=NumberTable[(tempdword.byte[1]&0xf0)>>4];
			LcdDisBuf[11]=NumberTable[tempdword.byte[2]&0x0f];
	  		lcd_bytes_display(14,&LcdDisBuf[7],5,USERLCD);	     	 //同时在操作员面和用户面显示
	  		lcd_bytes_display(14,&LcdDisBuf[7],5,OPERLCD);  		 //14:从LCD第7位开始
	        break;
	    case CLOSEPRICE:  									  		 //单品种或多品种方式下单价设置闪烁界面2：清未输入位，使闪烁
	        temp=tempdword.byte[3]&0x7f;				 			 //DisDword＝keynum(1Byte)+unitprice(3Bytes),BCD码格式
	        for(tt=5;tt>temp;tt--) {LcdDisBuf[12-tt]=0;} 			 //根据keynum的值，关相应位显示（5－keynum)，如keynum=2，则关LCD上第7、第8位和第9位的显示，keynum=3,则关第7和第8位显示
	        if((tempdword.byte[3]&0x80)==0x80) LcdDisBuf[8]|=0x80; 	 //keynum最高位是小数点显示控制位，＝1则显示小数点
	        else LcdDisBuf[8]&=0x7f;
	   		lcd_bytes_display(14,&LcdDisBuf[7],5-temp,USERLCD);	     //同时在操作员面和用户面显示
	  		lcd_bytes_display(14,&LcdDisBuf[7],5-temp,OPERLCD);  	 //14:从LCD第7位开始
	        break;
	    default:	break;
  	}//switch
}
/************************************************************************************************/


