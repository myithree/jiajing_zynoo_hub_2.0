/* add user code begin Header */
/**
  **************************************************************************
  * @file     main.c
  * @brief    main program
  **************************************************************************
  *                       Copyright notice & Disclaimer
  *
  * The software Board Support Package (BSP) that is made available to
  * download from Artery official website is the copyrighted work of Artery.
  * Artery authorizes customers to use, copy, and distribute the BSP
  * software and its related documentation for the purpose of design and
  * development in conjunction with Artery microcontrollers. Use of the
  * software is governed by this copyright notice and the following disclaimer.
  *
  * THIS SOFTWARE IS PROVIDED ON "AS IS" BASIS WITHOUT WARRANTIES,
  * GUARANTEES OR REPRESENTATIONS OF ANY KIND. ARTERY EXPRESSLY DISCLAIMS,
  * TO THE FULLEST EXTENT PERMITTED BY LAW, ALL EXPRESS, IMPLIED OR
  * STATUTORY OR OTHER WARRANTIES, GUARANTEES OR REPRESENTATIONS,
  * INCLUDING BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY,
  * FITNESS FOR A PARTICULAR PURPOSE, OR NON-INFRINGEMENT.
  *
  **************************************************************************
  */
/* add user code end Header */

/* Includes ------------------------------------------------------------------*/
#include "at32f402_405_wk_config.h"
#include "wk_crc.h"
#include "wk_qspi.h"
#include "wk_spi.h"
#include "wk_usb.h"
#include "wk_wdt.h"
#include "wk_gpio.h"
#include "usb_app.h"
#include "wk_system.h"

/* private includes ----------------------------------------------------------*/
/* add user code begin private includes */
#include "lcd_spi.h"
#include "get_current_time.h"
#include "ff_app.h"
#include "Agreement.h"
/* add user code end private includes */

/* private typedef -----------------------------------------------------------*/
/* add user code begin private typedef */
uint32_t start_time,spend_time;
/* add user code end private typedef */

/* private define ------------------------------------------------------------*/
/* add user code begin private define */

/* add user code end private define */

/* private macro -------------------------------------------------------------*/
/* add user code begin private macro */

/* add user code end private macro */

/* private variables ---------------------------------------------------------*/
/* add user code begin private variables */
uint32_t count = 0x13F;

/* add user code end private variables */

/* private function prototypes --------------------------------------------*/
/* add user code begin function prototypes */

/* add user code end function prototypes */

/* private user code ---------------------------------------------------------*/
/* add user code begin 0 */
void Command_Process(void);
/* add user code end 0 */

/**
  * @brief main function.
  * @param  none
  * @retval none
  */
int main(void)
{
  /* add user code begin 1 */

  /* add user code end 1 */

  /* system clock config. */
  wk_system_clock_config();

  /* config periph clock. */
  wk_periph_clock_config();

  /* nvic config. */
  wk_nvic_config();

  /* timebase config. */
  wk_timebase_init();

  /* init usb_otghs1 function. */
  wk_usb_otghs1_init();

  /* init spi1 function. */
  wk_spi1_init();

  /* init qspi1 function. */
  wk_qspi1_init();

  /* init crc function. */
  wk_crc_init();

  /* init wdt function. */
//  wk_wdt_init();

  /* init gpio function. */
  wk_gpio_config();

  /* init usb app function. */
  wk_usb_app_init();

  /* add user code begin 2 */
//	flash_qspi_erasechip();
	SPI_LCD_Init();
	Filesystem_init();
	
//	id_flash = flash_qspi_init();
//	QSPI_flash_mode();
//	qspi_flash_read_xip_dma_set(0, (uint8_t *)mydata, 300); // 写入
  /* add user code end 2 */

  while(1)
  {
     wk_usb_app_task();

    /* add user code begin 3 */
		
		
//		LCD_SetBackColor(0xff0000);           // 设置背景色

//		while(LCD_ReadScanLine() < 324 )
//			;
//			
//		LCD_Clear();                           // 清屏
//		wk_delay_ms(1);
//		
//		LCD_SetBackColor(0x0000ff);           // 设置背景色
//		
//		while(LCD_ReadScanLine() < 324 )
//			;
////		while(1)
////		{
////			ScanLine_positon[iii++] = LCD_ReadScanLine();
////			if(iii==200)iii = 0;
////		}
//		LCD_Clear();                           // 清屏
//		wk_delay_ms(1);
		Command_Process();
//		wdt_counter_reload();
    /* add user code end 3 */
  }
}

  /* add user code begin 4 */
void Command_Process(void)
{
		switch (commade_case) // 命令字
		{
			case KB2_BL_SYNC: // 签名----------------------------------------------------------------
				Vcp_Send_Message(SYNC);
				commade_case = 0;
				break;

			case GIF_TRANS_NOTICE:			     // 回应上传通知-----------------------------------------------------
				
				if(strncmp(gif1+2,(char *)receive_data,4) == 0)
				{
					res_csv = f_open(&fnew_csv, gif1, FA_WRITE);
				}
				else if(strncmp(gif2+2,(char *)receive_data,4) == 0)
				{
					res_csv = f_open(&fnew_csv, gif2, FA_WRITE);
				}
				else if(strncmp(gif3+2,(char *)receive_data,4) == 0)
				{
					res_csv = f_open(&fnew_csv, gif3, FA_WRITE);		
				}	
				if(res_csv == FR_OK)
				{
					Response_trans_notice();
					Updata_state_adderss = 0;
					crc_data_reset();
				}
				commade_case = 0;
				break;
			case GIF_TRANS_PACKAGE:												//----------------------------------
//				if(file_data_index == 8)file_data_index=0;
//				if(receive_data)memcpy(file_data_buffer + file_data_index*512, receive_data, 512);
//				file_data_index++;
				
				if(res_csv == FR_OK)
				{
					res_csv = f_write(&fnew_csv, receive_data, (UINT)4096, fnum_csv);// 将包存入flash
				}
				if(res_csv == FR_OK)
				{
					Updata_state_adderss += Vcp_Rx_Data_length;										// 更新下一个包存储的起始地址
					res_csv = f_lseek(
						&fnew_csv,
						Updata_state_adderss
					);
					crc_result = crc_block_calculate((uint32_t*)receive_data, Vcp_Rx_Data_length/4);
				}
				if(res_csv == FR_OK)
				{
					Response_gif_trans_packet();
				}
				
				commade_case = 0;
				break;
			case GIF_TRANS_DONE: //-----------------------------------------------------------
				res_csv = f_close(&fnew_csv);

				if(res_csv == FR_OK)
				{			 
					Response_gif_trans_done();
					crc_data_reset();   
				}
				commade_case = 0;
				break;

			default:
				commade_case = 0;
				break;
		}
}

//uint8_t file_data_buffer[4096];
//uint8_t file_data_index = 0;
//uint32_t Vcp_Rx_Data_length_last_all = 0;

//if(file_data_index < 8)
//{
//	memcpy(file_data_buffer + Vcp_Rx_Data_length_last_all, receive_data, Vcp_Rx_Data_length);
//	Vcp_Rx_Data_length_last_all += Vcp_Rx_Data_length;
//	file_data_index++;
//}
//if(res_csv == FR_OK && file_data_index == 8)
//{
//	res_csv = f_write(&fnew_csv, receive_data, (UINT)Vcp_Rx_Data_length, fnum_csv);// 将包存入flash
//	if(res_csv == FR_OK)
//	{
//		Updata_state_adderss += Vcp_Rx_Data_length;										// 更新下一个包存储的起始地址
//		res_csv = f_lseek(
//			&fnew_csv,
//			Updata_state_adderss
//		);
//		crc_result = crc_block_calculate((uint32_t*)receive_data, Vcp_Rx_Data_length_last_all/4);
//		if(res_csv == FR_OK)
//		{
//			Response_gif_trans_packet();
//		}
//	}
//}

//if(file_data_index == 8)
//{
//	file_data_index = 0;
//	Vcp_Rx_Data_length_last_all = 0;
//}

  /* add user code end 4 */
