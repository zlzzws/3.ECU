/**********************************************************************
*File name      :   GPIOControl.c
*Copyright      :   Zhuzhou CRRC Times Electric Co. Ltd ALL Rights Reserved
*Creat Date     :   2018/8/1
*Description    :   Include file read and write the GPIO of PS
*Version        :   REV1.0.0       
*Author:        :   feng
*
*History:
*REV1.0.0     feng    2018/8/1 Create
*********************************************************************/

/***********************************************************************
*Debug switch  section*
*********************************************************************/


/***********************************************************************
*Include file  section*
*********************************************************************/
#include "GPIOControl.h" 
 
/***********************************************************************
*Global Variable Declare Section*
*********************************************************************/
extern DEBUG_TYPE_ENUM g_DebugType_EU;

/***********************************************************************
*Local Macro Define Section*
*********************************************************************/

/***********************************************************************
*Local Struct Define Section*
*********************************************************************/


/***********************************************************************
*Local Prototype Declare Section*
*********************************************************************/

/***********************************************************************
*Static Variable Define Section*
*********************************************************************/
static int8_t  s_gpio_PowDownfd;
static int8_t  s_gpio_Rs485fd;

/***********************************************************************
*Function Define Section*
*********************************************************************/


/*  The specific GPIO being used must be setup and replaced thru this code.  
    The GPIO of 916 is in the path of most the sys dirs and in the export write.
    Figuring out the exact GPIO was not totally obvious when there were multiple GPIOs in the system.
    One way to do is to go into the gpiochips in /sys/class/gpio and view the label as it should
    reflect the address of the GPIO in the system. The name of the the chip appears to be the 1st GPIO of the controller.
    The export causes the gpio916 dir to appear in /sys/class/gpio. Then the direction and value can be changed by writing to them. 
    The performance of this is pretty good, using a nfs mount,running on open source linux, on the ML507 reference system,the GPIO can be toggled about every 4 usec.
    The following commands from the console setup the GPIO to be exported, set the direction of it to an output and write a 1 to the GPIO.
        bash> echo 916 > /sys/class/gpio/export
        bash> echo out > /sys/class/gpio/gpio916/direction
        bash> echo 1 > /sys/class/gpio/gpio916/value 
    if sysfs is not mounted on your system, the you need to mount it
        bash> mount -t sysfs sysfs /sys 
    the following bash script to toggle the gpio is also handy for testing.
        while [ 1 ]; do
        echo 1 > /sys/class/gpio/gpio916/value
        echo 0 > /sys/class/gpio/gpio916/value
    done
*/
/**********************************************************************
*Name           :   int8_t GPIO_PowDownIoCreat(void)          
*Function       :   Create the GPIO_POWDOW  Io sysfs               
*Para           :   
*Return         :   int8_t 0,success;-1 false.
*Version        :   REV1.0.0       
*Author:        :   feng
*History:
*REV1.0.0     feng    2018/8/1  Create
*********************************************************************/
int8_t GPIO_PowDownIoCreat(void)
{
    int16_t exportfd, directionfd;
    int8_t WrRdErr = 0;
    char loginfo[LOG_INFO_LENG] = {0};
 
    // The GPIO has to be exported to be able to see it
    // in sysfs
    exportfd = open("/sys/class/gpio/export", O_WRONLY);
    if (exportfd < 0)
    {
        printf("Cannot open GPIO  export\n");
        snprintf(loginfo, sizeof(loginfo)-1, "Cannot open GPIO export!");
        WRITELOGFILE(LOG_ERROR_1,loginfo);
        return CODE_ERR;
    }
    /*if /sys/class/gpio/export have been operated before,then write exportfd option will show error*/
    WrRdErr = write(exportfd,GPIO_POWDOW,4); //4 mean the write length
    if(WrRdErr == -1)
    {
        printf("write GPIO_POWDOW exportfd error!\n");
        snprintf(loginfo, sizeof(loginfo)-1, "write GPIO_POWDOW exportfd error!");
        WRITELOGFILE(LOG_ERROR_1,loginfo);
        return CODE_ERR;
    }
    close(exportfd);
    //set the direction of the GPIO957 to input
    directionfd = open(GPIO_POWDOW_DIREC, O_RDWR);
    if (directionfd < 0)
    {
        printf("Cannot open GPIO_POWDOW direction!\n");
        snprintf(loginfo, sizeof(loginfo)-1, "Cannot open GPIO_POWDOW direction!");
        WRITELOGFILE(LOG_ERROR_1,loginfo);
        return CODE_ERR;
    }
    write(directionfd, "in", 3);
    if(WrRdErr == -1)
    {
        printf("write GPIO_POWDOW directionfd error!\n");
        snprintf(loginfo, sizeof(loginfo)-1, "write GPIO_POWDOW directionfd error!");
        WRITELOGFILE(LOG_ERROR_1,loginfo);
        return CODE_ERR;
    }
    close(directionfd);
}

/**********************************************************************
*Name           :   int8_t GPIO_PowDowOpen(uint8_t value)            
*Function       :   open the gpio power down IO value                
*Para           :   
*Return         :   int8_t 0,success;-1 false.
*Version        :   REV1.0.0       
*Author:        :   feng
*History:
*REV1.0.0     feng    2018/8/1  Create
*********************************************************************/
int8_t GPIO_PowDowOpen(void)
{

    char loginfo[LOG_INFO_LENG] = {0};
    int8_t err = 0;
    s_gpio_PowDownfd = open(GPIO_POWDOW_VALUE, O_RDWR);
    if (s_gpio_PowDownfd < 0)
    {
        printf("Cannot open POWDOW /gpio948/value/ \n");
        snprintf(loginfo, sizeof(loginfo)-1, "Cannot open POWDOW /gpio948/value/");
        WRITELOGFILE(LOG_ERROR_1,loginfo);     
        err =  CODE_ERR;
    }
    return CODE_OK;
}

/**********************************************************************
*Name           :   int8_t GPIO_PowDowRead           
*Function       :   Read the Pow Down IO              
*Para           :   uint16_t Length
*Return         :   int8_t 0,success;-1 false.
*Version        :   REV1.0.0       
*Author:        :   feng
*History:
*REV1.0.0     feng    2018/8/1  Create
*********************************************************************/
int8_t GPIO_PowDowRead(uint8_t *value)
{    
    int8_t WrRdErr = 0;
    char GPIOValueChar[2] = {0}; 
    char  loginfo[LOG_INFO_LENG] =  {0};

    if(s_gpio_PowDownfd < 0)
    {
        printf("POWDOW /gpio948/value/ fd error\n");
        snprintf(loginfo, sizeof(loginfo)-1, "POWDOW /gpio948/value/ fd error");
        WRITELOGFILE(LOG_ERROR_1,loginfo);     
        return CODE_ERR;
    }
    lseek(s_gpio_PowDownfd,0,SEEK_SET);//set the file start site
    WrRdErr = read(s_gpio_PowDownfd,GPIOValueChar,2);
    if(WrRdErr == -1)
    {
        printf("gpio_PowDown value Read error\n");
        snprintf(loginfo, sizeof(loginfo)-1, "gpio_PowDown value Read error");
        WRITELOGFILE(LOG_ERROR_1,loginfo);     
        return CODE_ERR;
    }
    if(g_DebugType_EU == GPIO_DEBUG)
    {
        printf("gpio_PowDown GPIOValue %s\n",GPIOValueChar);
    }
    *value = (GPIOValueChar[0] - 48);
    return CODE_OK;

}

/**********************************************************************
*Name           :   int8_t GPIO_PowDowWrite(uint8_t value)            
*Function       :   写PowDow引脚              
*Para           :   
*Return         :   int8_t 0,success;-1 false.
*Version        :   REV1.0.0       
*Author:        :   feng
*History:
*REV1.0.0     feng    2018/8/1  Create
*********************************************************************/
int8_t GPIO_PowDowWrite(uint8_t value)
{
    
    int8_t WrRdErr = 0;
    char GPIOValueChar[2] = {0};
    sprintf(GPIOValueChar,"%d",value); 
    lseek(s_gpio_PowDownfd,0,SEEK_SET);//set the file start site
    WrRdErr = write(s_gpio_PowDownfd,GPIOValueChar,2);
    if(WrRdErr == -1)
    {
        printf("Write error\n");
        return CODE_ERR;
    }
    if(g_DebugType_EU == GPIO_DEBUG)
    {
        printf("s_gpio_PowDownfd GPIOValue %s\n",GPIOValueChar);
    }
    return CODE_OK;

}
/**********************************************************************
*Name           :   GPIO_SysFlWrLow            
*Function       :   
*Return         :   int8_t 0,success;-1 false.
*Version        :   REV1.0.0       
*Author:        :   feng
*History:
*REV1.0.0     feng    2018/8/1  Create
*********************************************************************/

int8_t GPIO_SysFlWrLow(void)
{
    int16_t exportfd, directionfd;
    int8_t WrRdErr = 0;
 
    // The GPIO has to be exported to be able to see it
    // in sysfs
    exportfd = open("/sys/class/gpio/export", O_WRONLY);
    if (exportfd < 0)
    {
        printf("Cannot open GPIO_POWDOW to export it\n");
        //return CODE_ERR;
    }//if have operate once  /sys/class/gpio/export,then write exportfd will write error
    WrRdErr = write(exportfd, GPIO_POWDOW, 4); //4 mean the write length
    if(WrRdErr == -1)
    {
        printf("write e GPIO_POWDOW xportfd error\n");
        //return CODE_ERR;
    }
    close(exportfd);
    if(g_DebugType_EU == GPIO_DEBUG)
    {
        printf("GPIO_POWDOW exported successfully\n");
    }

    // Update the direction of the GPIO to be an output
    directionfd = open(GPIO_POWDOW_DIREC, O_RDWR);
    if (directionfd < 0)
    {
        printf("Cannot open GPIO_POWDOW direction it\n");
        //return CODE_ERR;
    }
    /*set the gpio is low*/
    write(directionfd, "out", 4);
    if(WrRdErr == -1)
    {
        printf("write  GPIO_POWDOW directionfd error\n");
        //return CODE_ERR;
    }
    close(directionfd);
    s_gpio_PowDownfd = open(GPIO_POWDOW_VALUE, O_RDWR);
    if (s_gpio_PowDownfd  < 0)
    {
        printf("Cannot open GPIO POWDOW value\n");
        //return CODE_ERR;
    }
    write(s_gpio_PowDownfd,"0", 2);
    close(s_gpio_PowDownfd);

    if(g_DebugType_EU == GPIO_DEBUG)
    {
        printf("GPIO_POWDOW direction set as output successfully\n");       
    }

}

#if 0

/**********************************************************************
*Name           :   int8_t GPIO_RS485Open(void)        
*Function       :   Open the Io sysfs               
*Para           :   
*Return         :   int8_t 0,success;-1 false.
*Version        :   REV1.0.0       
*Author:        :   feng
*History:
*REV1.0.0     feng    2018/8/1  Create
*********************************************************************/
int8_t GPIO_RS485Open(void)
{
    int16_t exportfd, directionfd;
    int8_t WrRdErr = 0;
 
    // The GPIO has to be exported to be able to see it in sysfs    
    exportfd = open("/sys/class/gpio/export", O_WRONLY);
    if (exportfd < 0)
    {
        printf("Cannot open GPIO_RS485 to export it\n");
        return -1;
    }
	//if have operate once /sys/class/gpio/export,then write exportfd will write error
	//意味着没释放export并再次export，write操作会失败
    WrRdErr = write(exportfd, GPIO_RS485EN, 4); //4 mean the write length，956=906(Mio_base)+MIO:50
    if(WrRdErr == -1)
    {
        printf("write RS485 exportfd error\n");
        return -1;
    }
    close(exportfd);
    if(g_DebugType_EU == GPIO_DEBUG)
    {
        printf("GPIO exported successfully\n");
    }

    // Update the direction of the GPIO to be an output
    directionfd = open(GPIO_RS485EN_DIREC, O_RDWR);
    if (directionfd < 0)
    {
        printf("Cannot open GPIO direction it\n");
        return -1;
    }
    write(directionfd, "out", 4);
    if(WrRdErr == -1)
    {
        printf("write directionfd error\n");
        return -1;
    }
    close(directionfd);
    if(g_DebugType_EU == GPIO_DEBUG)
    {
        printf("GPIO direction set as output successfully\n");       
    }
}
/**********************************************************************
*Name           :   int8_t GPIO_RS485EnIOCreat(void)          
*Function       :   Create the GPIO_RS485  sysfs               
*Para           :   
*Return         :   int8_t 0,success;-1 false.
*Version        :   REV1.0.0       
*Author:        :   feng
*History:
*REV1.0.0     feng    2018/8/1  Create
*********************************************************************/
int8_t GPIO_RS485EnIOCreat(void)
{
    int16_t exportfd, directionfd;
    int8_t WrRdErr = 0;
 
    // The GPIO has to be exported to be able to see it
    // in sysfs
    exportfd = open("/sys/class/gpio/export", O_WRONLY);
    if (exportfd < 0)
    {
        printf("Cannot open GPIO to export it\n");
        return CODE_ERR;
    }//if have operate once  /sys/class/gpio/export,then write exportfd will write error
    WrRdErr = write(exportfd, GPIO_RS485EN, 4); //4 mean the write length
    if(WrRdErr == -1)
    {
        printf("write GPIO_RS485EN exportfd error\n");
        return CODE_ERR;
    }
    close(exportfd);
    if(g_DebugType_EU == GPIO_DEBUG)
    {
        printf("GPIO_RS485EN exported successfully\n");
    }

    // Update the direction of the GPIO to be an output
    directionfd = open(GPIO_RS485EN_DIREC, O_RDWR);
    if (directionfd < 0)
    {
        printf("Cannot open GPIO_RS485EN direction it\n");
        return CODE_ERR;
    }
    write(directionfd, "out", 4);
    if(WrRdErr == -1)
    {
        printf("write GPIO_RS485EN directionfd error\n");
        return CODE_ERR;
    }
    close(directionfd);
    if(g_DebugType_EU == GPIO_DEBUG)
    {
        printf("GPIO_RS485EN direction set as output successfully\n");       
    }
}
/**********************************************************************
*Name           :   int8_t GPIO_RS485EnOpen(uint8_t value)            
*Function       :   Open the GPIO_RS485EnOpen value               
*Para           :   
*Return         :   int8_t 0,success;-1 false.
*Version        :   REV1.0.0       
*Author:        :   feng
*History:
*REV1.0.0     feng    2018/8/1  Create
*********************************************************************/
int8_t GPIO_RS485EnOpen(void)
{

    s_gpio_Rs485fd = open(GPIO_RS485EN_VALUE, O_RDWR);
    if (s_gpio_Rs485fd < 0)
    {
        printf("Cannot open GPIO RS485EN value\n");
        return CODE_ERR;
    }
    return CODE_OK;

}
/**********************************************************************
*Name           :   int8_t GPIO_RS484EnWrite            
*Function       :   Write GPIO_RS484En IO               
*Para           :   
*Return         :   int8_t 0,success;-1 false.
*Version        :   REV1.0.0       
*Author:        :   feng
*History:
*REV1.0.0     feng    2018/8/1  Create
*********************************************************************/
int8_t GPIO_RS484EnWrite(uint8_t value)
{
    
    int8_t WrRdErr = 0;
    char GPIOValueChar[10] = {0};
    sprintf(GPIOValueChar,"%d",value); 
    lseek(s_gpio_Rs485fd,0,SEEK_SET);//set the file start site
    WrRdErr = write(s_gpio_Rs485fd,GPIOValueChar,2);
    if(WrRdErr == -1)
    {
        printf("Write Rs485EB error\n");
        return CODE_ERR;
    }
    if(g_DebugType_EU == GPIO_DEBUG)
    {
        printf("s_gpio_Rs485fd GPIOValue %s\n",GPIOValueChar);
    }
    return CODE_OK;
}
#endif