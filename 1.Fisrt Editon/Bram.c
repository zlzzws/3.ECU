/**********************************************************************
*File name 		:   Bram.c
*Copyright      :   Zhuzhou CRRC Times Electric Co. Ltd ALL Rights Reserved
*Creat Date     :   2018/1/29
*Description    :   Include the bram data read and write.
*Version        :   REV1.0.0       
*Author:        :   feng
*
*History:
*REV1.0.0     feng    2018/1/29  Create
*
*
*
*********************************************************************/

/**********************************************************************
*
*Include file  section
*
*********************************************************************/
#include "Bram.h"



/**********************************************************************
*
*Global Variable Declare Section
*
*********************************************************************/

extern DEBUG_TYPE_ENUM g_DebugType_EU;

/**********************************************************************
*
*Function Define Section
*
*********************************************************************/

/**********************************************************************
*Name			:   BramOpen(void)
*Function       :   
*Para     		:   
*Return    		:   return the fd
*Version        :   REV1.0.0       
*Author:        :   feng
*
*History:
*REV1.0.0     feng    2018/1/29  Create

*********************************************************************/
int8_t  BramOpen(void)
{
	int8_t fd;
	int8_t err = 0;
	//open /dev/mem
	fd = open("/dev/mem", O_RDWR | O_SYNC);
	if(fd  == -1)
	{
		perror("open /dev/mem:");
		//err = CODE_ERR;
	}
    return fd;

}
/**********************************************************************
*Name			:   BramMap
*Function       :   
*Para     		:   int8_t Fd
*					uint32_t phyaddr
*     				uint16_t PageSize PageSize is 4096
*Return    		:   return the fd
*Version        :   REV1.0.0       
*Author:        :   feng
*
*History:
*REV1.0.0     feng    2018/1/29  Create

*********************************************************************/
// PageSize is 4096
uint8_t *BramMap(int8_t Fd,uint32_t phyaddr,uint16_t PageSize)
{
	uint8_t *map_base = 0;
	uint32_t base = phyaddr & PAGE_MASK;
	map_base = mmap(NULL, PageSize, PROT_READ | PROT_WRITE, MAP_SHARED,
			Fd, base);
	if(map_base == MAP_FAILED)
	{
		perror("mmap:");
	}
	return map_base;
}
/**********************************************************************
*Name			:   BramPackReadU32
*Function       :   Read 32bit of Bram  from the phyaddr  address 
*Para     		:   uint32_t phyaddr  the addres of Bram
*               :   uint32_t Mapbase   the mmap address  
*Return    		:   Read value
*Version        :   REV1.0.0       
*Author:        :   feng
*
*History:
*REV1.0.0     feng    2018/1/29  Create
*MAX 4096
*********************************************************************/
int8_t BramPackReadU32(uint32_t phyaddr,uint8_t *Mapbase,uint32_t *inbuf, uint16_t length)
{
	uint16_t i;
	uint32_t val;
	uint32_t pgoffset = phyaddr & (~PAGE_MASK);//(~PAGE_MASK) == ~ (FFFF F000) == 0000 0FFF
    uint32_t *FromAddr;
    FromAddr = (uint32_t *)(Mapbase + pgoffset);
	memcpy(inbuf,FromAddr,length * 4); //length代表字节数，对于uint32_t来说，实际cpy数据需要*4
	return CODE_OK;

}
/**********************************************************************
*Name			:   BramPackWriteU32
*Function       :   Write 32bit of Bram  from the phyaddr  address 
*Para     		:   uint32_t phyaddr  the addres of Bram
*               :   uint32_t Mapbase   the mmap address  
*Return    		:   Read value
*Version        :   REV1.0.0       
*Author:        :   feng
*
*History:
*REV1.0.0     feng    2018/1/29  Create
*********************************************************************/
int8_t BramPackWriteU32(uint32_t phyaddr,uint8_t *Mapbase,uint32_t *outbuf, uint16_t length)
{

	uint32_t pgoffset = phyaddr & (~PAGE_MASK);
	uint32_t *ToAddr;
    ToAddr = (uint32_t *)(Mapbase + pgoffset);
	memcpy(ToAddr,outbuf,length * 4); //2~3us
	return CODE_OK;
}
/**********************************************************************
*Name			:   uint8_t BramReadU8
*Function       :   Write 8bit of Bram  from the phyaddr  address 
*Para     		:   uint32_t phyaddr  the addres of Bram
*               :   uint32_t Mapbase   the mmap address  
*Return    		:   Read value
*Version        :   REV1.0.0       
*Author:        :   feng
*
*History:
*REV1.0.0     feng    2018/1/29  Create
*********************************************************************/
uint8_t BramReadU8(uint32_t phyaddr,uint8_t *Mapbase)
{
	uint16_t i,j;
	uint8_t val = 0;
	uint32_t pgoffset = phyaddr & (~PAGE_MASK);
	val = *(volatile uint8_t *)(Mapbase + pgoffset);
	return val;
}
/**********************************************************************
*Name			:   BramWriteU8
*Function       :   Write 8bit of Bram  from the phyaddr  address 
*Para     		:   uint32_t phyaddr  the addres of Bram
*               :   uint32_t Mapbase   the mmap address
*				:	uint8_t val    
*Return    		:   void.
*Version        :   REV1.0.0       
*Author:        :   feng
*
*History:
*REV1.0.0     feng    2018/1/29  Create
*********************************************************************/
int8_t BramWriteU8(uint32_t phyaddr,uint8_t *Mapbase,uint8_t val)
{
	uint16_t i,j;
	uint32_t pgoffset = phyaddr & (~PAGE_MASK);

	*(volatile uint8_t *)(Mapbase + pgoffset) = val;
    return CODE_OK;
}
/**********************************************************************
*Name			:   CloseBram
*Function       :   Write 8bit of Bram  from the phyaddr  address 
*Para     		:   uint8_t *mapAdd  the addres of Bram
*               :   uint16_t size  
*Return    		:   
*Version        :   REV1.0.0       
*Author:        :   feng
*
*History:
*REV1.0.0     feng    2018/1/29  Create
*********************************************************************/
int8_t CloseBram(uint8_t *mapAdd,uint16_t size)
{
	
	munmap((void *)mapAdd, size);
	return CODE_OK;
}