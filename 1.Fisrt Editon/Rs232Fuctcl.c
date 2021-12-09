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
        return len;
    }
    else
    {
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

