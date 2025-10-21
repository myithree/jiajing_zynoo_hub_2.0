#ifndef __AGREEMENT_H__
#define __AGREEMENT_H__

#include "stdlib.h"


#include "usbd_int.h"
#include "cdc_class.h"
#include "cdc_desc.h"

#include "at32f402_405_wk_config.h"
#include "usb_conf.h"
#include "usb_core.h"
#include "wk_system.h"
#include "ff_app.h"


/* 报文长度 */
#define APP_TX_DATA_LEN   0x40
#define APP_RX_DATA_LEN   USBD2_RX_SIZE * 4     //  960 * 4 byte

/* 包头快捷属性 */
#define HEAD     									(Vcp_Rx_Data[0])
#define DATA_LEN 									(Vcp_Rx_Data[1]<<8 | Vcp_Rx_Data[2])   //2byte
#define COMMAND     							(Vcp_Rx_Data[3])
#define MYCRC         						(Vcp_Rx_Data[4])
#define EXTRA_CRC   							(Vcp_Rx_Data[DATA_LEN+4])
#define receive_data              (Vcp_Rx_Data+5)

/* 发送协议头和校验位 */
#define MCU_STATE_POZ         5

#define MCU_HEAD_POZ        0
#define MCU_DATA_LEN_POZ_HIGH    1
#define MCU_DATA_LEN_POZ_LOW    2
#define MCU_COMMAND_POZ     3
#define MCU_CRC_POZ         4
#define MCU_ERR_POZ         5

#define MCU_CRC_RESULT1         5
#define MCU_CRC_RESULT2         6
#define MCU_CRC_RESULT3         7
#define MCU_CRC_RESULT4         8

#define MCU_REC_DATA_LEN_H         9
#define MCU_REC_DATA_LEN_L         10

/* 发送协议头和校验值 */
#define MCU_HEAD_VAL        0XAB

/* 协议错误码 */
#define HEAD_ERROR    0x00
#define TAIL_ERROR    0x01
#define LENTH_ERROR   0x02
#define CRC_ERROR     0x03
#define COMMAND_ERROR 0x04
#define EMPTY_ERROR   0xFF
#define ERROR_CMD    	0xFF
/* 程序跳转指令 */


/*GIF（图片）传输命令*/
#define 			 GIF_TRANS_NOTICE               0x10
#define        GIF_TRANS_PACKAGE              0x13
#define        GIF_TRANS_DONE                 0x16

/*GIF（图片）设置命令*/
#define        GIF_SET_ORDER                  0x30
/* IAP命令 */
#define        KB2_BL_SYNC        		  0x01    //同步

/* 固件更新参数设置 */
#define 			 QUEST_ERASE						  0x52	
#define 		   FIRST_PACKAGE				 	  0x53	
#define  		 	 APP_PACKAGE 						  0x54
#define        UPDATE_RESULT    		    0x55
#define        JUMP_TO_APP    		    	0x56

extern uint32_t Updata_state_adderss;  //固件更新烧录时的烧录地址索引

extern uint8_t program_state;  //是否擦除完成状态



extern uint32_t crc_result;
extern uint8_t commade_case;
extern uint8_t Vcp_Rx_Data[APP_RX_DATA_LEN];
extern uint16_t Vcp_Rx_Data_length;
extern uint8_t SYNC[APP_TX_DATA_LEN];


typedef struct _RUN_STATE //运行标志
{
		uint32_t boot;     //默认是app状态0xffffff,
		uint32_t APP_STATE;//最初应是0xffffffff,每次进到app后应进行0xffffffff赋值，固件更新时将其赋为为零	
		uint32_t buffer2[6];
}__attribute__((aligned(32)))RUN_STATE_STRUCT;

/* 固件更新状态宏 */


/* 用来增加固件更新的可靠性，防止变砖 */




/* 接收数据预处理函数 */
void Get_Vcp_Data(uint8_t* data);

/* 数据处理函数 */
void Execute_Command(void);

/* 发送接口函数 */
void Vcp_Send_Message(uint8_t* data);

/*回应GIF上传通知*/
void Response_trans_notice(void);

/*回应GIF CRC校验值与长度*/
void Response_gif_trans_packet(void); 

/*回应GIF保存是否完整*/
void Response_gif_trans_done(void);
#endif


