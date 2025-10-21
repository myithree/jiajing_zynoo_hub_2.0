#include "Agreement.h"

extern otg_core_type otg_core_struct_hs; // customHID 句柄
uint8_t usb_send[APP_TX_DATA_LEN] = {0};	 // 发送缓冲区:

uint8_t Vcp_Rx_Data[APP_RX_DATA_LEN] = {0};
uint16_t Vcp_Rx_Data_length = 0;
uint32_t Updata_state_adderss = 0x0; // 地址索引

uint32_t crc_result = 0;

uint8_t program_state = 0;
uint8_t update_flag = 1;
uint8_t commade_case = 0;

uint8_t SYNC[APP_TX_DATA_LEN] =
		{
			0xAB, 0X00, 0x22, (KB2_BL_SYNC), (0x22 + KB2_BL_SYNC),
			0x00,																														  // ERROR_CODE??1
			0x01,																														  // run_mode??2iap   1app
			0x02, 0x02, 0x21, 0x21, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, // SN码21
			0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x2E, 0x30, 0x2E, 0x31															  // VERSION码11
		};

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     USB发送函数接口，使用这个函数发送
// 参数说明     *data           发送数据指针
// 返回参数     void
// 使用示例     无
// 备注信息     必须发送64字节,如果发不够就补0
//-------------------------------------------------------------------------------------------------------------------
void Vcp_Send_Message(uint8_t *data)
{
	uint32_t timeout = 0;
	timeout = 5000000;
	do
	{
		if(usb_vcp_send_data(&otg_core_struct_hs.dev, (uint8_t *)data, APP_TX_DATA_LEN) == SUCCESS)
		{
			break;
		}
	}while(timeout --);
}

/************************************************************* USB包预处理 ****************************************************************/
//-------------------------------------------------------------------------------------------------------------------
// 函数简介     USB接收数据接口，使用这个函数接收数据
// 参数说明     *data            传入数据指针(64字节)
// 返回参数     void
// 备注信息     建议放在接收中断
//-------------------------------------------------------------------------------------------------------------------	77777777777777
void Get_Vcp_Data(uint8_t *data)
{
	uint8_t l;
	
  if(Vcp_Rx_Data_length > 0)
  {
   
		if (HEAD != 0xAB) // 检查包头
		{
			Vcp_Send_Message((uint8_t *)"HEAD_ERROR_IAP"); // 包头异常
			return;
		}

		if (DATA_LEN > 0)
		{
			l = (uint8_t)(DATA_LEN + COMMAND + EXTRA_CRC);
		}

		else
		{
			l = (uint8_t)(DATA_LEN + COMMAND);
		}

		if (MYCRC != l)
		{
			Vcp_Send_Message((uint8_t *)"CRC_ERROR_IAP");
			return;
		}
		Execute_Command();
  }
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     USB包处理函数
// 参数说明     command         命令
// 参数说明     *data           传入的数据
// 参数说明     len             数据长度
// 返回参数     void
// 备注信息     务必不能让数据指针越界，以免碰到不该碰到的数
//-------------------------------------------------------------------------------------------------------------------
void Execute_Command(void)
{
	
	switch (COMMAND) // 命令字
	{
		case KB2_BL_SYNC: // 签名----------------------------------------------------------------
			commade_case = KB2_BL_SYNC;
			break;

		case GIF_TRANS_NOTICE:			     // 回应上传通知-----------------------------------------------------
			commade_case = GIF_TRANS_NOTICE;
			break;
		case GIF_TRANS_PACKAGE:												//----------------------------------
			commade_case = GIF_TRANS_PACKAGE;
			break;
		case GIF_TRANS_DONE: //-----------------------------------------------------------
			commade_case = GIF_TRANS_DONE;
			break;
		case GIF_SET_ORDER: //-----------------------------------------------------------
			commade_case = GIF_SET_ORDER;
			break;

		default:
			break;
	}
}

/**************************************************************** 指令执行 *****************************************************************/
//-------------------------------------------------------------------------------------------------------------------
// 函数简介     回应GIF上传通知
// 参数说明
// 返回参数
// 使用示例
// 备注信息
//-------------------------------------------------------------------------------------------------------------------

void Response_trans_notice(void) //
{
	uint8_t SYNC[APP_TX_DATA_LEN] = {0};	
	SYNC[MCU_HEAD_POZ] = MCU_HEAD_VAL;
	SYNC[MCU_DATA_LEN_POZ_HIGH] = 0X00;
	SYNC[MCU_DATA_LEN_POZ_LOW] = 0x01;
	SYNC[MCU_COMMAND_POZ] = GIF_TRANS_NOTICE;
	SYNC[MCU_CRC_POZ] = (uint8_t)(SYNC[MCU_DATA_LEN_POZ_HIGH]<<8 | SYNC[MCU_DATA_LEN_POZ_LOW])
		+ SYNC[MCU_COMMAND_POZ] 
		+ SYNC[MCU_CRC_POZ + (uint8_t)(SYNC[MCU_DATA_LEN_POZ_HIGH]<<8 | SYNC[MCU_DATA_LEN_POZ_LOW])];
	SYNC[MCU_STATE_POZ] = 0x01;
	
	Vcp_Send_Message(SYNC);
}
//-------------------------------------------------------------------------------------------------------------------
// 函数简介     回应GIF包校验
// 参数说明
// 返回参数
// 使用示例
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
void Response_gif_trans_packet(void)
{
	uint8_t SYNC[APP_TX_DATA_LEN] = {0};
	
	SYNC[MCU_HEAD_POZ] = MCU_HEAD_VAL;
	SYNC[MCU_DATA_LEN_POZ_HIGH] = 0X00;
	SYNC[MCU_DATA_LEN_POZ_LOW] = 0x06;
	SYNC[MCU_COMMAND_POZ] = GIF_TRANS_PACKAGE;
	
	SYNC[MCU_REC_DATA_LEN_H] = (uint8_t)(Vcp_Rx_Data_length >> (8*1));
	SYNC[MCU_REC_DATA_LEN_L] = (uint8_t)Vcp_Rx_Data_length;
	
	SYNC[MCU_CRC_RESULT1] = (uint8_t)(crc_result >> (8*3));
	SYNC[MCU_CRC_RESULT2] = (uint8_t)(crc_result >> (8*2));
	SYNC[MCU_CRC_RESULT3] = (uint8_t)(crc_result >> (8*1));
	SYNC[MCU_CRC_RESULT4] = (uint8_t)crc_result;
	
	SYNC[MCU_CRC_POZ] = (uint8_t)(SYNC[MCU_DATA_LEN_POZ_HIGH]<<8 | SYNC[MCU_DATA_LEN_POZ_LOW])
		+ SYNC[MCU_COMMAND_POZ] 
		+ SYNC[MCU_CRC_POZ + (SYNC[MCU_DATA_LEN_POZ_HIGH]<<8 | SYNC[MCU_DATA_LEN_POZ_LOW])];
	
 
	
	
	Vcp_Send_Message(SYNC);	
}
//-------------------------------------------------------------------------------------------------------------------
// 函数简介     回应GIF保存是否完成
// 参数说明
// 返回参数
// 使用示例
// 备注信息
//-------------------------------------------------------------------------------------------------------------------

void Response_gif_trans_done(void) //
{
	uint8_t SYNC[APP_TX_DATA_LEN] = {0};	
	SYNC[MCU_HEAD_POZ] = MCU_HEAD_VAL;
	SYNC[MCU_DATA_LEN_POZ_HIGH] = 0X00;
	SYNC[MCU_DATA_LEN_POZ_LOW] = 0x02;
	SYNC[MCU_COMMAND_POZ] = GIF_TRANS_DONE;
	SYNC[MCU_CRC_POZ] = (uint8_t)(SYNC[MCU_DATA_LEN_POZ_HIGH]<<8 | SYNC[MCU_DATA_LEN_POZ_LOW])
		+ SYNC[MCU_COMMAND_POZ] 
		+ SYNC[MCU_CRC_POZ + (uint8_t)(SYNC[MCU_DATA_LEN_POZ_HIGH]<<8 | SYNC[MCU_DATA_LEN_POZ_LOW])];
	SYNC[MCU_ERR_POZ] = 0x00;
	Vcp_Send_Message(SYNC);
}
