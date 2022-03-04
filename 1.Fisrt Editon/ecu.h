#include "BaseDefine.h"
extern g_tms570_errflag;

int8_t ECU_Diagnose_Standby(TMS570_BRAM_DATA *blvds_bram_wr_data,TMS570_BRAM_DATA *bram_tms570_12_data,ECU_ERROR_INFO *err_info);
int8_t FCU_Start_Stage(TMS570_BRAM_DATA *bram_tms_wr_data,TMS570_BRAM_DATA *bram_tms_rd_data,TMS570_BRAM_DATA *bram_rd_data,TMS570_BRAM_DATA *bram_wr_data);
int8_t ECU_Diagnose_EV(TMS570_BRAM_DATA *bram_tms_wr_9_11_data,ECU_ERROR_INFO *err_info);
int8_t ECU_Diagnose_Hybrid(uint8_t *buffer,TMS570_BRAM_DATA *tms_wr_data_9_11,TMS570_BRAM_DATA *tms_wr_data_12,ECU_ERROR_INFO *err_info);
int8_t Hybrid_Power_ctrl(int8_t *buffer,TMS570_BRAM_DATA *tms_wr_data_8,ECU_ERROR_INFO *err_info);
int8_t EV_Power_ctrl(int8_t *buffer,TMS570_BRAM_DATA *tms_wr_data_8,ECU_ERROR_INFO *err_info);
int8_t run_on_process(uint8_t *fault_buffer,uint8_t *power_buffer,TMS570_BRAM_DATA *tms_rd_data_8,TMS570_BRAM_DATA *tms_rd_data_9_11,TMS570_BRAM_DATA *tms_rd_data_12);
int8_t ECU_Shut_down();