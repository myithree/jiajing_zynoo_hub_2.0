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
uint16_t ScanLine_positon[200] = {0},iii=0;
/* add user code end private variables */

/* private function prototypes --------------------------------------------*/
/* add user code begin function prototypes */

/* add user code end function prototypes */

/* private user code ---------------------------------------------------------*/
/* add user code begin 0 */

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

  /* init wdt function. */
//  wk_wdt_init();

  /* init gpio function. */
  wk_gpio_config();

  /* init usb app function. */
  wk_usb_app_init();

  /* add user code begin 2 */
	SPI_LCD_Init();
  /* add user code end 2 */
//		while(1)
//		{
//			ScanLine_positon[iii++] = LCD_ReadScanLine();
//			if(iii==200)iii = 0;
//		}
  while(1)
  {
		wk_usb_app_task();

    /* add user code begin 3 */

		
		LCD_SetBackColor(0xff0000);           // 设置背景色

		while(LCD_ReadScanLine() < 324 )
			;
			
		LCD_Clear();                           // 清屏
		wk_delay_ms(1);
		
		LCD_SetBackColor(0x0000ff);           // 设置背景色
		
		while(LCD_ReadScanLine() < 324 )
			;
//		while(1)
//		{
//			ScanLine_positon[iii++] = LCD_ReadScanLine();
//			if(iii==200)iii = 0;
//		}
		LCD_Clear();                           // 清屏
		wk_delay_ms(1);
	
//		wdt_counter_reload();
    /* add user code end 3 */
  }
}

  /* add user code begin 4 */

  /* add user code end 4 */
