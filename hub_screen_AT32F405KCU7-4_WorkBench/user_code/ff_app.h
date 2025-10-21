#ifndef __FF_APP_H__
#define __FF_APP_H__

#include "ff.h"
#include "at32f402_405_wk_config.h"

#include <string.h>
#include "at32_qspiflash.h"


extern FATFS fs;        /* FatFs文件系统对象 */
extern FRESULT res_csv; /* 文件操作结果 */

extern MKFS_PARM opt;       // 格式化选项
extern BYTE work_buf[4096]; // 工作缓冲区（4096字节，足够大多数场景）
extern char *fileheader;    // 写入文件内容

extern FIL fnew_csv;      /* 文件对象 */
extern UINT fnum_csv[4];     /* 文件成功读写的数量，是一个返回值*/
extern char gif1[16];                              // 文件名
extern char gif2[16];                              // 文件名
extern char gif3[16];                              // 文件名

typedef enum _running_status_
{
	complete=0,
	running
}running_status;
//enum running_status
//{
//	suspend,
//	running
//};


typedef struct _mode_running_mark_
{ 
	running_status oled;
	running_status flash;
}mode_running_mark;
extern mode_running_mark dma_status;

void Filesystem_init(void);
void DRAW_IMAGE_QSPI(uint8_t frame);
#endif
