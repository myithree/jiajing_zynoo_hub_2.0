#include "Agreement.h"

extern otg_core_type otg_core_struct_hs; // customHID 句柄
uint8_t usb_send[APP_TX_DATA_LEN] = {0};	 // 发送缓冲区:

uint8_t Vcp_Rx_Data[APP_RX_DATA_LEN] = {0};
uint16_t Vcp_Rx_Data_length = 0;
uint32_t Updata_state_adderss = 0x0; // 地址索引

uint32_t receive_crc = 0;
uint32_t crc_result = 0;
uint32_t data_trans_len = 0;
uint16_t receive_frame_number = 0;//接收到的帧数

uint16_t need_to_erase_sector_number = 0;
uint16_t need_to_erase_block_number = 0;
trans_state_enum commade_case;


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



//-------------------------------------------------------------------------------------------------------------------
// 函数简介     包处理函数
// 参数说明     void
// 参数说明     void
// 参数说明     void
// 返回参数     void
// 备注信息     
//-------------------------------------------------------------------------------------------------------------------
void Execute_Command(void)
{
	uint8_t checkout_crc;
  if(Vcp_Rx_Data_length > 0)
  { 
		if(commade_case != GIF_TRANS_PACKAGE_TRANS_DATA)//只接受数据时，不用处理包
		{
			if (HEAD != 0xAB) // 检查包头
			{
				Vcp_Send_Message((uint8_t *)"HEAD_ERROR_IAP"); // 包头异常
				return;
			}
			if(COMMAND != GIF_TRANS_PACKAGE_TRANS_COMMAND )
			{
				checkout_crc = (uint8_t)(DATA_LEN + COMMAND + DATA_FINL);
				if (checkout_crc != RECEIVE_CRC )
				{
					Vcp_Send_Message((uint8_t *)"CRC_ERROR_IAP");
					return;
				}
			}
			switch (COMMAND) // 命令字
			{
				case KB2_BL_SYNC: // 签名----------------------------------------------------------------
					commade_case = KB2_BL_SYNC;
					break;

				case GIF_TRANS_NOTICE:			     // 回应上传通知-----------------------------------------------------
					commade_case = GIF_TRANS_NOTICE;
					break;
				
				case GIF_TRANS_PACKAGE_TRANS_COMMAND:												//----------------------------------
					data_trans_len = 	DATA_LEN;
					receive_crc = ((uint32_t)Vcp_Rx_Data[MCU_CRC_RESULT1])<<24 | ((uint32_t)Vcp_Rx_Data[MCU_CRC_RESULT2])<<16 | ((uint32_t)Vcp_Rx_Data[MCU_CRC_RESULT3])<<8 | (uint32_t)Vcp_Rx_Data[MCU_CRC_RESULT4];
					commade_case = GIF_TRANS_PACKAGE_TRANS_COMMAND;
					break;
				
				case GIF_SET_ORDER: //-----------------------------------------------------------
					commade_case = GIF_SET_ORDER;
					break;

				default:
					break;
			}
		}
		Command_Process();
	}
}
uint32_t start_time,end_time,spend_time;
void Command_Process(void)
{
	uint16_t i = 0 ;
	uint32_t erase_address = 0 ;
	switch (commade_case) // 命令字
	{
		case KB2_BL_SYNC: // 签名----------------------------------------------------------------
			Vcp_Send_Message(SYNC);
			commade_case = 0;
			break;

		case GIF_TRANS_NOTICE:			     // 回应上传通知-----------------------------------------------------
			receive_frame_number = receive_data[5]<<8 | receive_data[6];
			need_to_erase_sector_number =	(need_to_erase_sector_number != 0 ? 0 : ((receive_frame_number * FRAME_DATA_SIZE) / FLASH_SECTOR_SIZE + 1));
		  need_to_erase_block_number = need_to_erase_sector_number / 16 + 1;
		
		  //need_to_erase_sector_number = need_to_erase_sector_number %  
			if(strncmp(gif1+2,(char *)receive_data,4) == 0)
			{
				erase_address = 0 ;
				for(i = 0; i < need_to_erase_block_number; i++)
				{
					flash_qspi_erase_block(erase_address); // 擦除
					erase_address += FLASH_SECTOR_SIZE * 16;
				}
				Updata_state_adderss = 0;
				Response_trans_notice();
			}
			else if(strncmp(gif2+2,(char *)receive_data,4) == 0)
			{
				erase_address = FRAME_DATA_SIZE * 40 ;
				for(i = 0; i < need_to_erase_block_number; i++)
				{
					flash_qspi_erase_sector(erase_address); // 擦除
					erase_address += FLASH_SECTOR_SIZE * 16;
				}
				Updata_state_adderss = FRAME_DATA_SIZE * 40;   //每个gif40帧
				Response_trans_notice();
			}
			else if(strncmp(gif3+2,(char *)receive_data,4) == 0)
			{
				erase_address = FRAME_DATA_SIZE * 80 ;
				for(i = 0; i < need_to_erase_block_number; i++)
				{
					flash_qspi_erase_sector(erase_address); // 擦除
					erase_address += FLASH_SECTOR_SIZE * 16;
					
				}
				Updata_state_adderss = FRAME_DATA_SIZE * 80;   //每个gif40帧
				Response_trans_notice();
			}			
			
			
			commade_case = 0;
			break;
		case GIF_TRANS_PACKAGE_TRANS_COMMAND:												//----------------------------------
		  // 更新下一个包存储的起始地址,第一包包含传输信息共长Nondata_Part_Len，减去对应长度信息
			flash_qspi_writebuffer((uint8_t *)receive_data + 4, Updata_state_adderss, Vcp_Rx_Data_length - Nondata_Part_Len);	

			Updata_state_adderss = Updata_state_adderss + Vcp_Rx_Data_length - Nondata_Part_Len;	//更新存储数据索引		

			crc_data_reset();
			crc_block_calculate((uint32_t*)receive_data + 4, (Vcp_Rx_Data_length - Nondata_Part_Len)/4);
		  
		  data_trans_len = data_trans_len - Vcp_Rx_Data_length + Nondata_Part_Len;
			commade_case = GIF_TRANS_PACKAGE_TRANS_DATA;
		
			break;

		
		case GIF_TRANS_PACKAGE_TRANS_DATA:
		
			flash_qspi_writebuffer((uint8_t *)Vcp_Rx_Data, Updata_state_adderss, Vcp_Rx_Data_length);	

			Updata_state_adderss += Vcp_Rx_Data_length;
			crc_result = crc_block_calculate((uint32_t*)Vcp_Rx_Data, Vcp_Rx_Data_length/4);
		  data_trans_len -= Vcp_Rx_Data_length;
		
			if(data_trans_len == 0)
			{
				Response_gif_trans_packet();
				crc_data_reset(); 
				commade_case = 0;
			}				
			
			break;
		
		default:
			commade_case = 0;
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
	
	SYNC[MCU_DATA_LEN_POZ1] = 0X00;
	SYNC[MCU_DATA_LEN_POZ2] = 0X00;
	SYNC[MCU_DATA_LEN_POZ3] = 0X00;
	SYNC[MCU_DATA_LEN_POZ4] = 0x01;
	
	SYNC[MCU_COMMAND_POZ] = GIF_TRANS_NOTICE;
	
	SYNC[MCU_CRC_POZ] = (uint8_t)(SYNC[MCU_DATA_LEN_POZ1]<<24 | SYNC[MCU_DATA_LEN_POZ2]<<16 | SYNC[MCU_DATA_LEN_POZ3]<<8 | SYNC[MCU_DATA_LEN_POZ4])
		+ SYNC[MCU_COMMAND_POZ] 
		+ SYNC[ MCU_CRC_POZ + (uint8_t)SYNC[(SYNC[MCU_DATA_LEN_POZ1]<<24 | SYNC[MCU_DATA_LEN_POZ2]<<16 | SYNC[MCU_DATA_LEN_POZ3]<<8 | SYNC[MCU_DATA_LEN_POZ4])] ];
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
	
	SYNC[MCU_DATA_LEN_POZ1] = 0X00;
	SYNC[MCU_DATA_LEN_POZ2] = 0X00;
	SYNC[MCU_DATA_LEN_POZ3] = 0X00;
	SYNC[MCU_DATA_LEN_POZ4] = 0x01;
	SYNC[MCU_COMMAND_POZ] = GIF_TRANS_PACKAGE_TRANS_COMMAND;

	
	SYNC[MCU_CRC_POZ] = (uint8_t)(SYNC[MCU_DATA_LEN_POZ1]<<24 | SYNC[MCU_DATA_LEN_POZ2]<<16 | SYNC[MCU_DATA_LEN_POZ3]<<8 | SYNC[MCU_DATA_LEN_POZ4])
		+ SYNC[MCU_COMMAND_POZ] 
		+ SYNC[ MCU_CRC_POZ + (uint8_t)SYNC[(SYNC[MCU_DATA_LEN_POZ1]<<24 | SYNC[MCU_DATA_LEN_POZ2]<<16 | SYNC[MCU_DATA_LEN_POZ3]<<8 | SYNC[MCU_DATA_LEN_POZ4])] ];
	SYNC[MCU_STATE_POZ] = 0x01;
	
	Vcp_Send_Message(SYNC);	
}

