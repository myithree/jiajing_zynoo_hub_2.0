#include "ff_app.h"
#include "lcd_spi.h"

mode_running_mark dma_status;

FATFS fs;		  /* FatFs文件系统对象 */
FRESULT res_csv;  /* 文件操作结果 */

DIR my_dp[10];     /*文件目录*/
FILINFO my_file[10] = {NULL};  //目录文件

FIL fnew_csv;	  /* 文件对象 */
UINT fnum_csv[4]; /* 文件成功读写的数量，是一个返回值*/
UINT fnum_1csv[4];
uint32_t dsds = 33;

MKFS_PARM opt;		 // 格式化选项
BYTE work_buf[4096]; // 工作缓冲区（4096字节，足够大多数场景）

char gif1[16] = "0:gif1";							   // 文件名
char gif2[16] = "0:gif2";							   // 文件名
char gif3[16] = "0:gif3";							   // 文件名
char *fileheader = "Time,,Alarm value,,Alarm Way\r\n"; // 写入文件内容

void Filesystem_init(void)
{
	unsigned int i;
	
	res_csv = f_mount(&fs, "0:", 1);
	if (res_csv == FR_NO_FILESYSTEM)
	{
		opt.fmt = FM_FAT; // 自动选择格式化类型(FAT12/FAT16/FAT32)
		/* 格式化，操作FLASH_DATA存储介质，并且操作硬盘：1；默认自行分配大小：0*/
		res_csv = f_mkfs("0:", &opt, work_buf, (UINT)sizeof(work_buf));

		/* 格式化后，先取消挂载 */
		res_csv = f_mount(NULL, "0:", 1);
		/* 重新挂载	*/
		res_csv = f_mount(&fs, "0:", 1);
	}
//		res_csv = f_open(&fnew_csv, gif1,  FA_OPEN_ALWAYS);
//	  res_csv = f_open(&fnew_csv, gif2,  FA_OPEN_ALWAYS);
//	  res_csv = f_open(&fnew_csv, gif3,  FA_OPEN_ALWAYS);
		res_csv = f_open(&fnew_csv, gif1,  FA_READ);
	  res_csv = f_read(&fnew_csv, work_buf, 60, fnum_csv);
		f_close(&fnew_csv);
	
		res_csv = f_opendir(my_dp, "0:");
		i = 0;
		res_csv = f_readdir(my_dp,my_file);
		while(my_file[i].fname[0] != NULL)
		{
			i++;
			res_csv = f_readdir(my_dp,my_file+i);
			
		}
		f_closedir(my_dp);


	f_close(&fnew_csv);
}

// 画图函数

//void DRAW_IMAGE_QSPI(uint8_t frame)
//{
//	unsigned int i, j;

//	qspi_xip_enable(QSPI1, TRUE);

//	for (j = 0; j < 4; j++)
//	{
//		qspi_cs_flash;
//		QSPI_flash_mode();
//		f_open(&fnew_csv, gif1, FA_READ);
//		for (i = 0; i < 297 * 3 + 3; i++)
//		{
//			//      qspi_flash_read_xip_dma_set(Updata_state_adderss + (LCD_Width * LCD_Height * 3 * frame) + i * 60 + (LCD_Height) * (LCD_Width) * 3 / 4 * (j), image_biffer + i * 60, 60);
//			f_lseek(
//				&fnew_csv,
//				(LCD_Width * LCD_Height * 3 * frame) + i * 60 + (LCD_Height) * (LCD_Width) * 3 / 4 * (j));
//			f_read(&fnew_csv, (image_biffer + i * 60), 60, fnum_csv);
//		}
//		qspi_cs_oled;;
//		QSPI_oled_mode();
//		if (!i)
//		{
//			AMOLED_Block_Write(
//				0,
//				LCD_Width - 1,
//				LCD_Height / 4 * j,
//				LCD_Height / 4 * (j + 1) - 1); // set AMOLED GRAM
//			qspi_xip_dma_set(0x2C, image_biffer, (LCD_Height) * (LCD_Width) * 3 / 4, TRUE);
//		}
//		else
//		{
//			qspi_xip_dma_set(0x3C, image_biffer, (LCD_Height) * (LCD_Width) * 3 / 4, TRUE);
//		}
//	}
//	f_close(&fnew_csv);
//}
