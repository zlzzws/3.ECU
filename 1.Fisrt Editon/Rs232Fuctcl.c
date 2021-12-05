/**********************************************************************
*File name      :   Rs232Fuctcl.c
*Copyright      :   Zhuzhou CRRC Times Electric Co. Ltd ALL Rights Reserved
*Creat Date     :   2021/9/9
*Description    :   uartport control function
*Version        :   REV1.0.0       
*Author:        :   feng
*
*History:
*REV1.0.0     feng    2021/9/9 Create
*********************************************************************/

/**********************************************************************
*
*Debug switch  section
*
*********************************************************************/


/**********************************************************************
*
*Include file  section
*
*********************************************************************/
#include "Rs232Fuctcl.h"
/**********************************************************************
*
*Global Variable Declare Section
*

*********************************************************************/
extern DEBUG_TYPE_ENUM g_DebugType_EU;
extern BYTE_BIT g_VoiceFinger_ErrInfo;

/**********************************************************************
*
*Local Macro Define Section
*
*********************************************************************/

/**********************************************************************
*
*Local Struct Define Section
*
*********************************************************************/
static uint8_t s_UartRecvErrNum =0;
static uint8_t s_UartRecvErrFlag =0;
/**********************************************************************
*
*Local Prototype Declare Section
*
*********************************************************************/

/**********************************************************************
*
*Static Variable Define Section
*
*********************************************************************/


/**********************************************************************
*
*Function Define Section
*
*********************************************************************/
   
/******************************************************************* 
* 名称：    UART0_Open 
* 功能：    打开串口并返回串口设备文件描述 
* 入口参数：port ：串口设备文件 "/dev/ttyPS1"
* 出口参数：fd ：文件描述符 
*******************************************************************/  
int UART_Open(char* port) 
{  
    int fd;
	fd = open( port, O_RDWR|O_NOCTTY|O_NDELAY);  
	if (FALSE == fd)  
	{  
		perror("Can't Open Serial Port");  
		return(FALSE);  
	}  
	//恢复阻塞模式	 	                          
	if(fcntl(fd,F_SETFL,0) < 0)  
	{  
		printf("fcntl failed!\n");  
		return(FALSE);  
	}       
	else  
	{  
		printf("fcntl=%d\n",fcntl(fd, F_SETFL,0)); 
	} 
	//测试是否为终端设备      
	if(0 == isatty(STDIN_FILENO))  
	{  
		printf("standard input is not a terminal device\n");  
		return(FALSE);  
	}  
	else  
	{  
		printf("isatty success!\n");  
	}                
	printf("fd->open=%d\n",fd);  
	return fd;  
}  
/******************************************************************* 
* 名称：            UART0_Close 
* 功能：            关闭串口并返回串口设备文件描述 
* 入口参数：        fd :文件描述符  
* 出口参数：        void 
*******************************************************************/  
   
void UART_Close(int fd)  
{  
	close(fd);  
}  
   
/******************************************************************* 
* 名称：            UART0_Set 
* 功能：            设置串口数据位，停止位和效验位 
* 入口参数：       	fd:串口文件描述符 speed:串口速度 
*                  flow_ctrl:数据流控制 
*                  databits:数据位 取值为 7 或者8 
*                  stopbits:停止位 取值为 1 或者2 
*                  parity:效验类型 取值为N,E,O,,S 
*出口参数：        正确返回为1，错误返回为0 
*******************************************************************/  
int UART_Set(int fd,int speed,int flow_ctrl,int databits,int stopbits,int parity)  
{  
     
	int   i;  
	int   status;  
	int   speed_arr[]={ B115200, B19200, B9600, B4800, B2400, B1200, B300};  
	int   name_arr[] = {115200,  19200,  9600,  4800,  2400,  1200,  300};  
           
	struct termios options;  
     
	/*
	*tcgetattr(fd,&options)得到与fd指向对象的相关参数，并将它们保存于options,
	*该函数还可以测试配置是否正确，该串口是否可用等。
	*若调用成功，函数返回值为0，若调用失败，函数返回值为1.
	*/  
	if( tcgetattr( fd,&options)  !=  0)  
	{  
		perror("SetupSerial 1");      
		return(FALSE);   
	}  
    
    //设置串口输入波特率和输出波特率  
	for ( i= 0;  i < sizeof(speed_arr) / sizeof(int);  i++)  
	{  
		if  (speed == name_arr[i])  
		{               
			cfsetispeed(&options, speed_arr[i]);   
			cfsetospeed(&options, speed_arr[i]);    
		}  
	}       
     
    //修改控制模式，保证程序不会占用串口  
    options.c_cflag |= CLOCAL;  
    //修改控制模式，使得能够从串口中读取输入数据  
    options.c_cflag |= CREAD;  
    
    //设置数据流控制  
    switch(flow_ctrl)  
    {  
        case 0 ://不使用流控制  
              options.c_cflag &= ~CRTSCTS;  
              break;     
        
		case 1 ://使用硬件流控制  
              options.c_cflag |= CRTSCTS;  
              break;  
		case 2 ://使用软件流控制  
              options.c_cflag |= IXON | IXOFF | IXANY;  
              break;  
    }  
    //设置数据位  
    //屏蔽其他标志位  
    options.c_cflag &= ~CSIZE;  
    switch (databits)  
    {    
		case 5:  
                options.c_cflag |= CS5;  
                break;  
		case 6:  
                options.c_cflag |= CS6;  
                     break;  
		case 7:      
                options.c_cflag |= CS7;  
                 break;  
		case 8:      
                options.c_cflag |= CS8;  
                break;    
		default:     
                fprintf(stderr,"Unsupported data size\n");  
                return (FALSE);   
    }  
    //设置校验位  
    switch (parity)  
    {    
		case 'n':  
		case 'N': //无奇偶校验位。  
                 options.c_cflag &= ~PARENB;   
                 options.c_iflag &= ~INPCK;      
                 break;   
		case 'o':    
		case 'O'://设置为奇校验      
                 options.c_cflag |= (PARODD | PARENB);   
                 options.c_iflag |= INPCK;               
                 break;   
		case 'e':   
		case 'E'://设置为偶校验    
                 options.c_cflag |= PARENB;         
                 options.c_cflag &= ~PARODD;         
                 options.c_iflag |= INPCK;        
                 break;  
		case 's':  
		case 'S': //设置为空格   
                 options.c_cflag &= ~PARENB;  
                 options.c_cflag &= ~CSTOPB;  
                 break;   
        default:    
                 fprintf(stderr,"Unsupported parity\n");      
                 return (FALSE);   
    }   
    // 设置停止位   
    switch (stopbits)  
    {    
		case 1:     
                 options.c_cflag &= ~CSTOPB; break;   
		case 2:     
                 options.c_cflag |= CSTOPB; break;  
		default:     
                       fprintf(stderr,"Unsupported stop bits\n");   
                       return (FALSE);  
    }  
     
	//修改输出模式，原始数据输出  
	options.c_oflag &= ~OPOST; 
	//本地标志位：不回送、不启用规范输出、擦除字符不可见、不启用终端产生信号    
	options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
	//需要关掉ICRNL和IXON，不然0xd 0x11 0x13将识别为特殊功能
	options.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON); 	 
     
    //阻塞模式才有意义，设置等待时间和最小接收字符  
    options.c_cc[VTIME] = 1; /*read函数等待100ms时间返回 单位0.1s */    
    options.c_cc[VMIN] = 32; /* 读取VMIN个字符后开始计时*/     
	//清除输入队列缓存
    tcflush(fd,TCIFLUSH);
    //激活配置 (将修改后的termios数据设置到串口中）  
    if (tcsetattr(fd,TCSANOW,&options) != 0)    
	{  
		perror("com set error!\n");    
		return (FALSE);   
	}
	if(uart_DEBUG == g_DebugType_EU) 
	{
		printf("uart0 set success!\n");
	}
    return (TRUE);   
}  
   
/******************************************************************* 
* 名称：            UART_Recv 
* 功能：            接收串口数据 
* 入口参数：        fd:文件描述符     
*                   rcv_buf:接收串口中数据存入rcv_buf缓冲区中 
*                   data_len:一帧数据的长度 
* 出口参数：        正确返回为1，错误返回为0 
*******************************************************************/  
int UART_Recv(int fd, uint8_t *recv_buf,int date_len)  
{  
	int len = 0,i;
	static uint32_t recv_num=0,recv_err_num=0;
	char loginfo[LOG_INFO_LENG] = {0};	
    len = read(fd,recv_buf,date_len);  
    if(len == date_len)
    {			
		s_UartRecvErrNum =0;
		s_UartRecvErrFlag = 0;
        g_VoiceFinger_ErrInfo.Bit3 =0;//接收数据失败，故障位清0
		if(uart_DEBUG  == g_DebugType_EU)
		{
			printf("UART_Recv length is %d\n",len);
			for(i=0;i<len;i++)
			{
				printf("[%02x]",recv_buf[i]);
				if(i+1 == 32) 
				{
					printf("\n");
				}
			}
			recv_num++;
			printf("RecvNum:%d\n",recv_num);	
		} 			
        return len;
    }
    else
    {  	
			if(uart_DEBUG  == g_DebugType_EU)
			{
				printf("Errframe:UART_Recv length is %d\n",len);
				recv_err_num++;
				printf("recv_err_num:%d\n",recv_err_num);
			}
			s_UartRecvErrNum++;
			if(s_UartRecvErrNum>3)
			{
				s_UartRecvErrFlag++;
				s_UartRecvErrNum =0;
				printf("Uart Recv failed! Try %d times!\n",s_UartRecvErrFlag);				  				 
			}
			if(s_UartRecvErrFlag >= 10)
			{
				printf("UART_Send have alread tried 10 times !\n");
				printf("But UART_Recv is still ERROR !\n");		
				snprintf(loginfo, sizeof(loginfo)-1, "UART_Send have alread tried 10 times , but UART_Recv is still ERROR !");
        		WRITELOGFILE(LOG_ERROR_1,loginfo);
				s_UartRecvErrNum = 0;
				s_UartRecvErrFlag = 0;
                g_VoiceFinger_ErrInfo.Bit3 =1;//接收数据失败，故障位置1
				return FALSE;
			}	
	}
}
 
/******************************************************************** 
* Name：            UART0_Send
* 功能：            发送数据 
* 入口参数：        fd:文件描述符     
*                  send_buf:存放串口发送数据 
*                  data_len:一帧数据的个数 
* 出口参数：正确返回为1，错误返回为0 
*******************************************************************/  
int UART_Send(int fd, uint8_t *send_buf,int date_len)  
{  
    int len = 0,i;
	char loginfo[LOG_INFO_LENG] = {0};
	static uint8_t SendErr = 0;
	static uint32_t SendNum = 0;

    len = write(fd,send_buf,date_len);  
    if (len == date_len )  
	{  
		SendErr=0;
        g_VoiceFinger_ErrInfo.Bit2 =0;//发送失败故障位清零
		if(uart_DEBUG  == g_DebugType_EU)
		{
			printf("UART_Send date:\n");
			for(i=0;i<len;i++)
			{
				printf("[%x]",send_buf[i]);
				if(i+1 == 32) 
				{
					printf("\n");
				}
			}
			SendNum++;
			printf("SendNum:%d\n",SendNum);				
		}				
		return len;  
	}       
    else     
	{    
		SendErr++;	
		tcflush(fd,TCOFLUSH);//清理输出队列缓存	
		if(SendErr >= 3)
		{
			printf("UART_Send to Buffer failed !\n");			
			snprintf(loginfo, sizeof(loginfo)-1, "UART_Send to Buffer failed !");
        	WRITELOGFILE(LOG_ERROR_1,loginfo);
			SendErr=0;
        	g_VoiceFinger_ErrInfo.Bit2 =1;//发送失败故障位置1
			return FALSE;	
		}
	}     
}  

/******************************************************************** 
* 名称：            RecvDate_Trdp2Uart_pro 
* 功能：            处理TRDP接收的数据，通过Uart发送 
* 入口参数：        UART_VOICE_TPYE *UartRecvDate 从TRDP获取的数据 
*                  uint8_t *outbuf 需要通过UART发送的数据
* 出口参数：void
*******************************************************************/
void RecvDate_Trdp2Uart_pro(UART_VOICE_TPYE *UartRecvDate,uint8_t *outbuf)
{
	int i;
	BYTE_BIT VOICE_CMD[2] ={0}; 
	memset(outbuf, 0, 32);//对数组清零
	//由于语音模块复制主机发送的生命信号，如果等于0，语音模块返回第一帧会误判生命信号暂停
	static uint8_t uart_life =1; 
  
	if(uart_TO_TRDP_DEBUG == g_DebugType_EU)
	{
		printf("Uart Recv from TRDP:\n");
		printf("UartRecvDate ->WakeUpTime:%x\n",UartRecvDate ->WakeUpTime);
		for(i= 0;i<2;i++) 
		{
			printf("UartRecvDate ->UartDate_U8[%d]:%x\n",i,UartRecvDate ->UartDate_U8[i]);
		}				
	} 
	VOICE_CMD[0] = UartRecvDate ->UartDate_U8[0];
	VOICE_CMD[1] = UartRecvDate ->UartDate_U8[1]; 


	outbuf[0] = 0x7E;//开始符
	outbuf[1] = 0x01;//设备地址，0x01表示主机
	outbuf[2] = 0x0A;//控制字，0x0A表示主机发送
	outbuf[3] = 0x1A;//数据总长度（byte）,包含生命信号
	outbuf[4] = 0x88;//uart_life;//生命信号//Attention:生命信号已被强制为常量，后续记得修改
	outbuf[5] = UartRecvDate ->WakeUpTime;//唤醒时间设置
	outbuf[8] = UartRecvDate ->volumeGain;//音量增益设置	
	//语音唤醒使能
	if(VOICE_CMD[1].Bit4)
	{
		outbuf[6] = 0X11;
	}	
	//“介绍自己”使能
	if(VOICE_CMD[1].Bit5)
	{
		outbuf[7] = 0X11;
	}
	//指纹识别成功
	if(VOICE_CMD[1].Bit6)
	{
		outbuf[10] = 0X11;
	}
	//操纵台已打开
	if(VOICE_CMD[0].Bit0)
	{
		outbuf[11] = 0X11;
	}	
	//操纵台打开异常
	if(VOICE_CMD[0].Bit1)
	{
		outbuf[12] = 0X11;
	}
	//操纵台已关闭
	if(VOICE_CMD[0].Bit2)
	{
		outbuf[13] = 0X11;
	}
	//操纵台关闭异常
	if(VOICE_CMD[0].Bit3)
	{
		outbuf[14] = 0X11;
	}
	//左门处于打开状态
	if(VOICE_CMD[1].Bit2)
	{
		outbuf[15] = 0X11;
	}	
	//右门处于打开状态
	if(VOICE_CMD[1].Bit0)
	{
		outbuf[16] = 0X11;
	}
	//左门处于关闭状态
	if(VOICE_CMD[1].Bit1)
	{
		outbuf[17] = 0X11;
	}
	//右门处于关闭状态
	if(VOICE_CMD[1].Bit3)
	{
		outbuf[18] = 0X11;
	}
	//遮阳帘处于打开状态
	if(VOICE_CMD[0].Bit4)
	{
		outbuf[19] = 0X11;
	}
	//遮阳帘处于关闭状态
	if(VOICE_CMD[0].Bit5)
	{
		outbuf[20] = 0X11;
	}
	//校验和
	outbuf[30] = calc_checksum(outbuf, 30);
	//校验异或
	outbuf[31] = calc_Xor(outbuf, 30);
	//uart_life++;//Attention:生命信号已被强制为常量，后续记得修改
}



/******************************************************************** 
* 名称：            SendDate_Uart2Trdp_pro 
* 功能：            处理Uart接收的数据，通过TRDP发送 
* 入口参数：        fd:文件描述符     
*                  inbuf:通过Uart获取的数据 
*                  UART_VOICE_TPYE *UartSendDate 要通过TRDP发送的数据
* 出口参数：	   	数据正常返回1 ，数据有误返回0
*******************************************************************/
uint8_t SendDate_Uart2Trdp_pro(UART_VOICE_TPYE *UartSendDate,uint8_t *inbuf)
{
	int i;
	char loginfo[LOG_INFO_LENG] = {0};	
	BYTE_BIT VOICE_CMD[5] ={0};	
	static uint8_t VoiceHeadErrNum = 0;
	static uint8_t VoiceLifeErrNum = 0;
	static uint8_t VoiceCheckErrNum = 0;  	
	uint8_t Tmp_CheckSum =0,Tmp_Xor =0;
  	//static uint8_t s_voice_life =0;
	
  	if(inbuf[0] == 0xE7 && inbuf[1] == 0x10 && inbuf[2] == 0xA0 && inbuf[3] == 0x1A)
	{			
		VoiceHeadErrNum =0;
		//TODO:生命信号判断已经屏蔽，后续记得修改
		/*if(inbuf[4] == s_voice_life)
		{
			VoiceLifeErrNum++;
			if(uart_DEBUG  == g_DebugType_EU)
			{
				printf("Voice Device's life stopped %d times!\n",VoiceLifeErrNum);					
			}		
			if(VoiceLifeErrNum >= 10)
			{	
				printf("Voice Device's life stopped !\n");
				snprintf(loginfo, sizeof(loginfo)-1, "Voice Device's life stopped !");
                WRITELOGFILE(LOG_ERROR_1,loginfo);
				VoiceLifeErrNum = 0;
				g_VoiceFinger_ErrInfo.Bit4 =1;//接收数据解析错误
				return FALSE;				
			}
		}
		else if(VoiceLifeErrNum >0)
		{
			VoiceLifeErrNum = 0;
		}*/
		//通过语音唤醒成功
		if(inbuf[5] == 0x11)
		{
			VOICE_CMD[0].Bit0 = 1;
		}
		//通过指纹唤醒成功
		if(inbuf[6] == 0x11)
		{
			VOICE_CMD[0].Bit1 = 1;
		}
		//打开操纵台
		if(inbuf[10] == 0x11)
		{
			VOICE_CMD[3].Bit0 = 1;
		}
		//关闭操纵台
		if(inbuf[11] == 0x11)
		{
			VOICE_CMD[3].Bit1 = 1;
		}
		//打开客室照明
		if(inbuf[12] == 0x11)
		{
			VOICE_CMD[4].Bit4 = 1;
		}
		//关闭客室照明
		if(inbuf[13] == 0x11)
		{
			VOICE_CMD[4].Bit5 = 1;
		}
		//打开氛围灯
		if(inbuf[14] == 0x11)
		{
			VOICE_CMD[4].Bit6 = 1;
		}
		//关闭氛围灯
		if(inbuf[15] == 0x11)
		{
			VOICE_CMD[4].Bit7 = 1;
		}
		//开左门
		if(inbuf[16] == 0x11)
		{
			VOICE_CMD[4].Bit2 = 1;
		}
		//开右门
		if(inbuf[17] == 0x11)
		{
			VOICE_CMD[4].Bit0 = 1;
		}
		//关左门
		if(inbuf[18] == 0x11)
		{
			VOICE_CMD[4].Bit1 = 1;
		}
		//关右门
		if(inbuf[19] == 0x11)
		{
			VOICE_CMD[4].Bit3 = 1;
		}
		//打开遮阳帘
		if(inbuf[20] == 0x11)
		{
			VOICE_CMD[3].Bit2 = 1;
		}
		//关闭遮阳帘
		if(inbuf[21] == 0x11)
		{
			VOICE_CMD[3].Bit3 = 1;
		}
		//打开前照灯		
		if(inbuf[22] == 0x11)
		{
			VOICE_CMD[3].Bit4 = 1;
		}
		//关闭前照灯
		if(inbuf[23] == 0x11)
		{
			VOICE_CMD[3].Bit5 = 1;
		}
		//介绍自己
		if(inbuf[24] == 0x11)
		{
			VOICE_CMD[0].Bit2 = 1;
		}
		//校验和
		Tmp_CheckSum = calc_checksum(inbuf, 30);
		//校验异或
		Tmp_Xor = calc_Xor(inbuf, 30);
		if(uart_DEBUG  == g_DebugType_EU)
		{
			printf("Recv_Date CheckSum:%02X CheckXOR:%02X \n",inbuf[30],inbuf[31]);
			printf("Correct	  CheckSum:%02X CheckXOR:%02X \n",Tmp_CheckSum,Tmp_Xor);
		}
		//校验通过//Attention:校验值判断已经屏蔽，请注意修改
		/*
		if(Tmp_CheckSum==inbuf[30] && Tmp_Xor == inbuf[31])
		{*/					
			VoiceCheckErrNum =0;
			g_VoiceFinger_ErrInfo.Bit4 =0;//接收数据解析正确，故障位清0
			UartSendDate ->Life_voice_U8 = inbuf[4];
			memcpy(UartSendDate->UartDate_U8,VOICE_CMD,5);
			if(uart_TO_TRDP_DEBUG  == g_DebugType_EU)
			{
				printf("Uart Send to TRDP:\n");
				printf("UartSendDate ->Life_voice_U8:%x\n",UartSendDate ->Life_voice_U8);
				for(i= 0;i<5;i++) 
				{
					printf("UartSendDate->UartDate_U8[%d]:%x\n",i,UartSendDate->UartDate_U8[i]);
				}				
			}
		/*}
		else
		{
			VoiceCheckErrNum++;
			if (VoiceCheckErrNum >= 10)
			{	
				printf("Recv_Date CheckSum or CheckXOR Wrong!\n");
				snprintf(loginfo, sizeof(loginfo)-1, "Recv_Date CheckSum or CheckXOR Wrong!");
        		WRITELOGFILE(LOG_ERROR_1,loginfo);
				VoiceCheckErrNum =0;
				g_VoiceFinger_ErrInfo.Bit4 =1;//接收数据解析错误
				return FALSE;
			}		
		}*/		
		//s_voice_life = inbuf[4];
	}
	else
	{	
		VoiceHeadErrNum++;
		if(VoiceHeadErrNum >= 10)
		{
			printf("Recv_Date Frame Header contain mistakes!\n");
			snprintf(loginfo, sizeof(loginfo)-1, "Recv_Date Frame Header contain mistakes ! Recv_Date[0]:%x ,Recv_Date[1]:%x ,Recv_Date[2]:%x ,Recv_Date[3]:%x",\
					inbuf[0],inbuf[1],inbuf[2],inbuf[3]);
        	WRITELOGFILE(LOG_ERROR_1,loginfo);
			VoiceHeadErrNum =0;
			g_VoiceFinger_ErrInfo.Bit4 =1;//接收数据解析错误
			return FALSE;
		}		
	}
	return TRUE;
}
