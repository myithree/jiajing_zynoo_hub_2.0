/***
>>>>> 重要说明：
	*
	*  1.屏幕配置为16位RGB565格式
	*  2.SPI通信速度为 21M  
   *
>>>>> 其他说明：
	*
	*	1. 中文字库使用的是小字库，即用到了对应的汉字再去取模，用户可以根据需求自行增添或删减
	*	2. 各个函数的功能和使用可以参考函数的说明
	*
	*********************************************************************************************************************************************************************************************FANKE*****
***/

#include "lcd_spi.h"

//SPI_HandleTypeDef hspi3;			// SPI_HandleTypeDef 结构体变量

//#define  LCD_SPI hspi2           // SPI局部宏，方便修改和移植

//static pFONT *LCD_AsciiFonts;		// 英文字体，ASCII字符集
//static pFONT *LCD_CHFonts;		   // 中文字体（同时也包含英文字体）

// 因为这类SPI的屏幕，每次更新显示时，需要先配置坐标区域、再写显存，
// 在显示字符时，如果是一个个点去写坐标写显存，会非常慢，
// 因此开辟一片缓冲区，先将需要显示的数据写进缓冲区，最后再批量写入显存。
// 用户可以根据实际情况去修改此处缓冲区的大小，
// 例如，用户需要显示32*32的汉字时，需要的大小为 32*32*2 = 2048 字节（每个像素点占2字节）

uint16_t  LCD_Buff[2];        // LCD缓冲区，16位宽（每个像素点占2字节）


struct	//LCD相关参数结构体
{
	 uint32_t Color;  				//	LCD当前画笔颜色
	 uint32_t BackColor;			//	背景色
   uint8_t  ShowNum_Mode;		// 数字显示模式
   uint8_t  Direction;			//	显示方向
   uint16_t Width;            // 屏幕像素长度
   uint16_t Height;           // 屏幕像素宽度	
   uint8_t  X_Offset;         // X坐标偏移，用于设置屏幕控制器的显存写入方式
   uint8_t  Y_Offset;         // Y坐标偏移，用于设置屏幕控制器的显存写入方式
}LCD;


/**
  * @brief  spi mode switch between lcd and touch
  * @param  mode 
  *         1:touch, 0:lcd
  * @retval none
  */
void at_spi_switch_to_16bit(void)
{
   spi_enable(SPI1, FALSE);                           // 关闭SPI
   SPI1->ctrl1_bit.fbn = SPI_FRAME_16BIT;	            // 切换成16位数据格式
   spi_enable(SPI1, TRUE);				                  	// 使能SPI
}

void at_spi_switch_to_8bit(void)
{
   spi_enable(SPI1, FALSE);                           // 关闭SPI
   SPI1->ctrl1_bit.fbn = SPI_FRAME_8BIT;	            // 切换成8位数据格式
   spi_enable(SPI1, TRUE);				                   	// 使能SPI
}

/**
  * @brief  lcd SPI1 write
  * @param  data: write data
  * @retval read data
  */
void at_spi_transmit(spi_type* spi_x, uint8_t data)
{
	
	while( (spi_x->sts & SPI_I2S_TDBE_FLAG) == 0)
		;
	spi_i2s_data_transmit(spi_x, data);	
}

void at_spi_transmit_16bit(spi_type* spi_x, uint16_t data)
{
  
	while( (spi_x->sts & SPI_I2S_TDBE_FLAG) == 0)
		;
	spi_i2s_data_transmit(spi_x, data);	
}

/*****************************************************************************************
*	函 数 名: LCD_WriteCMD
*	入口参数: CMD - 需要写入的控制指令
*	返 回 值: 无
*	函数功能: 用于写入控制字
*	说    明: 无
******************************************************************************************/
#define wait_number 150
void  LCD_WriteCommand(uint8_t lcd_command)
{
	uint16_t i = 0;
	for(i = 0; i < wait_number; i++)
		__NOP();
	LCD_DC_Command;	//	DC引脚输出低，代表写指令	
	for(i = 0; i < wait_number; i++)
		__NOP();
	at_spi_transmit(SPI1, lcd_command);
	for(i = 0; i < wait_number; i++)
		__NOP(); 
	LCD_DC_Data;	//	DC引脚输出高，代表写数据	
	for(i = 0; i < wait_number; i++)
		__NOP(); 	
}

/****************************************************************************************************************************************
*	函 数 名: LCD_WriteData_8bit
*
*	入口参数: lcd_data - 需要写入的数据，8位
*
*	函数功能: 写入8位数据
*	
****************************************************************************************************************************************/

void  LCD_WriteData_8bit(uint8_t lcd_data)
{
	at_spi_transmit(SPI1, lcd_data);
}

/****************************************************************************************************************************************
*	函 数 名: LCD_WriteData_16bit
*
*	入口参数: lcd_data - 需要写入的数据，16位
*
*	函数功能: 写入16位数据
*	
****************************************************************************************************************************************/

void  LCD_WriteData_16bit(uint16_t lcd_data)
{
	at_spi_transmit(SPI1, lcd_data>>8);
	at_spi_transmit(SPI1, lcd_data);
}

/****************************************************************************************************************************************
*	函 数 名: LCD_WriteBuff
*
*	入口参数: DataBuff - 数据区，DataSize - 数据长度
*
*	函数功能: 批量写入数据到屏幕
*	
****************************************************************************************************************************************/

void  LCD_WriteBuff(uint16_t *DataBuff, uint16_t DataSize)
{
	uint32_t i;
	
  at_spi_switch_to_16bit();           // 切换成16位数据格式
	
	 	// 片选拉低，使能IC
	
	for(i=0;i<DataSize;i++)				
	{
		at_spi_transmit_16bit(SPI1, DataBuff[i]);
		//    spi_i2s_data_transmit(SPI1, DataBuff[i]);
	}
		//  while (spi_i2s_flag_get(SPI1, SPI_I2S_BF_FLAG) == SET)
	 	// 片选拉高	
	
  at_spi_switch_to_8bit();           // 切换成8位数据格式
}

/****************************************************************************************************************************************
*	函 数 名: SPI_LCD_Init
*
*	函数功能: 初始化SPI以及屏幕控制器的各种参数
*	
****************************************************************************************************************************************/

void SPI_LCD_Init(void)
{
	LCD_Backlight_ON;
	RST_1;
  wk_delay_ms(10);               // 屏幕刚完成复位时（包括上电复位），需要等待5ms才能发送指令
	RST_0;
	wk_delay_ms(100); 
	RST_1;
	wk_delay_ms(120); 
	
//	 	// 片选拉低，使能IC，开始通信

	LCD_WriteCommand(0xFD);
	LCD_WriteData_8bit(0x06);
	LCD_WriteData_8bit(0x08);
	LCD_WriteCommand(0x61);
	LCD_WriteData_8bit(0x07);
	LCD_WriteData_8bit(0x07);
	LCD_WriteCommand(0x73);
	LCD_WriteData_8bit(0x70);
	LCD_WriteCommand(0x73);
	LCD_WriteData_8bit(0x00); //07
	//bias
	LCD_WriteCommand(0x62);//
	LCD_WriteData_8bit(0x00);
	LCD_WriteData_8bit(0x44);
	LCD_WriteData_8bit(0x40);
	LCD_WriteCommand(0x63);//VGL
	LCD_WriteData_8bit(0x41);//
	LCD_WriteData_8bit(0x07);//
	LCD_WriteData_8bit(0x12);//
	LCD_WriteData_8bit(0x12);//
	//VSP
	LCD_WriteCommand(0x65);//Pump1=4.7MHz //PUMP1 VSP
	LCD_WriteData_8bit(0x08);//D6-5:pump1_clk[1:0] clamp 28 2b
	LCD_WriteData_8bit(0x17);//6.26
	LCD_WriteData_8bit(0x21);
	//VSN
	LCD_WriteCommand(0x66); //pump=2 AVCL
	LCD_WriteData_8bit(0x08); //clamp 08 0b 09
	LCD_WriteData_8bit(0x17); //10
	LCD_WriteData_8bit(0x21);
	//add source_neg_time
	LCD_WriteCommand(0x67);//pump_sel
	LCD_WriteData_8bit(0x20);//21 20
	LCD_WriteData_8bit(0x40);
	//gamma vap/van
	LCD_WriteCommand(0x68);//gamma vap/van
	LCD_WriteData_8bit(0x90);//90 9f
	LCD_WriteData_8bit(0x30);// 30
	LCD_WriteData_8bit(0x27);//77
	LCD_WriteData_8bit(0x2c);//07
	LCD_WriteCommand(0xb1);//frame rate
	LCD_WriteData_8bit(0x0F);//0x0f fr_h[5:0] 0F
	LCD_WriteData_8bit(0x02);//0x02 fr_v[4:0] 02
	LCD_WriteData_8bit(0x01);//0x04 fr_div[2:0] 03
	LCD_WriteCommand(0xB4);
	LCD_WriteData_8bit(0x01); //00:column 01:dot
	////porch
	LCD_WriteCommand(0xB5);
	LCD_WriteData_8bit(0x02);//0x02 vfp[6:0]
	LCD_WriteData_8bit(0x02);//0x02 vbp[6:0]
	LCD_WriteData_8bit(0x0a);//0x0A hfp[6:0]
	LCD_WriteData_8bit(0x14);//0x14 hbp[6:0]
	LCD_WriteCommand(0xB6);
	LCD_WriteData_8bit(0x04);//
	LCD_WriteData_8bit(0x01);//
	LCD_WriteData_8bit(0x9f);//
	LCD_WriteData_8bit(0x00);//
	LCD_WriteData_8bit(0x02);//
	////gamme sel
	LCD_WriteCommand(0xdf);//
	LCD_WriteData_8bit(0x11);//gofc_gamma_en_sel=1
	////gamma_test1 A1#_wangly
	//3030b_gamma_new_12.18
	//GAMMA---------------------------------/////////////
	LCD_WriteCommand(0xE2);
	LCD_WriteData_8bit(0x23);//vrp0[5:0] V63
	LCD_WriteData_8bit(0x22);//vrp1[5:0] V62
	LCD_WriteData_8bit(0x23);//vrp2[5:0] V61
	LCD_WriteData_8bit(0x35);//vrp3[5:0] V2
	LCD_WriteData_8bit(0x38);//vrp4[5:0] V1
	LCD_WriteData_8bit(0x3f);//vrp5[5:0] V0
	LCD_WriteCommand(0xE5);
	LCD_WriteData_8bit(0x3f);//vrn0[5:0] V0
	LCD_WriteData_8bit(0x30);//vrn1[5:0] V1
	LCD_WriteData_8bit(0x2d);//vrn2[5:0] V2
	LCD_WriteData_8bit(0x1d);//vrn3[5:0] V61
	LCD_WriteData_8bit(0x20);//vrn4[5:0] V62
	LCD_WriteData_8bit(0x22);//vrn5[5:0] V63
	LCD_WriteCommand(0xE1);
	LCD_WriteData_8bit(0x30);//prp0[6:0] V51
	LCD_WriteData_8bit(0x78);//prp1[6:0] V15
	LCD_WriteCommand(0xE4);
	LCD_WriteData_8bit(0x69);//prn0[6:0] V15
	LCD_WriteData_8bit(0x2f);//prn1[6:0] V51
	LCD_WriteCommand(0xE0);
	LCD_WriteData_8bit(0x05);//pkp0[4:0] V60
	LCD_WriteData_8bit(0x05);//pkp1[4:0] V56
	LCD_WriteData_8bit(0x10);//pkp2[4:0] V45
	LCD_WriteData_8bit(0x12);//pkp3[4:0] V37
	LCD_WriteData_8bit(0x11);//pkp4[4:0] V29
	LCD_WriteData_8bit(0x0d);//pkp5[4:0] V21
	LCD_WriteData_8bit(0x0e);//pkp6[4:0] V7
	LCD_WriteData_8bit(0x17);//pkp7[4:0] V3
	LCD_WriteCommand(0xE3);
	LCD_WriteData_8bit(0x13);//pkn0[4:0] V3
	LCD_WriteData_8bit(0x13);//pkn1[4:0] V7
	LCD_WriteData_8bit(0x13);//pkn2[4:0] V21
	LCD_WriteData_8bit(0x0c);//pkn3[4:0] V29
	LCD_WriteData_8bit(0x0f);//pkn4[4:0] V37
	LCD_WriteData_8bit(0x07);//pkn5[4:0] V45
	LCD_WriteData_8bit(0x05);//pkn6[4:0] V56
	LCD_WriteData_8bit(0x05);//pkn7[4:0] V60
	//GAMMA---------------------------------/////////////
	//source
	LCD_WriteCommand(0xE6);
	LCD_WriteData_8bit(0x00);
	LCD_WriteData_8bit(0xff);//SC_EN_START[7:0] f0
	LCD_WriteCommand(0xE7);
	LCD_WriteData_8bit(0x01);//CS_START[3:0] 01
	LCD_WriteData_8bit(0x04);//scdt_inv_sel cs_vp_en
	LCD_WriteData_8bit(0x03);//CS1_WIDTH[7:0]12
	LCD_WriteData_8bit(0x03);//CS2_WIDTH[7:0]12
	LCD_WriteData_8bit(0x00);//PREC_START[7:0] 06
	LCD_WriteData_8bit(0x12);//PREC_WIDTH[7:0] 12
	LCD_WriteCommand(0xE8); //source
	LCD_WriteData_8bit(0x00); //VCMP_OUT_EN 81-vcmp/vref_output pad
	LCD_WriteData_8bit(0x70); //chopper_sel[6:4]
	LCD_WriteData_8bit(0x00); //gchopper_sel[6:4] 60
	////gate
	LCD_WriteCommand(0xEc);
	LCD_WriteData_8bit(0x52);//50
	LCD_WriteCommand(0xF1);
	LCD_WriteData_8bit(0x01);//te_pol tem_extend
	LCD_WriteData_8bit(0x01);
	LCD_WriteData_8bit(0x02);
	LCD_WriteCommand(0xF6);
	LCD_WriteData_8bit(0x01);
	LCD_WriteData_8bit(0x30);
	LCD_WriteData_8bit(0x00);
	LCD_WriteData_8bit(0x00);//SPI2L: 40
	LCD_WriteCommand(0xfd);
	LCD_WriteData_8bit(0xfa);
	LCD_WriteData_8bit(0xfc);
	LCD_WriteCommand(0x3a);
	LCD_WriteData_8bit(0x55);//SH 0x66  0x55 16bit/pixel
	LCD_WriteCommand(0x35);
	LCD_WriteData_8bit(0x00);
	LCD_WriteCommand(0x36);//bgr_[3]
	LCD_WriteData_8bit(0x00);//c0
	LCD_WriteCommand(0x21);
	LCD_WriteCommand(0x11); // exit sleep
	wk_delay_ms(120);
	LCD_WriteCommand(0x29); // display on
	wk_delay_ms(10);

//	 	// 片选拉高		

// 以下进行一些驱动的默认设置
   LCD_SetDirection(Direction_V);  	      //	设置显示方向
   LCD_SetColor(LCD_CYAN);               // 设置画笔色  
	 LCD_SetBackColor(LIGHT_BLUE);           // 设置背景色
	 LCD_Clear();                           // 清屏

 

// 全部设置完毕之后，打开背光	
//   LCD_Backlight_ON;  // 引脚输出高电平点亮背光
	
}

/****************************************************************************************************************************************
*	函 数 名:	 LCD_SetAddress
*
*	入口参数:	 x1 - 起始水平坐标   y1 - 起始垂直坐标  
*              x2 - 终点水平坐标   y2 - 终点垂直坐标	   
*	
*	函数功能:   设置需要显示的坐标区域		 			 
*****************************************************************************************************************************************/

void LCD_SetAddress(uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y2)		
{
	 	// 片选拉低，使能IC
	at_spi_switch_to_8bit();
	LCD_WriteCommand(0x2a);			//	列地址设置，即X坐标
	LCD_WriteData_16bit(x1+LCD.X_Offset);
	LCD_WriteData_16bit(x2+LCD.X_Offset);

	LCD_WriteCommand(0x2b);			//	行地址设置，即Y坐标
	LCD_WriteData_16bit(y1+LCD.Y_Offset);
	LCD_WriteData_16bit(y2+LCD.Y_Offset);

	LCD_WriteCommand(0x2c);			//	开始写入显存，即要显示的颜色数据
	
//	while( (LCD_SPI.Instance->SR & 0x0080) != RESET);	//	等待通信完成
	 	// 片选拉高		
}

/****************************************************************************************************************************************
*	函 数 名:	LCD_SetColor
*
*	入口参数:	Color - 要显示的颜色，示例：0x0000FF 表示蓝色
*
*	函数功能:	此函数用于设置画笔的颜色，例如显示字符、画点画线、绘图的颜色
*
*	说    明:	1. 为了方便用户使用自定义颜色，入口参数 Color 使用24位 RGB888的颜色格式，用户无需关心颜色格式的转换
*					2. 24位的颜色中，从高位到低位分别对应 R、G、B  3个颜色通道
*
*****************************************************************************************************************************************/

void LCD_SetColor(uint32_t Color)
{
	uint16_t Red_Value = 0, Green_Value = 0, Blue_Value = 0; //各个颜色通道的值

	Red_Value   = (uint16_t)((Color&0x00F80000)>>8);   // 转换成 16位 的RGB565颜色
	Green_Value = (uint16_t)((Color&0x0000FC00)>>5);
	Blue_Value  = (uint16_t)((Color&0x000000F8)>>3);

	LCD.Color = (uint16_t)(Red_Value | Green_Value | Blue_Value);  // 将颜色写入全局LCD参数		
}

/****************************************************************************************************************************************
*	函 数 名:	LCD_SetBackColor
*
*	入口参数:	Color - 要显示的颜色，示例：0x0000FF 表示蓝色
*
*	函数功能:	设置背景色,此函数用于清屏以及显示字符的背景色
*
*	说    明:	1. 为了方便用户使用自定义颜色，入口参数 Color 使用24位 RGB888的颜色格式，用户无需关心颜色格式的转换
*					2. 24位的颜色中，从高位到低位分别对应 R、G、B  3个颜色通道
*
*****************************************************************************************************************************************/

void LCD_SetBackColor(uint32_t Color)
{
	uint16_t Red_Value = 0, Green_Value = 0, Blue_Value = 0; //各个颜色通道的值

	Red_Value   = (uint16_t)((Color&0x00F80000)>>8);   // 转换成 16位 的RGB565颜色
	Green_Value = (uint16_t)((Color&0x0000FC00)>>5);
	Blue_Value  = (uint16_t)((Color&0x000000F8)>>3);

	LCD.BackColor = (uint16_t)(Red_Value | Green_Value | Blue_Value);	// 将颜色写入全局LCD参数		
	//	LCD.BackColor = (uint16_t)Color;	
}

/****************************************************************************************************************************************
*	函 数 名:	LCD_SetDirection
*
*	入口参数:	direction - 要显示的方向
*
*	函数功能:	设置要显示的方向
*
*	说    明:   1. 可输入参数 Direction_H 、Direction_V 、Direction_H_Flip 、Direction_V_Flip        
*              2. 使用示例 LCD_DisplayDirection(Direction_H) ，即设置屏幕横屏显示
*
*****************************************************************************************************************************************/

void LCD_SetDirection(uint8_t direction)
{
	LCD.Direction = direction;    // 写入全局LCD参数

	 	// 片选拉低，使能IC
			
   if( direction == Direction_H )   // 横屏显示
   {
      LCD_WriteCommand(0x36);    		// 显存访问控制 指令，用于设置访问显存的方式
      LCD_WriteData_8bit(0x70);        // 横屏显示
      LCD.X_Offset   = 0;             // 设置控制器坐标偏移量
      LCD.Y_Offset   = 35;   
      LCD.Width      = LCD_Height;		// 重新赋值长、宽
      LCD.Height     = LCD_Width;		
   }
   else if( direction == Direction_V )
   {
      LCD_WriteCommand(0x36);    		// 显存访问控制 指令，用于设置访问显存的方式
      LCD_WriteData_8bit(0x00);        // 垂直显示
      LCD.X_Offset   = 35;             // 设置控制器坐标偏移量
      LCD.Y_Offset   = 0;     
      LCD.Width      = LCD_Width;		// 重新赋值长、宽
      LCD.Height     = LCD_Height;						
   }
   else if( direction == Direction_H_Flip )
   {
      LCD_WriteCommand(0x36);   			 // 显存访问控制 指令，用于设置访问显存的方式
      LCD_WriteData_8bit(0xA0);         // 横屏显示，并上下翻转，RGB像素格式
      LCD.X_Offset   = 0;              // 设置控制器坐标偏移量
      LCD.Y_Offset   = 35;      
      LCD.Width      = LCD_Height;		 // 重新赋值长、宽
      LCD.Height     = LCD_Width;				
   }
   else if( direction == Direction_V_Flip )
   {
      LCD_WriteCommand(0x36);    		// 显存访问控制 指令，用于设置访问显存的方式
      LCD_WriteData_8bit(0xC0);        // 垂直显示 ，并上下翻转，RGB像素格式
      LCD.X_Offset   = 0;             // 设置控制器坐标偏移量
      LCD.Y_Offset   = 0;     
      LCD.Width      = LCD_Width;		// 重新赋值长、宽
      LCD.Height     = LCD_Height;				
   }     
	
	   
//	while( (LCD_SPI.Instance->SR & 0x0080) != RESET);	//	等待通信完成
	 	// 片选拉高			
}


uint16_t LCD_ReadScanLine(void)
{
		LCD_WriteCommand(0x45);
		while( (SPI1->sts & SPI_I2S_TDBE_FLAG) == 0)
			;// 等待SPI空闲
		
		
    SPI1->ctrl1_bit.spien = FALSE;//关闭spi
		
		SPI1->ctrl2_bit.mdiv3en = FALSE;
    SPI1->ctrl2_bit.mdiv_h = FALSE;
    SPI1->ctrl1_bit.mdiv_l = SPI_MCLK_DIV_16;//spi时钟分频改为16
		
		SPI1->ctrl1_bit.spien = TRUE; 
 

		GPIOA->cfgr  &= (uint32_t)~(0x03 << (4 * 2));//把GPIOC的PINS_4位置置为零
	  GPIOA->cfgr  |= (uint32_t)(GPIO_MODE_OUTPUT << (4 * 2));//将GPIOA的PINS_4设为GPIO_MODE_OUTPUT模式
		
		GPIOA->clr = GPIO_PINS_4;//将GPIOC的PINS_4设为低电平

		
		
    SPI1->ctrl1_bit.spien = FALSE;//关闭spi
		
    SPI1->ctrl1_bit.slben = TRUE;
    SPI1->ctrl1_bit.slbtd = FALSE;
    SPI1->ctrl1_bit.ora = FALSE;    //spi模式改为单线主机只收模式
		
    SPI1->ctrl1_bit.spien = TRUE; 

    uint8_t data[3] = {0};
    int i = 0;
    while(i < 2)
    {
        if (SPI1->sts & SPI_I2S_RDBF_FLAG)
            data[i++] = (uint8_t)(SPI1->dt);
    }
    
    __DSB();

    while ((SPI1->sts & SPI_I2S_RDBF_FLAG) != SPI_I2S_RDBF_FLAG);
    /* read the received data */
    data[2] = (uint8_t)(SPI1->dt);
    while ((SPI1->sts & SPI_I2S_TDBE_FLAG) == 0);

		
    GPIOA->scr = GPIO_PINS_4;//将GPIOC的PINS_4设为高电平
		
		GPIOA->cfgr  &= (uint32_t)~(0x03 << (4 * 2));//把GPIOC的PINS_4位置置为零
	  GPIOA->cfgr  |= (uint32_t)(GPIO_MODE_MUX << (4 * 2));//将GPIOA的PINS_4设为GPIO_MODE_MUX模式
		
		SPI1->ctrl1_bit.spien = FALSE;//关闭spi
		
		SPI1->ctrl1_bit.slben = TRUE;
    SPI1->ctrl1_bit.slbtd = TRUE;
    SPI1->ctrl1_bit.ora = FALSE;    //spi模式改为单线主机只发模式
		
		SPI1->ctrl2_bit.mdiv3en = TRUE;
    SPI1->ctrl2_bit.mdiv_h = FALSE;
    SPI1->ctrl1_bit.mdiv_l = 0;    //spi时钟分频改为3
		
    SPI1->ctrl1_bit.spien = TRUE;

    return data[1] << 1 | !!data[2];
}


/****************************************************************************************************************************************
*	函 数 名:	LCD_Clear
*
*	函数功能:	清屏函数，将LCD清除为 LCD.BackColor 的颜色
*
*	说    明:	先用 LCD_SetBackColor() 设置要清除的背景色，再调用该函数清屏即可
*
*****************************************************************************************************************************************/

void LCD_Clear(void)
{
	uint32_t i;
	
	LCD_SetAddress(0,0,LCD.Width-1,LCD.Height-1);			//设置坐标	
	at_spi_switch_to_16bit();           // 切换成16位数据格式
	 	// 片选拉低，使能IC
	
	for(i=0;i<LCD.Width*LCD.Height;i++)				
	{
		at_spi_transmit_16bit(SPI1, LCD.BackColor);
//    spi_i2s_data_transmit(SPI1, LCD.BackColor);
	}
//	while (spi_i2s_flag_get(SPI1, SPI_I2S_BF_FLAG) == SET)
//	while( (LCD_SPI.Instance->SR & 0x0080) != RESET);	//	等待通信完成
	  	// 片选拉高		
  at_spi_switch_to_8bit();           // 切换成8位数据格式
}


/****************************************************************************************************************************************
*	函 数 名:	LCD_ClearRect
*
*	入口参数:	x - 起始水平坐标
*					y - 起始垂直坐标
*					width  - 要清除区域的横向长度
*					height - 要清除区域的纵向宽度
*
*	函数功能:	局部清屏函数，将指定位置对应的区域清除为 LCD.BackColor 的颜色
*
*	说    明:	1. 先用 LCD_SetBackColor() 设置要清除的背景色，再调用该函数清屏即可
*				   2. 使用示例 LCD_ClearRect( 10, 10, 100, 50) ，清除坐标(10,10)开始的长100宽50的区域
*
*****************************************************************************************************************************************/

void LCD_ClearRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height)
{
	uint16_t i;

  LCD_SetAddress( x, y, x+width-1, y+height-1);	// 设置坐标	

  at_spi_switch_to_16bit();           // 切换成16位数据格式
	
	 	// 片选拉低，使能IC
	
	for(i=0;i<width*height;i++)				
	{
		at_spi_transmit_16bit(SPI1, LCD.BackColor);
//    spi_i2s_data_transmit(SPI1, LCD.BackColor);
	}
//	while( (LCD_SPI.Instance->SR & 0x0080) != RESET);	//	等待通信完成
	 	// 片选拉高	
	
  at_spi_switch_to_8bit();           // 切换成8位数据格式
}


/****************************************************************************************************************************************
*	函 数 名:	LCD_DrawPoint
*
*	入口参数:	x - 起始水平坐标
*					y - 起始垂直坐标
*					color  - 要绘制的颜色，使用 24位 RGB888 的颜色格式，用户无需关心颜色格式的转换
*
*	函数功能:	在指定坐标绘制指定颜色的点
*
*	说    明:	使用示例 LCD_DrawPoint( 10, 10, 0x0000FF) ，在坐标(10,10)绘制蓝色的点
*
*****************************************************************************************************************************************/

void LCD_DrawPoint(uint16_t x,uint16_t y,uint32_t color)
{
	LCD_SetAddress(x,y,x,y);	//	设置坐标 
//	
	 	// 片选拉低，使能IC

	LCD_WriteData_16bit(color);//LCD.Color
	
//	while( (LCD_SPI.Instance->SR & 0x0080) != RESET);	//	等待通信完成
	 	// 片选拉高		
} 




/*****************************************************************************************************************************************
*	函 数 名:	LCD_ShowNumMode
*
*	入口参数:	mode - 设置变量的显示模式
*
*	函数功能:	设置变量显示时多余位补0还是补空格，可输入参数 Fill_Space 填充空格，Fill_Zero 填充零
*
*	说    明:   1. 只有 LCD_DisplayNumber() 显示整数 和 LCD_DisplayDecimals()显示小数 这两个函数用到
*					2. 使用示例 LCD_ShowNumMode(Fill_Zero) 设置多余位填充0，例如 123 可以显示为 000123
*
*****************************************************************************************************************************************/

void LCD_ShowNumMode(uint8_t mode)
{
	LCD.ShowNum_Mode = mode;
}






/***************************************************************************************************************************************
*	函 数 名: LCD_DrawLine
*
*	入口参数: x1 - 起点 水平坐标
*			 	 y1 - 起点 垂直坐标
*
*				 x2 - 终点 水平坐标
*            y2 - 终点 垂直坐标
*
*	函数功能: 在两点之间画线
*
*	说    明: 该函数移植于ST官方评估板的例程
*						 
*****************************************************************************************************************************************/

#define ABS(X)  ((X) > 0 ? (X) : -(X))    

void LCD_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
	int16_t deltax = 0, deltay = 0, x = 0, y = 0, xinc1 = 0, xinc2 = 0, 
	yinc1 = 0, yinc2 = 0, den = 0, num = 0, numadd = 0, numpixels = 0, 
	curpixel = 0;

	deltax = ABS(x2 - x1);        /* The difference between the x's */
	deltay = ABS(y2 - y1);        /* The difference between the y's */
	x = x1;                       /* Start x off at the first pixel */
	y = y1;                       /* Start y off at the first pixel */

	if (x2 >= x1)                 /* The x-values are increasing */
	{
	 xinc1 = 1;
	 xinc2 = 1;
	}
	else                          /* The x-values are decreasing */
	{
	 xinc1 = -1;
	 xinc2 = -1;
	}

	if (y2 >= y1)                 /* The y-values are increasing */
	{
	 yinc1 = 1;
	 yinc2 = 1;
	}
	else                          /* The y-values are decreasing */
	{
	 yinc1 = -1;
	 yinc2 = -1;
	}

	if (deltax >= deltay)         /* There is at least one x-value for every y-value */
	{
	 xinc1 = 0;                  /* Don't change the x when numerator >= denominator */
	 yinc2 = 0;                  /* Don't change the y for every iteration */
	 den = deltax;
	 num = deltax / 2;
	 numadd = deltay;
	 numpixels = deltax;         /* There are more x-values than y-values */
	}
	else                          /* There is at least one y-value for every x-value */
	{
	 xinc2 = 0;                  /* Don't change the x for every iteration */
	 yinc1 = 0;                  /* Don't change the y when numerator >= denominator */
	 den = deltay;
	 num = deltay / 2;
	 numadd = deltax;
	 numpixels = deltay;         /* There are more y-values than x-values */
	}
	for (curpixel = 0; curpixel <= numpixels; curpixel++)
	{
	 LCD_DrawPoint(x,y,LCD.Color);             /* Draw the current pixel */
	 num += numadd;              /* Increase the numerator by the top of the fraction */
	 if (num >= den)             /* Check if numerator >= denominator */
	 {
		num -= den;               /* Calculate the new numerator value */
		x += xinc1;               /* Change the x as appropriate */
		y += yinc1;               /* Change the y as appropriate */
	 }
	 x += xinc2;                 /* Change the x as appropriate */
	 y += yinc2;                 /* Change the y as appropriate */
	}  
}


/***************************************************************************************************************************************
*	函 数 名: LCD_DrawLine_V
*
*	入口参数: x - 水平坐标
*			 	 y - 垂直坐标
*				 height - 垂直宽度
*
*	函数功能: 在指点位置绘制指定长宽的 垂直 线
*
*	说    明: 1. 该函数移植于ST官方评估板的例程
*				 2. 要绘制的区域不能超过屏幕的显示区域		
*            3. 如果只是画垂直的线，优先使用此函数，速度比 LCD_DrawLine 快很多
*  性能测试：
*****************************************************************************************************************************************/

void LCD_DrawLine_V(uint16_t x, uint16_t y, uint16_t height)
{
   uint16_t i ; // 计数变量

	for (i = 0; i < height; i++)
	{
       LCD_Buff[i] =  LCD.Color;  // 写入缓冲区
   }   
   LCD_SetAddress( x, y, x, y+height-1);	     // 设置坐标	

   LCD_WriteBuff(LCD_Buff,height);          // 写入显存
}

/***************************************************************************************************************************************
*	函 数 名: LCD_DrawLine_H
*
*	入口参数: x - 水平坐标
*			 	 y - 垂直坐标
*				 width  - 水平宽度
*
*	函数功能: 在指点位置绘制指定长宽的 水平 线
*
*	说    明: 1. 该函数移植于ST官方评估板的例程
*				 2. 要绘制的区域不能超过屏幕的显示区域		
*            3. 如果只是画 水平 的线，优先使用此函数，速度比 LCD_DrawLine 快很多
*  性能测试：
**********************************************************************************************************************************fanke*******/

void LCD_DrawLine_H(uint16_t x, uint16_t y, uint16_t width)
{
   uint16_t i ; // 计数变量

	for (i = 0; i < width; i++)
	{
       LCD_Buff[i] =  LCD.Color;  // 写入缓冲区
   }   
   LCD_SetAddress( x, y, x+width-1, y);	     // 设置坐标	

   LCD_WriteBuff(LCD_Buff,width);          // 写入显存
}

/***************************************************************************************************************************************
*	函 数 名: LCD_DrawRect
*
*	入口参数: x - 水平坐标
*			 	 y - 垂直坐标
*			 	 width  - 水平宽度
*				 height - 垂直宽度
*
*	函数功能: 在指点位置绘制指定长宽的矩形线条
*
*	说    明: 1. 该函数移植于ST官方评估板的例程
*				 		2. 要绘制的区域不能超过屏幕的显示区域
*						 
*****************************************************************************************************************************************/

void LCD_DrawRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height)
{
	 // 绘制水平线
   LCD_DrawLine_H( x,  y,  width);           
   LCD_DrawLine_H( x,  y+height-1,  width);

   // 绘制垂直线
   LCD_DrawLine_V( x,  y,  height);
   LCD_DrawLine_V( x+width-1,  y,  height);
}

/***************************************************************************************************************************************
*	函 数 名: LCD_DrawCircle
*
*	入口参数: x - 圆心 水平坐标
*			 	 y - 圆心 垂直坐标
*			 	 r  - 半径
*
*	函数功能: 在坐标 (x,y) 绘制半径为 r 的圆形线条
*
*	说    明: 1. 该函数移植于ST官方评估板的例程
*				 2. 要绘制的区域不能超过屏幕的显示区域
*
*****************************************************************************************************************************************/

void LCD_DrawCircle(uint16_t x, uint16_t y, uint16_t r)
{
	int Xadd = -r, Yadd = 0, err = 2-2*r, e2;
	do {   

		LCD_DrawPoint(x-Xadd,y+Yadd,LCD.Color);
		LCD_DrawPoint(x+Xadd,y+Yadd,LCD.Color);
		LCD_DrawPoint(x+Xadd,y-Yadd,LCD.Color);
		LCD_DrawPoint(x-Xadd,y-Yadd,LCD.Color);
		
		e2 = err;
		if (e2 <= Yadd) {
			err += ++Yadd*2+1;
			if (-Xadd == Yadd && e2 <= Xadd) e2 = 0;
		}
		if (e2 > Xadd) err += ++Xadd*2+1;
    }
    while (Xadd <= 0);   
}


/***************************************************************************************************************************************
*	函 数 名: LCD_DrawEllipse
*
*	入口参数: x - 圆心 水平坐标
*			 	 y - 圆心 垂直坐标
*			 	 r1  - 水平半轴的长度
*				 r2  - 垂直半轴的长度
*
*	函数功能: 在坐标 (x,y) 绘制水平半轴为 r1 垂直半轴为 r2 的椭圆线条
*
*	说    明: 1. 该函数移植于ST官方评估板的例程
*				 2. 要绘制的区域不能超过屏幕的显示区域
*
*****************************************************************************************************************************************/

void LCD_DrawEllipse(int x, int y, int r1, int r2)
{
  int Xadd = -r1, Yadd = 0, err = 2-2*r1, e2;
  float K = 0, rad1 = 0, rad2 = 0;
   
  rad1 = r1;
  rad2 = r2;
  
  if (r1 > r2)
  { 
    do {
      K = (float)(rad1/rad2);
		 
			LCD_DrawPoint(x-Xadd,y+(uint16_t)(Yadd/K),LCD.Color);
			LCD_DrawPoint(x+Xadd,y+(uint16_t)(Yadd/K),LCD.Color);
			LCD_DrawPoint(x+Xadd,y-(uint16_t)(Yadd/K),LCD.Color);
			LCD_DrawPoint(x-Xadd,y-(uint16_t)(Yadd/K),LCD.Color);     
		 
      e2 = err;
      if (e2 <= Yadd) {
        err += ++Yadd*2+1;
        if (-Xadd == Yadd && e2 <= Xadd) e2 = 0;
      }
      if (e2 > Xadd) err += ++Xadd*2+1;
    }
    while (Xadd <= 0);
  }
  else
  {
    Yadd = -r2; 
    Xadd = 0;
    do { 
      K = (float)(rad2/rad1);

			LCD_DrawPoint(x-(uint16_t)(Xadd/K),y+Yadd,LCD.Color);
			LCD_DrawPoint(x+(uint16_t)(Xadd/K),y+Yadd,LCD.Color);
			LCD_DrawPoint(x+(uint16_t)(Xadd/K),y-Yadd,LCD.Color);
			LCD_DrawPoint(x-(uint16_t)(Xadd/K),y-Yadd,LCD.Color);  
		 
      e2 = err;
      if (e2 <= Xadd) 
			{
        err += ++Xadd*3+1;
        if (-Yadd == Xadd && e2 <= Yadd) e2 = 0;
      }
      if (e2 > Yadd) err += ++Yadd*3+1;     
    }
    while (Yadd <= 0);
  }
}


/***************************************************************************************************************************************
*	函 数 名: LCD_FillCircle
*
*	入口参数: x - 圆心 水平坐标
*			 	 y - 圆心 垂直坐标
*			 	 r  - 半径
*
*	函数功能: 在坐标 (x,y) 填充半径为 r 的圆形区域
*
*	说    明: 1. 该函数移植于ST官方评估板的例程
*				 2. 要绘制的区域不能超过屏幕的显示区域
*
*****************************************************************************************************************************************/

void LCD_FillCircle(uint16_t x, uint16_t y, uint16_t r)
{
  int32_t  D;    /* Decision Variable */ 
  uint32_t  CurX;/* Current X Value */
  uint32_t  CurY;/* Current Y Value */ 
  
  D = 3 - (r << 1);
  
  CurX = 0;
  CurY = r;
  
  while (CurX <= CurY)
  {
    if(CurY > 0) 
    { 
      LCD_DrawLine_V(x - CurX, y - CurY,2*CurY);
      LCD_DrawLine_V(x + CurX, y - CurY,2*CurY);
    }
    
    if(CurX > 0) 
    {
		// LCD_DrawLine(x - CurY, y - CurX,x - CurY,y - CurX + 2*CurX);
		// LCD_DrawLine(x + CurY, y - CurX,x + CurY,y - CurX + 2*CurX); 	

      LCD_DrawLine_V(x - CurY, y - CurX,2*CurX);
      LCD_DrawLine_V(x + CurY, y - CurX,2*CurX);
    }
    if (D < 0)
    { 
      D += (CurX << 2) + 6;
    }
    else
    {
      D += ((CurX - CurY) << 2) + 10;
      CurY--;
    }
    CurX++;
  }
  LCD_DrawCircle(x, y, r);  
}




/***************************************************************************************************************************************
*	函 数 名: LCD_FillRect
*
*	入口参数: x - 水平坐标
*			 	 y - 垂直坐标
*			 	 width  - 水平宽度
*				 height -垂直宽度
*
*	函数功能: 在坐标 (x,y) 填充指定长宽的实心矩形
*
*	说    明: 要绘制的区域不能超过屏幕的显示区域
*						 
*****************************************************************************************************************************************/

void LCD_FillRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height,uint16_t Color)
{
	uint16_t i;

   LCD_SetAddress( x, y, width-1, height-1);	// 设置坐标	

  at_spi_switch_to_16bit();           // 切换成16位数据格式
	
	 	// 片选拉低，使能IC
	
	for(i=0;i<width*height;i++)				
	{
		at_spi_transmit_16bit(SPI1,LCD.Color);
//    spi_i2s_data_transmit(SPI1, LCD.Color);
	}
//	while( (LCD_SPI.Instance->SR & 0x0080) != RESET);	//	等待通信完成
	 	// 片选拉高	
	
  at_spi_switch_to_8bit();           // 切换成8位数据格式
}





/***************************************************************************************************************************************
*	函 数 名: LCD_DrawImage
*
*	入口参数: x - 起始水平坐标
*				 y - 起始垂直坐标
*			 	 width  - 图片的水平宽度
*				 height - 图片的垂直宽度
*				*pImage - 图片数据存储区的首地址
*
*	函数功能: 在指定坐标处显示图片
*
*	说    明: 1.要显示的图片需要事先进行取模、获悉图片的长度和宽度
*            2.使用 LCD_SetColor() 函数设置画笔色，LCD_SetBackColor() 设置背景色
*						 
*****************************************************************************************************************************************/

void 	LCD_DrawImage(uint16_t x,uint16_t y,uint16_t width,uint16_t height,const uint8_t *pImage) 
{  
   uint8_t   disChar;	         // 字模的值
	uint16_t  Xaddress = x;       // 水平坐标
 	uint16_t  Yaddress = y;       // 垂直坐标  
	uint16_t  i=0,j=0,m=0;        // 计数变量
	uint16_t  BuffCount = 0;      // 缓冲区计数
   uint16_t  Buff_Height = 0;    // 缓冲区的行数

// 因为缓冲区大小有限，需要分多次写入
   Buff_Height = (sizeof(LCD_Buff)/2) / height;    // 计算缓冲区能够写入图片的多少行

	for(i = 0; i <height; i++)             // 循环按行写入
	{
		for(j = 0; j <(float)width/8; j++)  
		{
			disChar = *pImage;

			for(m = 0; m < 8; m++)
			{ 
				if(disChar & 0x01)	
				{		
               LCD_Buff[BuffCount] =  LCD.Color;			// 当前模值不为0时，使用画笔色绘点
				}
				else		
				{		
				   LCD_Buff[BuffCount] = LCD.BackColor;		//否则使用背景色绘制点
				}
				disChar >>= 1;     // 模值移位
				Xaddress++;        // 水平坐标自加
				BuffCount++;       // 缓冲区计数       
				if( (Xaddress - x)==width ) // 如果水平坐标达到了字符宽度，则退出当前循环,进入下一行的绘制		
				{											 
					Xaddress = x;				                 
					break;
				}
			}	
			pImage++;			
		}
      if( BuffCount == Buff_Height*width  )  // 达到缓冲区所能容纳的最大行数时
      {
         BuffCount = 0; // 缓冲区计数清0

         LCD_SetAddress( x, Yaddress , x+width-1, Yaddress+Buff_Height-1);	// 设置坐标	
         LCD_WriteBuff(LCD_Buff,width*Buff_Height);          // 写入显存     

         Yaddress = Yaddress+Buff_Height;    // 计算行偏移，开始写入下一部分数据
      }     
      if( (i+1)== height ) // 到了最后一行时
      {
         LCD_SetAddress( x, Yaddress , x+width-1,i+y);	   // 设置坐标	
         LCD_WriteBuff(LCD_Buff,width*(i+1+y-Yaddress));    // 写入显存     
      }
	}	
}

//LSB 逐行扫描 真16位色彩
void LCD_DrawImage_RGB565(uint16_t x, uint16_t y, uint16_t width, uint16_t height, const uint8_t *pImage)
{  
    uint16_t color;              // 用于存储16位的颜色值
    uint16_t Yaddress = y;       // 垂直坐标  
    uint16_t i = 0, j = 0;       // 计数变量
    uint16_t BuffCount = 0;      // 缓冲区计数
    uint16_t Buff_Height = 0;    // 缓冲区的行数

    // 计算缓冲区能够写入图片的行数
    Buff_Height = (sizeof(LCD_Buff) / 2) / width;

    for(i = 0; i < height; i++)  // 循环按行写入
    {
        for(j = 0; j < width; j++)
        {
            // 从pImage中读取16位RGB565颜色值
            color = (*(pImage + 1) << 8) | *pImage;  // 高字节在前，低字节在后
            pImage += 2;  // 移动到下一个像素

            // 将颜色值写入缓冲区
            LCD_Buff[BuffCount] = color;
            BuffCount++;

            // 当缓冲区满时，写入LCD并重置缓冲区
            if(BuffCount == Buff_Height * width)
            {
                BuffCount = 0;
                LCD_SetAddress(x, Yaddress, x + width - 1, Yaddress + Buff_Height - 1);  // 设置坐标
                LCD_WriteBuff(LCD_Buff, width * Buff_Height);  // 写入显存
                Yaddress += Buff_Height;  // 行偏移
            }
        }

        // 如果到了最后一行
        if((i + 1) == height)
        {
            LCD_SetAddress(x, Yaddress, x + width - 1, i + y);
            LCD_WriteBuff(LCD_Buff, width * (i + 1 + y - Yaddress));  // 写入显存
        }
    }
}

//////LSB 逐行扫描 真16位色彩
//void LCD_DrawImage_RGB565_FromFlash(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t flashaddr)
//{
//		uint16_t Buff_Height = (sizeof(LCD_Buff) / 2) / width;  // 计算缓冲区能存放多少行,   除2是因为
//    uint16_t full_blocks = height / Buff_Height;            // 完整的缓冲区块数
//    uint16_t remaining_lines = height % Buff_Height;        // 剩余不能填满缓冲区的行数

//    uint16_t Yaddress = y;                                  // 用于显示行的垂直位置
//    uint32_t data_size = Buff_Height * width * 2;           // 每次读取的数据大小（单位: 字节）

//		uint16_t block;
//	
//    // 合并Flash读取和LCD写入，减少函数调用开销
//    // 处理完整的缓冲区块
//    for (block = 0; block < full_blocks; block++)
//    {
//        flash_spi_read((uint8_t *)LCD_Buff, flashaddr, data_size);  // 从Flash读取一个缓冲区块的数据
//        flashaddr += data_size;                                     // 更新Flash地址
//        LCD_SetAddress(x, Yaddress, x + width - 1, Yaddress + Buff_Height - 1);  // 设置写入LCD的地址范围
//        LCD_WriteBuff(LCD_Buff, width * Buff_Height);               // 写入LCD
//        Yaddress += Buff_Height;                                    // 垂直偏移
//    }

//    // 仅处理剩余的最后一部分行
//    if (remaining_lines > 0)
//    {
//        data_size = remaining_lines * width * 2;                    // 计算剩余行的字节数
//        flash_spi_read((uint8_t *)LCD_Buff, flashaddr, data_size);  // 从Flash读取剩余行的数据
//        LCD_SetAddress(x, Yaddress, x + width - 1, y + height - 1); // 设置写入LCD的地址范围
//        LCD_WriteBuff(LCD_Buff, width * remaining_lines);           // 写入LCD剩余行
//    }
//}


