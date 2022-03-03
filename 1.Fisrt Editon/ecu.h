#include "BaseDefine.h"
extern g_tms570_errflag;

int8_t ECU_Diagnose_function(TMS570_BRAM_DATA *bram_rd_data,TMS570_BRAM_DATA *bram_wr_data,TMS570_BRAM_DATA *bram_tms570_data,ECU_ERROR_INFO err_info);
int8_t FCU_Start_Stage(TMS570_BRAM_DATA *bram_tms_wr_data,TMS570_BRAM_DATA *bram_tms_rd_data,TMS570_BRAM_DATA *bram_rd_data,TMS570_BRAM_DATA *bram_wr_data);