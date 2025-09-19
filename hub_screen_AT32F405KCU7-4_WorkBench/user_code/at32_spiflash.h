#ifndef __AT_SPI_FLASH_H
#define __AT_SPI_FLASH_H
#include "at32f402_405.h"

#define FLASH_SPI                   SPI1
//#define FLASH_SPI_CLOCK             CRM_SPI1_PERIPH_CLOCK

///*SCK Pin*/
//#define FLASH_SPI_SCK_PIN           GPIO_PINS_3
//#define FLASH_SPI_SCK_GPIO          GPIOB
//#define FLASH_SPI_SCK_GPIO_CLOCK    CRM_GPIOB_PERIPH_CLOCK

///*MISO Pin*/
//#define FLASH_SPI_MISO_PIN          GPIO_PINS_4
//#define FLASH_SPI_MISO_GPIO         GPIOB
//#define FLASH_SPI_MISO_GPIO_CLOCK   CRM_GPIOB_PERIPH_CLOCK

///*MOSI Pin*/
//#define FLASH_SPI_MOSI_PIN          GPIO_PINS_5
//#define FLASH_SPI_MOSI_GPIO         GPIOB
//#define FLASH_SPI_MOSI_GPIO_CLOCK   CRM_GPIOB_PERIPH_CLOCK

/*CS Pin*/
#define FLASH_SPI_CS_PIN            GPIO_PINS_5
#define FLASH_SPI_CS_GPIO           GPIOA
//#define FLASH_SPI_CS_GPIO_CLOCK     CRM_GPIOA_PERIPH_CLOCK

// W25Q64指令表1
#define W25Q64_Write_Enable                             0x06    // 写使能
#define W25Q64_Write_Disable                            0x04    // 写禁止
#define W25Q64_Read_Status_register_1                   0x05    // 读状态寄存器1
#define W25Q64_Read_Status_register_2                   0x35    // 读状态寄存器2
#define W25Q64_Write_Status_register                    0x01    // 写状态寄存器
#define W25Q64_Page_Program                             0x02    // 页编程
#define W25Q64_Quad_Page_Program                        0x32    // 四通道页编程
#define W25Q64_Block_Erase_64KB                         0xDC    // 擦除64KB块,后面跟随四位地址
#define W25Q64_Block_Erase_32KB                         0x52    // 擦除32KB块
#define W25Q64_Sector_Erase_4KB                         0x21    // 擦除4KB扇区,后面跟随四位地址
#define W25Q64_Chip_Erase                               0x60    // 擦除整个芯片0xC7
#define W25Q64_Erase_Suspend                            0x75    // 擦除暂停
#define W25Q64_Erase_Resume                             0x7A    // 擦除恢复
#define W25Q64_Power_down                               0xB9    // 进入掉电模式
#define W25Q64_High_Performance_Mode                    0xA3    // 高性能模式
#define W25Q64_Continuous_Read_Mode_Reset               0xFF    // 连续读取模式复位
#define W25Q64_Release_Power_Down_HPM_Device_ID         0xAB    // 解除掉电/高性能模式，读取设备ID
#define W25Q64_Manufacturer_Device_ID                   0x90    // 读取制造商ID/设备ID
#define W25Q64_Read_Unique_ID                           0x4B    // 读取唯一ID
#define W25Q64_JEDEC_ID                                 0x9F    // 读取JEDEC ID

// W25Q64指令集表2(读指令)
#define W25Q64_Read_Data                                0x03    // 读取数据
#define W25Q64_Fast_Read                                0x0B    // 快速读取
#define W25Q64_Fast_Read_Dual_Output                    0x3B    // 双输出快速读取
#define W25Q64_Fast_Read_Dual_IO                        0xBB    // 双I/O快速读取
#define W25Q64_Fast_Read_Quad_Output                    0x6B    // 四输出快速读取
#define W25Q64_Fast_Read_Quad_IO                        0xEB    // 四I/O快速读取
#define W25Q64_Octal_Word_Read_Quad_IO                  0xE3    // 八字节四I/O读取


// 定义 SPI Flash 的 Chip Select 引脚操作
#define qspi_cs_flash       gpio_bits_reset(FLASH_SPI_CS_GPIO, FLASH_SPI_CS_PIN)  // CS 置低
#define qspi_cs_oled      gpio_bits_set(FLASH_SPI_CS_GPIO, FLASH_SPI_CS_PIN)    // CS 置高


///* FLASH 指令, 不同 FLASH 型号的指令不同, 此处为 Demo 用的 FLASH 型号 W25Q128FV */
//#define  FLASH_GETID              0x90  /*!< 获取 FLASH ID 的指令 */
//#define  FLASH_CMD_WREN           0x06  /*!< 写使能指令 */
//#define  FLASH_CMD_WRDIS           0x06  /*!< 写使能指令 */
//#define  FLASH_CMD_ERASE_SECTOR	  0x20  /*!< 擦除扇区指令 */
//#define  FLASH_CMD_READSTS	      0x05  /*!< 读取状态寄存器指令 */
//#define  FLASH_CMD_WRITEPAGE	    0x02  /*!< 页编程指令 */
//#define  FLASH_CMD_READPAGE	      0x03  /*!< 页读取指令 */



uint16_t flash_qspi_init(void);

uint32_t flash_qspi_write(uint8_t* pbuffer, uint32_t writeaddr, uint16_t numbytetowrite);
uint32_t flash_qspi_read(uint8_t* pbuffer, uint32_t readaddr, uint16_t numbytetoread);

void qspi_flash_write_xip_dma_set(uint32_t add, uint8_t* buf, uint32_t length);
void qspi_flash_read_xip_dma_set(uint32_t add, uint8_t* buf, uint32_t length);


void flash_qspi_sendbyte(uint8_t byte);       // 发送一个字节
void flash_qspi_writeenable(void);

void flash_qspi_erasechip(void);
void flash_qspi_erase_block(uint32_t address);

uint32_t flash_qspi_getsize(void);
uint32_t flash_qspi_sectorsize(void);
uint32_t flash_qspi_pagesize(void);
uint32_t flash_qspi_getid(void);

void flash_qspi_writebuffer(uint8_t* pbuffer, uint32_t writeaddr, uint16_t numbytetowrite);

void qspi_read_spi_data(uint8_t *data, uint32_t counter);
void qspi_read_spi_command(uint32_t add, uint32_t counter);

void qspi_read_command(uint32_t add, uint32_t counter);
void qspi_read_data(uint8_t *data, uint32_t counter);

void qspi_write_command(uint32_t add, uint32_t counter);
void qspi_write_data(uint8_t *data, uint32_t counter);

void QSPI_flash_mode(void);
#endif


