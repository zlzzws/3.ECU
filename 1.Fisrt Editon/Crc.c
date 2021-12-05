/**********************************************************************
*File name 		:   Crc.c
*Copyright      :   Zhuzhou CRRC Times Electric Co. Ltd ALL Rights Reserved
*Creat Date     :   2018/1/29
*Description    :   crc32 calculate
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
#include "Crc.h"
/**********************************************************************
*
*Global Table Define Section
*
*********************************************************************/

uint32_t Crc32Table[256] =
{
0X00000000L, 0X04c11db7L, 0X09823b6eL, 0X0d4326d9L, 
0X130476dcL, 0X17c56b6bL, 0X1a864db2L, 0X1e475005L, 
0X2608edb8L, 0X22c9f00fL, 0X2f8ad6d6L, 0X2b4bcb61L, 
0X350c9b64L, 0X31cd86d3L, 0X3c8ea00aL, 0X384fbdbdL, 
0X4c11db70L, 0X48d0c6c7L, 0X4593e01eL, 0X4152fda9L, 
0X5f15adacL, 0X5bd4b01bL, 0X569796c2L, 0X52568b75L,
0X6a1936c8L, 0X6ed82b7fL, 0X639b0da6L, 0X675a1011L, 
0X791d4014L, 0X7ddc5da3L, 0X709f7b7aL, 0X745e66cdL, 
0X9823b6e0L, 0X9ce2ab57L, 0X91a18d8eL, 0X95609039L, 
0X8b27c03cL, 0X8fe6dd8bL, 0X82a5fb52L, 0X8664e6e5L, 
0Xbe2b5b58L, 0Xbaea46efL, 0Xb7a96036L, 0Xb3687d81L, 
0Xad2f2d84L, 0Xa9ee3033L, 0Xa4ad16eaL, 0Xa06c0b5dL,
0Xd4326d90L, 0Xd0f37027L, 0Xddb056feL, 0Xd9714b49L, 
0Xc7361b4cL, 0Xc3f706fbL, 0Xceb42022L, 0Xca753d95L, 
0Xf23a8028L, 0Xf6fb9d9fL, 0Xfbb8bb46L, 0Xff79a6f1L, 
0Xe13ef6f4L, 0Xe5ffeb43L, 0Xe8bccd9aL, 0Xec7dd02dL, 
0X34867077L, 0X30476dc0L, 0X3d044b19L, 0X39c556aeL, 
0X278206abL, 0X23431b1cL, 0X2e003dc5L, 0X2ac12072L, 
0X128e9dcfL, 0X164f8078L, 0X1b0ca6a1L, 0X1fcdbb16L, 
0X018aeb13L, 0X054bf6a4L, 0X0808d07dL, 0X0cc9cdcaL, 
0X7897ab07L, 0X7c56b6b0L, 0X71159069L, 0X75d48ddeL, 
0X6b93dddbL, 0X6f52c06cL, 0X6211e6b5L, 0X66d0fb02L, 
0X5e9f46bfL, 0X5a5e5b08L, 0X571d7dd1L, 0X53dc6066L,
0X4d9b3063L, 0X495a2dd4L, 0X44190b0dL, 0X40d816baL, 
0Xaca5c697L, 0Xa864db20L, 0Xa527fdf9L, 0Xa1e6e04eL, 
0Xbfa1b04bL, 0Xbb60adfcL, 0Xb6238b25L, 0Xb2e29692L, 
0X8aad2b2fL, 0X8e6c3698L, 0X832f1041L, 0X87ee0df6L, 
0X99a95df3L, 0X9d684044L, 0X902b669dL, 0X94ea7b2aL, 
0Xe0b41de7L, 0Xe4750050L, 0Xe9362689L, 0Xedf73b3eL,
0Xf3b06b3bL, 0Xf771768cL, 0Xfa325055L, 0Xfef34de2L, 
0Xc6bcf05fL, 0Xc27dede8L, 0Xcf3ecb31L, 0Xcbffd686L, 
0Xd5b88683L, 0Xd1799b34L, 0Xdc3abdedL, 0Xd8fba05aL, 
0X690ce0eeL, 0X6dcdfd59L, 0X608edb80L, 0X644fc637L, 
0X7a089632L, 0X7ec98b85L, 0X738aad5cL, 0X774bb0ebL, 
0X4f040d56L, 0X4bc510e1L, 0X46863638L, 0X42472b8fL, 
0X5c007b8aL, 0X58c1663dL, 0X558240e4L, 0X51435d53L, 
0X251d3b9eL, 0X21dc2629L, 0X2c9f00f0L, 0X285e1d47L, 
0X36194d42L, 0X32d850f5L, 0X3f9b762cL, 0X3b5a6b9bL, 
0X0315d626L, 0X07d4cb91L, 0X0a97ed48L, 0X0e56f0ffL, 
0X1011a0faL, 0X14d0bd4dL, 0X19939b94L, 0X1d528623L,
0Xf12f560eL, 0Xf5ee4bb9L, 0Xf8ad6d60L, 0Xfc6c70d7L, 
0Xe22b20d2L, 0Xe6ea3d65L, 0Xeba91bbcL, 0Xef68060bL, 
0Xd727bbb6L, 0Xd3e6a601L, 0Xdea580d8L, 0Xda649d6fL, 
0Xc423cd6aL, 0Xc0e2d0ddL, 0Xcda1f604L, 0Xc960ebb3L, 
0Xbd3e8d7eL, 0Xb9ff90c9L, 0Xb4bcb610L, 0Xb07daba7L, 
0Xae3afba2L, 0Xaafbe615L, 0Xa7b8c0ccL, 0Xa379dd7bL,
0X9b3660c6L, 0X9ff77d71L, 0X92b45ba8L, 0X9675461fL, 
0X8832161aL, 0X8cf30badL, 0X81b02d74L, 0X857130c3L, 
0X5d8a9099L, 0X594b8d2eL, 0X5408abf7L, 0X50c9b640L, 
0X4e8ee645L, 0X4a4ffbf2L, 0X470cdd2bL, 0X43cdc09cL, 
0X7b827d21L, 0X7f436096L, 0X7200464fL, 0X76c15bf8L, 
0X68860bfdL, 0X6c47164aL, 0X61043093L, 0X65c52d24L, 
0X119b4be9L, 0X155a565eL, 0X18197087L, 0X1cd86d30L, 
0X029f3d35L, 0X065e2082L, 0X0b1d065bL, 0X0fdc1becL, 
0X3793a651L, 0X3352bbe6L, 0X3e119d3fL, 0X3ad08088L, 
0X2497d08dL, 0X2056cd3aL, 0X2d15ebe3L, 0X29d4f654L, 
0Xc5a92679L, 0Xc1683bceL, 0Xcc2b1d17L, 0Xc8ea00a0L,
0Xd6ad50a5L, 0Xd26c4d12L, 0Xdf2f6bcbL, 0Xdbee767cL,
0Xe3a1cbc1L, 0Xe760d676L, 0Xea23f0afL, 0Xeee2ed18L,
0Xf0a5bd1dL, 0Xf464a0aaL, 0Xf9278673L, 0Xfde69bc4L,
0X89b8fd09L, 0X8d79e0beL, 0X803ac667L, 0X84fbdbd0L,
0X9abc8bd5L, 0X9e7d9662L, 0X933eb0bbL, 0X97ffad0cL,
0Xafb010b1L, 0Xab710d06L, 0Xa6322bdfL, 0Xa2f33668L,
0Xbcb4666dL, 0Xb8757bdaL, 0Xb5365d03L, 0Xb1f740b4L
};

/**********************************************************************
*Name			:   uint32_t Crc32CalU32Bit( uint8_t *InBuff, uint32_t Length )
*Function       :   crc calibrate  the U32 data
*Para     		:   uint8_t *InBuff   the address of the crc array
*					uint32_t Length     the crc length of U32 data
*
*Return    		:   Crc32  the calculate result
*Version        :   REV1.0.0       
*Author:        :   feng
*
*History:
*REV1.0.0     feng    2018/1/29  Create
*********************************************************************/
uint32_t Crc32CalU32Bit(uint32_t *InBuff, uint32_t Length )
{
	
	uint32_t Crc32 = 0;//CRC32校验码初值
	uint8_t TabItem;
	uint8_t CrcTemp[4];
    uint8_t i = 0;
	while(Length--)
	{
		CrcTemp[0] = (*InBuff ) & 0xFF;
		CrcTemp[1] = (*InBuff >> 8) & 0xFF;
		CrcTemp[2] = (*InBuff >> 16) & 0xFF;
		CrcTemp[3] = (*InBuff >> 24)& 0xFF;
		InBuff ++;
		for(i = 0;i < 4;i++)
		{
			
			TabItem = ( Crc32 >> 24 )^ CrcTemp[i];
			Crc32 = ( Crc32 << 8 ) ^ Crc32Table[TabItem];			
		}
	}
	return Crc32;
}

/**********************************************************************
*Name			:  uint8_t calc_checksum(uint8_t const *p_data, int32_t data_len)
*Function       :   校验和
*Para     		:   uint8_t  *data 	 	  需要校验的数组
					int32_t data_len	  需要校验的数据长度	
*Return    		:   the Check_Sum calculate result
*Version        :   REV1.0.0       
*Author:        :   zlz
*
*History:
*REV1.0.0     zlz    2021/9/11  Create
**********************************************************************/
uint8_t calc_checksum(uint8_t *date, uint32_t date_len)
{
    uint8_t sum = 0;
	while(date_len--)
	{
		sum += *date++;
	}
	return sum;	
}
/**********************************************************************
*Name			:  uint8_t calc_Xor(unsigned int *dat, uint32_t data_len)
*Function       :   异或校验
*Para     		:   uint8_t  *data 		  需要异或校验的数组
					int32_t data_len	  需要异或的数据长度	
*Return    		:   the xor calculate result
*Version        :   REV1.0.0       
*Author:        :   zlz
*
*History:
*REV1.0.0     zlz    2021/9/11  Create
**********************************************************************/

uint8_t calc_Xor(uint8_t *date, uint32_t data_len)
{
	int i;
	uint8_t xor_value;
	xor_value = date[0];
	for(i=1;i<data_len;i++)
	{	
		xor_value ^= date[i];
	}
	return xor_value;
}