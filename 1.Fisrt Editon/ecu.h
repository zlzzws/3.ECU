#include "BaseDefine.h"
extern g_tms570_errflag;

#define DIAGNOSE_STANDBY 0
#define DIAGNOSE_EV_HYBRID 1

int8_t FCU_Start_Stage(TMS570_BRAM_DATA *bram_tms_wr_data,TMS570_BRAM_DATA *bram_tms_rd_data,TMS570_BRAM_DATA *bram_rd_data,TMS570_BRAM_DATA *bram_wr_data,ECU_ERROR_INFO *err_info);
int8_t ECU_Diagnose(uint8_t *fault_buffer,TMS570_BRAM_DATA *blvds_bram_rd_data,TMS570_BRAM_DATA *blvds_bram_wr_data,\
                    TMS570_BRAM_DATA *tms_wr_data_9_11,TMS570_BRAM_DATA *tms_wr_data_12,ECU_ERROR_INFO *err_info,uint8_t diagnose_type);
int8_t Hybrid_Power_ctrl(int8_t *power_buffer,TMS570_BRAM_DATA *tms_wr_data_8,TMS570_BRAM_DATA *tms_wr_data_9_11,ECU_ERROR_INFO *err_info);
int8_t EV_Power_ctrl(int8_t *power_buffer,TMS570_BRAM_DATA *tms_wr_data_8,ECU_ERROR_INFO *err_info);
int8_t Send_Data_binding(uint8_t *fault_buffer,uint8_t *power_buffer,TMS570_BRAM_DATA *tms_rd_data_8,TMS570_BRAM_DATA *tms_rd_data_9_11,TMS570_BRAM_DATA *tms_rd_data_12);
int8_t ECU_Shut_down();