#ifdef  REDUNDANCY_FUNCION

#include "BaseDefine.h"
#include "math.h"

extern g_tms570_errflag;

#define DIAGNOSE_STANDBY 0      /*车辆上电初期，standby模式的诊断*/
#define DIAGNOSE_HYBRID 1       /*车辆混动诊断*/
#define DIAGNOSE_EV 2           /*车辆混动诊断*/
#define Z_COMPRESSION_FACTOR   {1.00018,-0.0022546,0.01053,-0.013205,\
                                -0.00067291,0.028051,-0.024126,-0.0058663,\
                                0.000010817,-0.00012653,0.00019788,0.00085677,\
                                -1.44E-07,1.22E-06,7.76E-07,-1.74E-05,\
                                1.24E-09,-8.97E-09,-1.67E-08,1.47E-07,\
                                -4.47E-12,3.03E-11,6.33E-11,-4.70E-10}



int8_t FCU_Start_Stage(TMS570_BRAM_DATA *bram_tms_wr_data,TMS570_BRAM_DATA *bram_tms_rd_data,TMS570_BRAM_DATA *bram_rd_data,TMS570_BRAM_DATA *bram_wr_data,ECU_ERROR_INFO *err_info);
int8_t ECU_Diagnose(uint8_t *fault_buffer,TMS570_BRAM_DATA *blvds_bram_rd_data,TMS570_BRAM_DATA *blvds_bram_wr_data,\
                    TMS570_BRAM_DATA *tms_wr_data_9_11,TMS570_BRAM_DATA *tms_wr_data_12,ECU_ERROR_INFO *err_info,uint8_t diagnose_type);
int8_t Hybrid_Power_ctrl(int8_t *power_buffer,TMS570_BRAM_DATA *tms_wr_data_8,TMS570_BRAM_DATA *tms_wr_data_9_11,ECU_ERROR_INFO *err_info);
int8_t Send_Data_binding(uint8_t *fault_buffer,uint8_t *power_buffer,TMS570_BRAM_DATA *tms_rd_data_8,TMS570_BRAM_DATA *tms_rd_data_9_11,TMS570_BRAM_DATA *tms_rd_data_12);
int8_t FCU_DCDC_Shut_down();
int8_t EV_Shut_down();
#endif