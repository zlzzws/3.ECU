/**********************************************************************
*File name      :   modbus.c
*Copyright      :   Zhuzhou CRRC Times Electric Co. Ltd ALL Rights Reserved
*Creat Date     :   2018/8/27
*Description    :   Creat a modbus server and test some function
*                   1.test the feedback of High-voltage check automatic
*                   2.test the read of version about system
*                   3. 
*                   4.
*Version        :   REV1.0.0       
*Author:        :   feng
*
*History:
*REV1.0.0     feng    2018/8/27  Create
*REV1.0.1     feng    2018/8/31 
*                    
*
*********************************************************************/

/**********************************************************************
*
*Include file  section
*
*********************************************************************/
#include "ModbusSer.h"

/**********************************************************************
*
*Global Variable Declare Section
*
*********************************************************************/


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
/* 
typedef struct 
{
    int nb_bits;
    int start_bits;
    int nb_input_bits;
    int start_input_bits;
    int nb_input_registers;
    int start_input_registers;
    int nb_registers;
    int start_registers;
    uint8_t *tab_bits;
    uint8_t *tab_input_bits;
    uint16_t *tab_input_registers;
    uint16_t *tab_registers;
} modbus_mapping_t;
*/
/**********************************************************************
*
*Local Prototype Declare Section
*
*********************************************************************/
void LightTest();
void VersionSet();
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
/**********************************************************************
*Name           :   ModBusChanCali
*Function       :   lighten the lamp of Voltage-high Check feedback
*               :   
*Para           :   
*
*Return         :   
*History:
*REV1.0.0         2018/8/27  Create
*
*********************************************************************/
void ModBusChanCali(modbus_mapping_t *mb_mapping)
{
    if(mb_mapping->tab_registers[2])
    {
        mb_mapping->tab_registers[280]=1;
    }else
    {
        mb_mapping->tab_registers[280]=0;
    }

}
/**********************************************************************
*Name           :   void LightTest()  
*Function       :   lighten the lamp of Voltage-high Check feedback
*               :   
*Para           :   
*
*Return         :   
*Version        :   REV1.0.0       
*Author:        :   feng
*
*History:
*REV1.0.0     feng    2018/8/27  Create
*
*********************************************************************/
void LightTest(modbus_mapping_t *mb_mapping)
{
    if(mb_mapping->tab_registers[2])
    {
        mb_mapping->tab_registers[280]=1;
    }else
    {
        mb_mapping->tab_registers[280]=0;
    }
    int i;
    for( i=24;i<29;i++)
    {
      if(mb_mapping->tab_registers[i])
      {
        mb_mapping->tab_registers[257+i]=1;
      }else
      {
        mb_mapping->tab_registers[257+i]=0;
      }
    }
}
/**********************************************************************
*Name           :   void VersionSet() 
*Function       :    set Version parameter
*               :   
*Para           :   
*
*Return         :   uint8_t 1,success;0 false.
*Version        :   REV1.0.0       
*Author:        :   feng
*
*History:
*REV1.0.0     feng    2018/8/31  Create
*
*********************************************************************/
void VersionSet(modbus_mapping_t *mb_mapping,VERSION *Versi_p) 
{
    //regist 1226
    mb_mapping->tab_input_registers[226]=Versi_p -> Total_Ver_U16;/*Total Version*/

    mb_mapping->tab_input_registers[210]=Versi_p -> Linux_Ver_U16;/*OS Version of MCU1*/
    mb_mapping->tab_input_registers[211]=Versi_p -> SySCfgFile_Ver_U16;/*eclr.out Version of MCU1*/
    mb_mapping->tab_input_registers[212]=Versi_p -> SftCfgFile_Ver_U16;/*boot Version of MCU1*/
    mb_mapping->tab_input_registers[213]=Versi_p -> RrdCfgFile_Ver_U16;/*OMAP_NEXT Version of MCU1*/
    mb_mapping->tab_input_registers[214]=Versi_p -> SysPrmFile_Ver_U16;/*System configure file Version of MCU1*/
    mb_mapping->tab_input_registers[215]=Versi_p -> EADS_RunVer_U16;/*Software configure file Version of MCU1*/
    mb_mapping->tab_input_registers[216]=Versi_p -> Boot_Ver_U16;/*Falut log set of MCU1*/

    mb_mapping->tab_input_registers[223]=Versi_p -> FPGA_Ver_U16;/*OS Version of MCU2*/
    mb_mapping->tab_input_registers[218]=Versi_p -> Urmdik_Ver_U16;/*OMAP_NEXT Version of MCU2*/
    mb_mapping->tab_input_registers[217]=Versi_p -> DevicTree_Ver_U16;/*boot Version of MCU2*/
    mb_mapping->tab_input_registers[219]=Versi_p -> TMS570_RTSVer_U16;/*Software configure Version of MCU2*/
    mb_mapping->tab_input_registers[220]=Versi_p -> TMS570_IECVer_U16;/*Software configure file Version of MCU2*/
    mb_mapping->tab_input_registers[221]=Versi_p -> MAX1_Ver_U16;/*MVB configure Version*/
    mb_mapping->tab_input_registers[222]=0 ;/*  MAX2_Ver_U16 MVB_Confm file Version*/

    mb_mapping->tab_input_registers[225]=123;/*CPLD Version*/
    mb_mapping->tab_input_registers[224]=1234;/*FPGA Version*/

    mb_mapping->tab_input_registers[203]=321;/*Logic procedure Version*/
    mb_mapping->tab_input_registers[200]=4321;/*contravariant Version*/
    mb_mapping->tab_input_registers[201]=567;/*bond procedure Version*/
    //regist 8160
    mb_mapping->tab_registers[160]=123;/*System parameter Version*/
    mb_mapping->tab_registers[153]=324;/*Logic parameter Version*/
    mb_mapping->tab_registers[152]=234;/*contravariant parameter Version*/
    mb_mapping->tab_registers[155]=4321;/*bond parameter Version*/
}
/**********************************************************************
*Name           :   int8_t VersionInit(VERSION *Versi_p)
*Function       :   Init the  Version parameter             :   
*Para           :   VERSION *Versi_p
*                   Total_Ver_U16 = 100,mean version is 1.0.0
*Return         :   int8_t 0,success;else false.
*Version        :   REV1.0.0       
*Author:        :   feng
*
*History:
*REV1.0.0     feng    2018/9/3  Create
*
*********************************************************************/
int8_t VersionInit(VERSION *Versi_p)
{
    Versi_p -> Total_Ver_U16  = TOTAL_VERSION; /*Total Version*/
    Versi_p -> Linux_Ver_U16 =  LINUX_VERSION;/*OS Version of MCU1*/
    Versi_p -> SySCfgFile_Ver_U16 = 100;/*eclr.out Version of MCU1*/
    Versi_p -> SftCfgFile_Ver_U16 = 100;/*boot Version of MCU1*/
    Versi_p -> EADS_RunVer_U16 = EADS_VERSION_PTU;/*Software configure file Version of MCU1*/
    Versi_p -> Boot_Ver_U16 = BOOT_VERSION;/*Falut log set of MCU1*/
    Versi_p -> FPGA_Ver_U16 = FPGA_VERSION;/*OS Version of MCU2*/
    Versi_p -> Urmdik_Ver_U16 = RAMDISK_VERSION;/*OMAP_NEXT Version of MCU2*/
    Versi_p -> DevicTree_Ver_U16 = DTB_VERSION;/*boot Version of MCU2*/
    Versi_p -> TMS570_RTSVer_U16 = 100;/*Software configure Version of MCU2*/
    Versi_p -> TMS570_IECVer_U16 = 100;/*Software configure file Version of MCU2*/
    Versi_p -> MAX1_Ver_U16 = 100;/*MVB configure Version*/
}


