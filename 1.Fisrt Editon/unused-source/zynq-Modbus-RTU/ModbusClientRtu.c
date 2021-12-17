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
*Author:        :   deng
*
*History:
*REV1.0.0     deng    2018/8/27  Create
*REV1.0.1     deng    2018/8/31 
*
*********************************************************************/
#if 0
/**********************************************************************
*
*Include file  section
*
*********************************************************************/
#include "ModbusClientRtu.h"
#include "GPIOControl.h"

/**********************************************************************
*
*Global Variable Declare Section
*
*********************************************************************/
extern DEBUG_TYPE_ENUM g_DebugType_EU;
extern const uint8_t UT_BITS_TAB[];
extern const uint8_t UT_INPUT_BITS_TAB[];
extern const uint16_t UT_REGISTERS_TAB[];
extern const uint16_t UT_INPUT_REGISTERS_TAB[];
extern modbus_t *g_ModbusCtx;
extern uint32_t g_RTUUsec_U32;
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
/**********************************************************************
*Name           :   void LightTest()  
*Function       :   lighten the lamp of Voltage-high Check feedback
*               :   
*Para           :   
*
*Return         :   
*Version        :   REV1.0.0       
*Author:        :   deng
*
*History:
*REV1.0.0     deng    2018/8/27  Create
*
*********************************************************************/


/*
 * Copyright © 2001-2011 Stéphane Raimbault <stephane.raimbault@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1+
 *
 * This library implements the Modbus protocol.
 * http://libmodbus.org/
 */



const char *modbus_strerror(int errnum) {
    switch (errnum) {
    case EMBXILFUN:
        return "Illegal function";
    case EMBXILADD:
        return "Illegal data address";
    case EMBXILVAL:
        return "Illegal data value";
    case EMBXSFAIL:
        return "Slave device or server failure";
    case EMBXACK:
        return "Acknowledge";
    case EMBXSBUSY:
        return "Slave device or server is busy";
    case EMBXNACK:
        return "Negative acknowledge";
    case EMBXMEMPAR:
        return "Memory parity error";
    case EMBXGPATH:
        return "Gateway path unavailable";
    case EMBXGTAR:
        return "Target device failed to respond";
    case EMBBADCRC:
        return "Invalid CRC";
    case EMBBADDATA:
        return "Invalid data";
    case EMBBADEXC:
        return "Invalid exception code";
    case EMBMDATA:
        return "Too many data";
    case EMBBADSLAVE:
        return "Response not from requested slave";
    default:
        return strerror(errnum);
    }
}


void _error_print(modbus_t *ctx, const char *context)
{
    if (ctx->debug) {
        fprintf(stderr, "ERROR %s", modbus_strerror(errno));
        if (context != NULL) {
            fprintf(stderr, ": %s\n", context);
        } else {
            fprintf(stderr, "\n");
        }
    }
}


static void _sleep_response_timeout(modbus_t *ctx)
{
    /* Response timeout is always positive */
#ifdef _WIN32
    /* usleep doesn't exist on Windows */
    Sleep((ctx->response_timeout.tv_sec * 1000) +
          (ctx->response_timeout.tv_usec / 1000));
#else
    /* usleep source code */
    struct timespec request, remaining;
    request.tv_sec = ctx->response_timeout.tv_sec;
    request.tv_nsec = ((long int)ctx->response_timeout.tv_usec) * 1000;
    while (nanosleep(&request, &remaining) == -1 && errno == EINTR) {
        request = remaining;
    }
#endif
}

/*
 *  ---------- Request     Indication ----------
 *  | Client | ---------------------->| Server |
 *  ---------- Confirmation  Response ----------
 */

/* Computes the length to read after the function received */
static uint8_t compute_meta_length_after_function(int function,
                                                  msg_type_t msg_type)
{
    int length;

    if (msg_type == MSG_INDICATION) {
        if (function <= MODBUS_FC_WRITE_SINGLE_REGISTER) {
            length = 4;
        } else if (function == MODBUS_FC_WRITE_MULTIPLE_COILS ||
                   function == MODBUS_FC_WRITE_MULTIPLE_REGISTERS) {
            length = 5;
        } else if (function == MODBUS_FC_MASK_WRITE_REGISTER) {
            length = 6;
        } else if (function == MODBUS_FC_WRITE_AND_READ_REGISTERS) {
            length = 9;
        } else {
            /* MODBUS_FC_READ_EXCEPTION_STATUS, MODBUS_FC_REPORT_SLAVE_ID */
            length = 0;
        }
    } else {
        /* MSG_CONFIRMATION */
        switch (function) {
        case MODBUS_FC_WRITE_SINGLE_COIL:
        case MODBUS_FC_WRITE_SINGLE_REGISTER:
        case MODBUS_FC_WRITE_MULTIPLE_COILS:
        case MODBUS_FC_WRITE_MULTIPLE_REGISTERS:
            length = 4;
            break;
        case MODBUS_FC_MASK_WRITE_REGISTER:
            length = 6;
            break;
        default:
            length = 1;
        }
    }

    return length;
}

/* Computes the length to read after the meta information (address, count, etc) */
static int compute_data_length_after_meta(modbus_t *ctx, uint8_t *msg,
                                          msg_type_t msg_type)
{
    int function = msg[ctx->backend->header_length];
    int length;

    if (msg_type == MSG_INDICATION) {
        switch (function) {
        case MODBUS_FC_WRITE_MULTIPLE_COILS:
        case MODBUS_FC_WRITE_MULTIPLE_REGISTERS:
            length = msg[ctx->backend->header_length + 5];
            break;
        case MODBUS_FC_WRITE_AND_READ_REGISTERS:
            length = msg[ctx->backend->header_length + 9];
            break;
        default:
            length = 0;
        }
    } else {
        /* MSG_CONFIRMATION */
        if (function <= MODBUS_FC_READ_INPUT_REGISTERS ||
            function == MODBUS_FC_REPORT_SLAVE_ID ||
            function == MODBUS_FC_WRITE_AND_READ_REGISTERS) {
            length = msg[ctx->backend->header_length + 1];
        } else {
            length = 0;
        }
    }

    length += ctx->backend->checksum_length;//ctx->backend->checksum_length ==2

    return length;
}

/* Computes the length of the expected response */
static unsigned int compute_response_length_from_request(modbus_t *ctx, uint8_t *req)
{
    int length;
    const int offset = ctx->backend->header_length;

    switch (req[offset]) {
    case MODBUS_FC_READ_COILS:
    case MODBUS_FC_READ_DISCRETE_INPUTS: {
        /* Header + nb values (code from write_bits) */
        int nb = (req[offset + 3] << 8) | req[offset + 4];
        length = 2 + (nb / 8) + ((nb % 8) ? 1 : 0);
    }
        break;
    case MODBUS_FC_WRITE_AND_READ_REGISTERS:
    case MODBUS_FC_READ_HOLDING_REGISTERS:
    case MODBUS_FC_READ_INPUT_REGISTERS:
        /* Header + 2 * nb values */
        length = 2 + 2 * (req[offset + 3] << 8 | req[offset + 4]);
        break;
    case MODBUS_FC_READ_EXCEPTION_STATUS:
        length = 3;
        break;
    case MODBUS_FC_REPORT_SLAVE_ID:
        /* The response is device specific (the header provides the
           length) */
        return MSG_LENGTH_UNDEFINED;
    case MODBUS_FC_MASK_WRITE_REGISTER:
        length = 7;
        break;
    default:
        length = 5;
    }

    return offset + length + ctx->backend->checksum_length;
}

static int check_confirmation(modbus_t *ctx, uint8_t *req,
                              uint8_t *rsp, int rsp_length)
{
    int rc;
    int rsp_length_computed;
    const int offset = ctx->backend->header_length;
    const int function = rsp[offset];

    if (ctx->backend->pre_check_confirmation) {
        rc = ctx->backend->pre_check_confirmation(ctx, req, rsp, rsp_length);
        if (rc == -1) {
            if (ctx->error_recovery & MODBUS_ERROR_RECOVERY_PROTOCOL) {
                _sleep_response_timeout(ctx);
                modbus_flush(ctx);
            }
            return -1;
        }
    }

    rsp_length_computed = compute_response_length_from_request(ctx, req);

    /* Exception code */
    if (function >= 0x80) {
        if (rsp_length == (offset + 2 + (int)ctx->backend->checksum_length) &&
            req[offset] == (rsp[offset] - 0x80)) {
            /* Valid exception code received */

            int exception_code = rsp[offset + 1];
            if (exception_code < MODBUS_EXCEPTION_MAX) {
                errno = MODBUS_ENOBASE + exception_code;
            } else {
                errno = EMBBADEXC;
            }
            _error_print(ctx, NULL);
            return -1;
        } else {
            errno = EMBBADEXC;
            _error_print(ctx, NULL);
            return -1;
        }
    }

    /* Check length */
    if ((rsp_length == rsp_length_computed ||
         rsp_length_computed == MSG_LENGTH_UNDEFINED) &&
        function < 0x80) {
        int req_nb_value;
        int rsp_nb_value;

        /* Check function code */
        if (function != req[offset]) {
            if (ctx->debug) {
                fprintf(stderr,
                        "Received function not corresponding to the request (0x%X != 0x%X)\n",
                        function, req[offset]);
            }
            if (ctx->error_recovery & MODBUS_ERROR_RECOVERY_PROTOCOL) {
                _sleep_response_timeout(ctx);
                modbus_flush(ctx);
            }
            errno = EMBBADDATA;
            return -1;
        }

        /* Check the number of values is corresponding to the request */
        switch (function) {
        case MODBUS_FC_READ_COILS:
        case MODBUS_FC_READ_DISCRETE_INPUTS:
            /* Read functions, 8 values in a byte (nb
             * of values in the request and byte count in
             * the response. */
            req_nb_value = (req[offset + 3] << 8) + req[offset + 4];
            req_nb_value = (req_nb_value / 8) + ((req_nb_value % 8) ? 1 : 0);
            rsp_nb_value = rsp[offset + 1];
            break;
        case MODBUS_FC_WRITE_AND_READ_REGISTERS:
        case MODBUS_FC_READ_HOLDING_REGISTERS:
        case MODBUS_FC_READ_INPUT_REGISTERS:
            /* Read functions 1 value = 2 bytes */
            req_nb_value = (req[offset + 3] << 8) + req[offset + 4];
            rsp_nb_value = (rsp[offset + 1] / 2);
            break;
        case MODBUS_FC_WRITE_MULTIPLE_COILS:
        case MODBUS_FC_WRITE_MULTIPLE_REGISTERS:
            /* N Write functions */
            req_nb_value = (req[offset + 3] << 8) + req[offset + 4];
            rsp_nb_value = (rsp[offset + 3] << 8) | rsp[offset + 4];
            break;
        case MODBUS_FC_REPORT_SLAVE_ID:
            /* Report slave ID (bytes received) */
            req_nb_value = rsp_nb_value = rsp[offset + 1];
            break;
        default:
            /* 1 Write functions & others */
            req_nb_value = rsp_nb_value = 1;
        }

        if (req_nb_value == rsp_nb_value) {
            rc = rsp_nb_value;
        } else {
            if (ctx->debug) {
                fprintf(stderr,
                        "Quantity not corresponding to the request (%d != %d)\n",
                        rsp_nb_value, req_nb_value);
            }

            if (ctx->error_recovery & MODBUS_ERROR_RECOVERY_PROTOCOL) {
                _sleep_response_timeout(ctx);
                modbus_flush(ctx);
            }

            errno = EMBBADDATA;
            rc = -1;
        }
    } else {
        if (ctx->debug) {
            fprintf(stderr,
                    "Message length not corresponding to the computed length (%d != %d)\n",
                    rsp_length, rsp_length_computed);
        }
        if (ctx->error_recovery & MODBUS_ERROR_RECOVERY_PROTOCOL) {
            _sleep_response_timeout(ctx);
            modbus_flush(ctx);
        }
        errno = EMBBADDATA;
        rc = -1;
    }

    return rc;
}





/* Sends a request/response */
static int send_msg(modbus_t *ctx, uint8_t *msg, int msg_length)
{
    int rc;
    int i;

    msg_length = ctx->backend->send_msg_pre(msg, msg_length);//返回值应该是rc

    if (ctx->debug) {
        for (i = 0; i < msg_length; i++)
            printf("[%.2X]", msg[i]);
        printf("\n");
    }

    /* In recovery mode, the write command will be issued until to be
       successful! Disabled by default. */
    do {
        rc = ctx->backend->send(ctx, msg, msg_length);
        if (rc == -1) {
            _error_print(ctx, NULL);
            if (ctx->error_recovery & MODBUS_ERROR_RECOVERY_LINK) {
                int saved_errno = errno;

                if ((errno == EBADF || errno == ECONNRESET || errno == EPIPE)) {
                    modbus_close(ctx);
                    _sleep_response_timeout(ctx);
                    modbus_connect(ctx);
                } else {
                    _sleep_response_timeout(ctx);
                    modbus_flush(ctx);
                }
                errno = saved_errno;
            }
        }
    } while ((ctx->error_recovery & MODBUS_ERROR_RECOVERY_LINK) &&
             rc == -1);

    if (rc > 0 && rc != msg_length) {
        errno = EMBBADDATA;
        return -1;
    }

    return rc;
}

int _modbus_receive_msg(modbus_t *ctx, uint8_t *msg, msg_type_t msg_type)
{
    int rc;
    fd_set rset;
    struct timeval tv;
    struct timeval *p_tv;
    int length_to_read;
    int msg_length = 0;
    _step_t step;

    if (ctx->debug)
	{
        if (msg_type == MSG_INDICATION) 
	    {
            printf("Waiting for a indication...\n");
        } 
		else 
		{
           printf("Waiting for a confirmation...\n");
         }
    }

    /* Add a file descriptor to the set */
    FD_ZERO(&rset);
    FD_SET(ctx->s, &rset);

    /* We need to analyse the message step by step.  At the first step, we want
     * to reach the function code because all packets contain this
     * information. */
    step = _STEP_FUNCTION;
    length_to_read = ctx->backend->header_length + 1;
    if (msg_type == MSG_INDICATION) {
        /* Wait for a message, we don't know when the message will be
         * received */
        p_tv = NULL;
    } 
	else 
	{
        tv.tv_sec = ctx->response_timeout.tv_sec;
        tv.tv_usec = ctx->response_timeout.tv_usec;
        p_tv = &tv;
    }

    while (length_to_read != 0) {
        rc = ctx->backend->select(ctx, &rset, p_tv, length_to_read);//当由信号时通知
        if (rc == -1) 
		{	//错误尝试机制
            _error_print(ctx, "select");
            if (ctx->error_recovery & MODBUS_ERROR_RECOVERY_LINK) 
			{
                int saved_errno = errno;

                if (errno == ETIMEDOUT) 
				{
                    _sleep_response_timeout(ctx);
                    modbus_flush(ctx);
                } 
				else if (errno == EBADF) 
			    {
                    modbus_close(ctx);
                    modbus_connect(ctx);
                }
                errno = saved_errno;
            }
            return -1;
        }

        rc = ctx->backend->recv(ctx, msg + msg_length, length_to_read);//将读取的数据放入rsp[260]缓存，返回实际读到的字节数
        if (rc == 0)
		{
            errno = ECONNRESET;
            rc = -1;
        }

        if (rc == -1) 
		{
            _error_print(ctx, "read");
            if ((ctx->error_recovery & MODBUS_ERROR_RECOVERY_LINK) &&(errno == ECONNRESET || errno == ECONNREFUSED ||errno == EBADF)) 
           {
                int saved_errno = errno;
                modbus_close(ctx);
                modbus_connect(ctx);
                /* Could be removed by previous calls */
                errno = saved_errno;
            }
            return -1;
        }

        /* Display the hex code of each character received */
        if (ctx->debug)
		{
            int i;
            for (i=0; i < rc; i++)
            printf("<%.2X>", msg[msg_length + i]);
        }

        /* Sums bytes received */
        msg_length += rc;
       /* Computes remaining bytes */
        length_to_read -= rc;
		//数据读取完成检查
        if (length_to_read == 0)
		{
            switch (step) 
			{
            case _STEP_FUNCTION:
                /* Function code position */
                length_to_read = compute_meta_length_after_function( msg[ctx->backend->header_length], msg_type);//第一帧，msg[1]是功能码
                if(length_to_read != 0)
                {
                    step = _STEP_META;//第二帧开始，step变为了_STEP_META 
                    break;
                } /* else switches straight to the next step */
            case _STEP_META:
                length_to_read = compute_data_length_after_meta(ctx, msg, msg_type);
                if ((msg_length + length_to_read) > (int)ctx->backend->max_adu_length) 
				{
                    errno = EMBBADDATA;
                    _error_print(ctx, "too many data");
                    return -1;
                }
                step = _STEP_DATA;
                break;
            default:
                break;
            }
        }

        if (length_to_read > 0 && (ctx->byte_timeout.tv_sec > 0 || ctx->byte_timeout.tv_usec > 0))
		{
            /* If there is no character in the buffer, the allowed timeout
               interval between two consecutive bytes is defined by byte_timeout */
            tv.tv_sec = ctx->byte_timeout.tv_sec;
            tv.tv_usec = ctx->byte_timeout.tv_usec;
            p_tv = &tv;
        }
        /* else timeout isn't set again, the full response must be read before
           expiration of response timeout (for CONFIRMATION only) */
    }

    if (ctx->debug)
        printf("\n");

    return ctx->backend->check_integrity(ctx, msg, msg_length);
}


/* Reads the data from a remove device and put that data into an array */
/*最终执行读取数据的函数*/
static int read_registers_lmu(modbus_t *ctx, int function, int addr, int nb,
                          uint16_t *dest)
{
    int rc;
    int req_length;
    uint8_t req[_MIN_REQ_LENGTH];//_MIN_REQ_LENGTH = 12
    uint8_t rsp[MAX_MESSAGE_LENGTH];
    static uint32_t s_rec_error_num = 0;
    static uint32_t s_send_num = 0;	

    if (nb > MODBUS_MAX_READ_REGISTERS) {
        if (ctx->debug) {
            fprintf(stderr,
                    "ERROR Too many registers requested (%d > %d)\n",
                    nb, MODBUS_MAX_READ_REGISTERS);
        }
        errno = EMBMDATA;
        return -1;
    }

    req_length = ctx->backend->build_request_basis(ctx, function, addr, nb, req);//构造modbus-RTU帧头（RTU帧的前6个字节，包括地址、功能码、寄存器地址、个数等信息）

    GPIO_RS484EnWrite(1);//2ms
    rc = send_msg(ctx, req, req_length);
    if (rc > 0) 
    {
        int offset;
        int i;
        s_send_num++;
        //when baud is set to 19200,g_RTUUsec_U32 = 4300,4000 will be ok,but baud 115200 will be error
        usleep(g_RTUUsec_U32);
        GPIO_RS484EnWrite(0);
        rc = _modbus_receive_msg(ctx, rsp, MSG_CONFIRMATION);
        if (rc == -1)
        {
            s_rec_error_num++;
            return -1;
        }               
        rc = check_confirmation(ctx, req, rsp, rc);
        if (rc == -1)
        {
            s_rec_error_num++;
            return -1;
        }

        offset = ctx->backend->header_length;

        if(g_DebugType_EU == RTU_DEBUG)
        {
            printf("s_send_num:%d \n ",s_send_num);
            printf("s_rec_error_num:%d \n ",s_rec_error_num);
        }
        for (i = 0; i < rc; i++) 
        {
            /* shift reg hi_byte to temp OR with lo_byte */
            dest[i] = (rsp[offset + 2 + (i << 1)] << 8) |
                rsp[offset + 3 + (i << 1)];
        }
    }
    return rc;
}

/* Reads the holding registers of remote device and put the data into an
   array */
int modbus_read_registers_lmu(modbus_t *ctx, int addr, int nb, uint16_t *dest)
{
    int status;

    if (ctx == NULL) 
    {
        errno = EINVAL;
        return -1;
    }

    if (nb > MODBUS_MAX_READ_REGISTERS) 
    {
        if (ctx->debug) 
        {
            fprintf(stderr,
                    "ERROR Too many registers requested (%d > %d)\n",
                    nb, MODBUS_MAX_READ_REGISTERS);
        }
        errno = EMBMDATA;
        return -1;
    }

    status = read_registers_lmu(ctx, MODBUS_FC_READ_HOLDING_REGISTERS,
                            addr, nb, dest);
    return status;
}

int RtuInit(int baudrate)
{
    const int NB_REPORT_SLAVE_ID = 10;//从设备ID
    uint16_t *tab_rp_registers_bad = NULL;
    int i;
    uint8_t value;
    int rc;
    float real;
    uint32_t old_response_to_sec;
    uint32_t old_response_to_usec;
    uint32_t new_response_to_sec;
    uint32_t new_response_to_usec;
    uint32_t old_byte_to_sec;
    uint32_t old_byte_to_usec;
    int use_backend;
   
    use_backend = RTU;
	//UART0对应ttyPS1
    g_ModbusCtx = modbus_new_rtu("/dev/ttyPS1", baudrate, 'N', 8, 1);//创建一个新的RTU环境（ctx），设置波特率，校验方式，数据位，停止位   
    if (g_ModbusCtx == NULL) 
    {
        fprintf(stderr, "Unable to allocate libmodbus context\n");
        return -1;
    }
    if(g_DebugType_EU == RTU_DEBUG)
    {
        modbus_set_debug(g_ModbusCtx, TRUE); 
    }
//用于设置连接失败或者不期望接收到的字节时应用的错误恢复模式
    modbus_set_error_recovery(g_ModbusCtx,
                              MODBUS_ERROR_RECOVERY_LINK |
                              MODBUS_ERROR_RECOVERY_PROTOCOL);
    
    if (use_backend == RTU) 
    {
        modbus_set_slave(g_ModbusCtx, RTU_HONGWAI_ID);//设置slave终端ID
    }
	//获取响应超时的具体时间
    modbus_get_response_timeout(g_ModbusCtx, &old_response_to_sec, &old_response_to_usec);
	
    if (modbus_connect(g_ModbusCtx) == -1)
    {
        fprintf(stderr, "Connection failed: %s\n", modbus_strerror(errno));
        modbus_free(g_ModbusCtx);
        return -1;
    }
	
    modbus_get_response_timeout(g_ModbusCtx, &new_response_to_sec, &new_response_to_usec);
    
}


int RtuReadRegister(uint16_t *tab_rp_registers)
{

    int i;
    int rc;
    /** HOLDING REGISTERS **/
    if(g_DebugType_EU == RTU_DEBUG)
    {
        printf("modbus_read_registers:\n ");
    }

    rc = modbus_read_registers_lmu(g_ModbusCtx, UT_REGISTERS_ADDRESS,
                               UT_REGISTERS_NB, tab_rp_registers);
    if(g_DebugType_EU == RTU_DEBUG)
    {
        for(i = 0; i < UT_REGISTERS_NB;i++)
        {
            printf("registe %d value %X\n",i,*tab_rp_registers);
            tab_rp_registers++;
        } 
    }
    /* End of single register */
}

int RtuReadRegister_DMU(uint16_t *tab_rp_registers,SENSOR_NUM_ENUM SenSorNum)
{

    int i;
    int rc;//返回值
    /** HOLDING REGISTERS **/
    if(g_DebugType_EU == RTU_DEBUG)
    {
        printf("modbus_read_registers:\n ");
    }
    switch(SenSorNum)
    {
        case Sensor_1:
        rc = modbus_read_registers_lmu(g_ModbusCtx, UT_REGISTERS_ADDRESS_SENSOR1,
                               UT_REGISTERS_NB_SENSOR, tab_rp_registers);
        break;
        case Sensor_2:
        rc = modbus_read_registers_lmu(g_ModbusCtx, UT_REGISTERS_ADDRESS_SENSOR2,
                               UT_REGISTERS_NB_SENSOR, tab_rp_registers);
        break;
        case Sensor_3:
        rc = modbus_read_registers_lmu(g_ModbusCtx, UT_REGISTERS_ADDRESS_SENSOR3,
                               UT_REGISTERS_NB_SENSOR, tab_rp_registers);
        break;
        case Sensor_4:
        rc = modbus_read_registers_lmu(g_ModbusCtx, UT_REGISTERS_ADDRESS_SENSOR4,
                               UT_REGISTERS_NB_SENSOR, tab_rp_registers);
        break;
        case Sensor_5:
        rc = modbus_read_registers_lmu(g_ModbusCtx, UT_REGISTERS_ADDRESS_SENSOR5,
                               UT_REGISTERS_NB_SENSOR, tab_rp_registers);
        break;
        case Sensor_6:
        rc = modbus_read_registers_lmu(g_ModbusCtx, UT_REGISTERS_ADDRESS_SENSOR6,
                               UT_REGISTERS_NB_SENSOR, tab_rp_registers);
        break;
        case Sensor_7:
        rc = modbus_read_registers_lmu(g_ModbusCtx, UT_REGISTERS_ADDRESS_SENSOR7,
                               UT_REGISTERS_NB_SENSOR, tab_rp_registers);
        break;
        case Sensor_8:
        rc = modbus_read_registers_lmu(g_ModbusCtx, UT_REGISTERS_ADDRESS_SENSOR8,
                               UT_REGISTERS_NB_SENSOR, tab_rp_registers);
        break;
        case Sensor_9:
        rc = modbus_read_registers_lmu(g_ModbusCtx, UT_REGISTERS_ADDRESS_SENSOR9,
                               UT_REGISTERS_NB_SENSOR, tab_rp_registers);
        break;
        case Sensor_10:
        rc = modbus_read_registers_lmu(g_ModbusCtx, UT_REGISTERS_ADDRESS_SENSOR10,
                               UT_REGISTERS_NB_SENSOR, tab_rp_registers);
        break;
        case Sensor_11:
        rc = modbus_read_registers_lmu(g_ModbusCtx, UT_REGISTERS_ADDRESS_SENSOR11,
                               UT_REGISTERS_NB_SENSOR, tab_rp_registers);
        break;
        case Sensor_12:
        rc = modbus_read_registers_lmu(g_ModbusCtx, UT_REGISTERS_ADDRESS_SENSOR12,
                               UT_REGISTERS_NB_SENSOR, tab_rp_registers);
        break;
        case Sensor_13:
        rc = modbus_read_registers_lmu(g_ModbusCtx, UT_REGISTERS_ADDRESS_SENSOR13,
                               UT_REGISTERS_NB_SENSOR, tab_rp_registers);
        break;
        case Sensor_14:
        rc = modbus_read_registers_lmu(g_ModbusCtx, UT_REGISTERS_ADDRESS_SENSOR14,
                               UT_REGISTERS_NB_SENSOR, tab_rp_registers);
        break;
        case Sensor_15:
        rc = modbus_read_registers_lmu(g_ModbusCtx, UT_REGISTERS_ADDRESS_SENSOR15,
                               UT_REGISTERS_NB_SENSOR, tab_rp_registers);
        break;
        case Sensor_16:
        rc = modbus_read_registers_lmu(g_ModbusCtx, UT_REGISTERS_ADDRESS_SENSOR16,
                               UT_REGISTERS_NB_SENSOR, tab_rp_registers);
        break;
        case Sensor_DMU:
        rc = modbus_read_registers_lmu(g_ModbusCtx, UT_REGISTERS_ADDRESS_DMU,
                               UT_REGISTERS_NB_DMU, tab_rp_registers);
        break;
        default:
        printf("Sensor Num Error\n");

    }
    if(g_DebugType_EU == RTU_DEBUG)
    {
        for(i = 0; i < 3;i++)
        {
            printf("registe %d value %X\n",i,*tab_rp_registers);
            tab_rp_registers++;
        } 
    }
    /* End of single register */
}

int RtuReadRegister_HongWai(uint16_t *tab_rp_registers,SENSOR_NUM_ENUM SenSorNum)
{

    int i;
    int rc;
    /** HOLDING REGISTERS **/
    if(g_DebugType_EU == RTU_DEBUG)
    {
        printf("modbus_read_registers:\n ");
    }
    switch(SenSorNum)
    {
        case Sensor_1:
        rc = modbus_read_registers_lmu(g_ModbusCtx, UT_REGISTERS_ADDRESS_NUM1,
                               UT_REGISTERS_NB_HONGWAI, tab_rp_registers);
        break;
        case Sensor_2:
        rc = modbus_read_registers_lmu(g_ModbusCtx, UT_REGISTERS_ADDRESS_NUM2,
                               UT_REGISTERS_NB_HONGWAI, tab_rp_registers);
        break;
        case Sensor_3:
        rc = modbus_read_registers_lmu(g_ModbusCtx, UT_REGISTERS_ADDRESS_NUM3,
                               UT_REGISTERS_NB_HONGWAI, tab_rp_registers);
        break;
        case Sensor_4:
        rc = modbus_read_registers_lmu(g_ModbusCtx, UT_REGISTERS_ADDRESS_NUM4,
                               UT_REGISTERS_NB_HONGWAI, tab_rp_registers);
        break;
        case Sensor_5:
        rc = modbus_read_registers_lmu(g_ModbusCtx, UT_REGISTERS_ADDRESS_NUM5,
                               UT_REGISTERS_NB_HONGWAI, tab_rp_registers);
        break;
        case Sensor_6:
        rc = modbus_read_registers_lmu(g_ModbusCtx, UT_REGISTERS_ADDRESS_NUM6,
                               UT_REGISTERS_NB_HONGWAI, tab_rp_registers);
        break;
        case Sensor_7:
        rc = modbus_read_registers_lmu(g_ModbusCtx, UT_REGISTERS_ADDRESS_NUM7,
                               UT_REGISTERS_NB_HONGWAI, tab_rp_registers);
        break;
        case Sensor_8:
        rc = modbus_read_registers_lmu(g_ModbusCtx, UT_REGISTERS_ADDRESS_NUM8,
                               UT_REGISTERS_NB_HONGWAI, tab_rp_registers);
        break;
        case Sensor_9:
        rc = modbus_read_registers_lmu(g_ModbusCtx, UT_REGISTERS_ADDRESS_NUM9,
                               UT_REGISTERS_NB_HONGWAI, tab_rp_registers);
        break;
        case Sensor_10:
        rc = modbus_read_registers_lmu(g_ModbusCtx, UT_REGISTERS_ADDRESS_NUM10,
                               UT_REGISTERS_NB_HONGWAI, tab_rp_registers);
        break;
        case Sensor_11:
        rc = modbus_read_registers_lmu(g_ModbusCtx, UT_REGISTERS_ADDRESS_NUM11,
                               UT_REGISTERS_NB_HONGWAI, tab_rp_registers);
        break;
        case Sensor_12:
        rc = modbus_read_registers_lmu(g_ModbusCtx, UT_REGISTERS_ADDRESS_NUM12,
                               UT_REGISTERS_NB_HONGWAI, tab_rp_registers);
        break;
        case Sensor_13:
        rc = modbus_read_registers_lmu(g_ModbusCtx, UT_REGISTERS_ADDRESS_NUM13,
                               UT_REGISTERS_NB_HONGWAI, tab_rp_registers);
        break;
        default:
        printf("Hongwai Num Error\n");

    }
    if(g_DebugType_EU == RTU_DEBUG)
    {
        for(i = 0; i < 3;i++)
        {
            printf("registe %d value %X\n",i,*tab_rp_registers);
            tab_rp_registers++;
        } 
    }
    /* End of single register */
}
#endif