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
#include "get_current_time.h"

/* 报文长度 */
#define APP_TX_DATA_LEN   0x40
#define APP_RX_DATA_LEN   512     //  128 * 4 byte

/* 包头快捷属性 */
#define HEAD     									(Vcp_Rx_Data[0])
#define DATA_LEN 									(((uint32_t)Vcp_Rx_Data[1])<<24 | ((uint32_t)Vcp_Rx_Data[2])<<16 | ((uint32_t)Vcp_Rx_Data[3])<<8 | ((uint32_t)Vcp_Rx_Data[4]))  //2byte
#define COMMAND     							(Vcp_Rx_Data[5])
#define RECEIVE_CRC         			(Vcp_Rx_Data[6])
#define DATA_FINL   							(Vcp_Rx_Data[DATA_LEN+6])
#define receive_data              (Vcp_Rx_Data+8)


#define FRAME_DATA_SIZE               LCD_Width * LCD_Height * 2    //单位byte
/* 发送协议头和校验位 */
#define MCU_STATE_POZ         	7

#define MCU_HEAD_POZ        		0

#define MCU_DATA_LEN_POZ1      1
#define MCU_DATA_LEN_POZ2      2
#define MCU_DATA_LEN_POZ3      3
#define MCU_DATA_LEN_POZ4      4

#define MCU_COMMAND_POZ     		5
#define MCU_CRC_POZ         		6
#define MCU_ERR_POZ         		7

#define MCU_CRC_RESULT1         8
#define MCU_CRC_RESULT2         9
#define MCU_CRC_RESULT3         10
#define MCU_CRC_RESULT4         11


#define MCU_HEAD_VAL        0XAB

#define Nondata_Part_Len        12

/* 协议错误码 */
#define HEAD_ERROR    0x00
#define TAIL_ERROR    0x01
#define LENTH_ERROR   0x02
#define CRC_ERROR     0x03
#define COMMAND_ERROR 0x04
#define EMPTY_ERROR   0xFF
#define ERROR_CMD    	0xFF
/* 程序跳转指令 */







extern uint16_t receive_frame_number;//接收到的帧数
extern uint32_t Updata_state_adderss;  //固件更新烧录时的烧录地址索引

extern uint8_t program_state;  //是否擦除完成状态


extern uint32_t data_trans_len;
extern uint32_t receive_crc;
extern uint32_t crc_result;
extern uint8_t Vcp_Rx_Data[APP_RX_DATA_LEN];
extern uint16_t Vcp_Rx_Data_length;
extern uint8_t SYNC[APP_TX_DATA_LEN];


typedef enum
{

	/*GIF（图片）传输命令*/
  GIF_TRANS_NOTICE        =       0x70,
  GIF_TRANS_PACKAGE_TRANS_COMMAND       =       0x71,/*!< 接收数据包状态 */
	GIF_TRANS_PACKAGE_TRANS_DATA       =       0xFF,/*!<接收整包并直接存储 */
	
	/*GIF（图片）设置命令*/
	GIF_SET_ORDER          =        0x72,
/* IAP命令 */
  KB2_BL_SYNC         =  		  0x01,    //同步

/* 固件更新参数设置 */
	QUEST_ERASE				=		  0x52,	
  FIRST_PACKAGE			=	 	  0x53,	
 	APP_PACKAGE 			=			  0x54,
  UPDATE_RESULT    	=	    0x55,
  JUMP_TO_APP    		=    	0x56,
 
} trans_state_enum;

extern trans_state_enum commade_case;
/* 固件更新状态宏 */


/* 用来增加固件更新的可靠性，防止变砖 */





/* 包处理函数 */
void Execute_Command(void);
void Command_Process(void);
/* 发送接口函数 */
void Vcp_Send_Message(uint8_t* data);

/*回应GIF上传通知*/
void Response_trans_notice(void);

/*回应GIF CRC校验值与长度*/
void Response_gif_trans_packet(void); 

#endif


