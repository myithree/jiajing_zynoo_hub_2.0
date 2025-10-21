/*-----------------------------------------------------------------------*/
/* Low level disk I/O module SKELETON for FatFs     (C)ChaN, 2019        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "ff.h"		/* Obtains integer types */
#include "diskio.h" /* Declarations of disk functions */
#include "at32f402_405_wk_config.h"
#include "at32_qspiflash.h"

/* Definitions of physical drive number for each drive */
#define material 0 /* Example: Map Ramdisk to physical drive 0 */
#define firmware 1 /* Example: Map MMC/SD card to physical drive 1 */
#define DEV_USB 2  /* Example: Map USB MSD to physical drive 2 */

#define sFLASH_ID 0x18EF

void qspi_flash_read_xip_dma_set(uint32_t add, uint8_t *buf, uint32_t length);
void flash_qspi_writebuffer(uint8_t *pbuffer, uint32_t writeaddr, uint16_t numbytetowrite);
uint32_t flash_get_sector_size(void);
uint32_t flash_get_sector_num(void);

DWORD get_fattime(void) // 返回
{
	return ((DWORD)(2025 - 1980) << 25) | // Year 2025
		   ((DWORD)7 << 21) |			  // Month 7
		   ((DWORD)16 << 16) |			  // Mday 16
		   ((DWORD)0 << 11) |			  // Hour 0
		   ((DWORD)0 << 5) |			  // Min 0
		   ((DWORD)0 >> 1);				  // Sec 0 秒要除以2
}

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status(
	BYTE pdrv /* Physical drive nmuber to identify the drive */
)
{
	DSTATUS status = STA_NOINIT;
	switch (pdrv)
	{
	case material:
		/* SPI Flash状态检测：读取SPI Flash 设备ID */
		if (sFLASH_ID == flash_qspi_init()) // #define sFLASH_ID 0XEF18 //这为W25Q256设备返回值
		{
			/* 设备ID读取结果正确 */
			status = STA_NODISK;
		}
		else
		{
			/* 设备ID读取结果错误 */
			status = STA_NOINIT;
		}
		break;
	case firmware:
		/* SPI Flash状态检测：读取SPI Flash 设备ID */
		if (sFLASH_ID == flash_qspi_init()) // #define sFLASH_ID 0XEF18 //这为W25Q256设备返回值
		{
			/* 设备ID读取结果正确 */
			status = STA_NODISK;
		}
		else
		{
			/* 设备ID读取结果错误 */
			status = STA_NOINIT;
		}
		break;
	default:
		status = STA_NOINIT;
	}
	return status;
}

/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize(
	BYTE pdrv /* Physical drive nmuber to identify the drive */
)
{
	DSTATUS status = STA_NOINIT;
	switch (pdrv)
	{
	case material:
		status = disk_status(material);
		break;
	case firmware: /* SPI Flash */
		/* 初始化SPI Flash */
		// SPI_Flash_Init(); // 就是对存储介质所连接芯片的IO进行初始化，对IO口所对应的SPI进行初始化
		/* 延时一小段时间 */
		// i = 500;
		// while (--i)
		// 	;
		/* 唤醒SPI Flash */
		// SPI_Flash_WAKEUP();
		/* 获取SPI Flash芯片状态 */
		status = disk_status(firmware); // 01
		break;
	default:
		status = STA_NOINIT;
	}
	return status;
}

/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read(
	BYTE pdrv,	  /* Physical drive nmuber to identify the drive */
	BYTE *buff,	  /* Data buffer to store read data */
	LBA_t sector, /* Start sector in LBA */
	UINT count	  /* Number of sectors to read */
)
{
	DRESULT status = RES_PARERR;
	QSPI_flash_mode();
	switch (pdrv)
	{
	case material:
		sector += 0;
	qspi_flash_read_xip_dma_set(sector << 12, buff, count == 0 ? 4096 : count << 12);
		status = RES_OK;
		break;

	case firmware:
		sector += 7936;
		// 一个数左移12位，就是扩大2的12次方，4096倍，即FatFs的最小单位设置成4096个字节

		qspi_flash_read_xip_dma_set(sector << 12, buff, count == 0 ? 4096 : count << 12);
		status = RES_OK; // 在这默认全部读取正常
		break;

	default:
		status = RES_PARERR;
	}
	return status;
}

/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if FF_FS_READONLY == 0

DRESULT disk_write(
	BYTE pdrv,		  /* Physical drive nmuber to identify the drive */
	const BYTE *buff, /* Data to be written */
	LBA_t sector,	  /* Start sector in LBA */
	UINT count		  /* Number of sectors to write */
)
{
	uint32_t write_addr;
	DRESULT status = RES_PARERR;
	if (!count)
	{
		return RES_PARERR; /* Check parameter */
	}

	switch (pdrv)
	{
	case material:

		while (count--)
		{
			write_addr = sector << 12;
			flash_qspi_erase_block(write_addr); // 擦除
			flash_qspi_writebuffer((uint8_t *)buff, write_addr, 4096); // 写入
			sector++;
			buff += 4096; // 默认FatFs的一个扇区（簇）4096个大小
		}
		status = RES_OK;
		break;
	case firmware:
		sector += 7936;

		while (count--)
		{
			write_addr = sector << 12;
			flash_qspi_erase_block(write_addr); // 擦除

			flash_qspi_writebuffer((uint8_t *)buff, write_addr, 4096); // 写入
			sector++;
			buff += 4096; // 默认FatFs的一个扇区（簇）4096个大小
		}
		/*
		//一般使用过程中都操作一个，可以直接按照下面这样写，但按照上面这样写更严谨
		write_addr = sector<<12;
		SPI_Flash_Erase_Sector(write_addr);//擦除
		SPI_Flash_Write((u8 *)buff,write_addr,count<<12);//写入
		*/
		status = RES_OK;
		break;
	default:
		status = RES_PARERR;
	}
	return status;
}

#endif

/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl(
	BYTE pdrv, /* Physical drive nmuber (0..) */
	BYTE cmd,  /* Control code */
	void *buff /* Buffer to send/receive control data */
)
{
	DRESULT status = RES_PARERR;
	switch (pdrv)
	{
	case material:
		switch (cmd)
		{
		case CTRL_SYNC:
			status = RES_OK;
			break;
		case GET_SECTOR_COUNT: // 扇区个数
			// 大小：7936*4096/1024/1024=31MB
			// 总共32MB，分给这个区31MB,31*1024(KB)*1024(Byte)/4096=7936，按照最大扇区设置的4096个字节，分了7936个扇区给它
			*(DWORD *)buff = 7936;
			status = RES_OK;
			break;
		/* 扇区大小  */
		case GET_SECTOR_SIZE:
			*(WORD *)buff = 4096;
			status = RES_OK;
			break;
		/* 同时擦除扇区个数 */
		case GET_BLOCK_SIZE:
			*(DWORD *)buff = 1;
			status = RES_OK;
			break;
		case CTRL_TRIM:
			status = RES_OK;
			break;
		default:
			status = RES_PARERR;
			break;
		}
		break;
	case firmware:
		switch (cmd)
		{
		case CTRL_SYNC: // 写入同步在disk_write函数已经完成，这里默认返回ok
			status = RES_OK;
			break;
		case GET_SECTOR_COUNT: // 扇区个数
			// 大小：256*4096/1024/1024=1MB
			*(DWORD *)buff = 256; // 存储空间大小，有多少个FatFs扇区(sector)
			// 总共32MB，分给这个区1MB,1*1024(KB)*1024(Byte)/4096=256，按照最大扇区设置的4096个字节，分了256个扇区给它
			status = RES_OK;
			break;
		/* 扇区大小  */
		case GET_SECTOR_SIZE:
			*(WORD *)buff = 4096; // 每个FatFs扇区(sector)的大小，有多少个字节
			status = RES_OK;
			break;
		/* 同时擦除扇区个数 */
		case GET_BLOCK_SIZE: // 获取擦除的最小个数，以FatFs扇区（sector）为单位，注意：FatFs的扇区和Flash的扇区不是一个概念
			*(DWORD *)buff = 1;
			status = RES_OK;
			break;
		case CTRL_TRIM:
			status = RES_OK;
			break;
		default:
			status = RES_PARERR;
			break;
		}
		break;
	default:
		status = RES_PARERR;
	}
	return status;
}
