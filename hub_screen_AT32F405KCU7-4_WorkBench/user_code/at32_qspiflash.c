#include "at32_qspiflash.h"
#include "wk_system.h"

// 定义 Flash 操作相关常量
#define FLASH_PAGE_SIZE 256      // Flash 页大小
#define FLASH_SECTOR_SIZE 0x1000 // Flash 扇区大小 (4KB)

// 静态函数声明
// static void flash_qspi_writeenable(void);               // 启用写操作
static void flash_qspi_waitforwriteend(void);                                                    // 等待写操作结束
static void flash_qspi_writepage(uint8_t *pbuffer, uint32_t writeaddr, uint16_t numbytetowrite); // 写入一页数据
void w25q64_cmd_read_spi_config(qspi_cmd_type *qspi_cmd_struct, uint32_t addr, uint32_t counter);
void w25n_read_id(uint8_t *id_buf);
uint32_t flash_spi_getid(void); // 获取设备 ID
void flash_qspi_sendbyte(uint8_t commder);

qspi_cmd_type w25q64_cmd_config; // 配置qspi结构体

// Flash 的大小和扇区大小
uint32_t qspi_flash_size = 0x1000000;              // Flash 总大小，16MB

/**
 * @brief  初始化 SPI Flash 的外设和 GPIO,读取 FLASH 设备的标识符。
 * @param  None
 * @retval 返回 FLASH 设备的标识符
 */
uint16_t flash_qspi_init(void)
{
  // 获取设备 ID
  uint16_t device_id = 0;
  
  w25n_read_id((uint8_t *)(&device_id));

  if (device_id == 0x16EF) {
    qspi_flash_size = 0x800000;  // 设备为 8MB Flash
  } 
	else if (device_id == 0x17EF) 
	{
    qspi_flash_size = 0x1000000;  // 设备为 16MB Flash
  } 
	else if (device_id == 0x18EF) 
	{
    qspi_flash_size = 0x2000000;  // 设备为 16MB Flash
  } 
	else {
	qspi_flash_size = 0;  // 未知设备，初始化失败
    return 1;
  }
  return device_id; // 初始化成功
}

/**
 * @brief  w25n read id
 * @param  id_buf: the pointer of id buf
 * @retval none
 */
void w25n_read_id(uint8_t *id_buf)
{
  uint8_t i;
  qspi_xip_enable(QSPI1, FALSE);

  w25q64_cmd_config.pe_mode_enable = FALSE;
  w25q64_cmd_config.pe_mode_operate_code = 0;
  w25q64_cmd_config.instruction_code = 0x90;
  w25q64_cmd_config.instruction_length = QSPI_CMD_INSLEN_1_BYTE;
  w25q64_cmd_config.address_code = 0;
  w25q64_cmd_config.address_length = QSPI_CMD_ADRLEN_3_BYTE;
  w25q64_cmd_config.data_counter = 2;
  w25q64_cmd_config.second_dummy_cycle_num = 0; // 单位是clk周期
  w25q64_cmd_config.operation_mode = QSPI_OPERATE_MODE_111;
  w25q64_cmd_config.read_status_config = QSPI_RSTSC_HW_AUTO;
  w25q64_cmd_config.read_status_enable = FALSE;
  w25q64_cmd_config.write_data_enable = FALSE;

  qspi_cmd_operation_kick(QSPI1, &w25q64_cmd_config);

  
  while (qspi_flag_get(QSPI1, QSPI_RXFIFORDY_FLAG) == RESET)
  {
    ;
  }

  for (i = 0; i < 2; ++i)
  {
    id_buf[i] = qspi_byte_read(QSPI1);
  }
  /* wait command completed */
  while (qspi_flag_get(QSPI1, QSPI_CMDSTS_FLAG) == RESET)
    ;
  qspi_flag_clear(QSPI1, QSPI_CMDSTS_FLAG);
}

/**
 * @brief  xip init w25q64 config
 * @param  qspi_xip_struct:the pointer for qspi_xip_type parameter
 * @retval none
 */
void xip_init_flash_config(qspi_xip_type *qspi_xip_struct)
{
  qspi_xip_struct->read_instruction_code = 0x6B;
  qspi_xip_struct->read_address_length = QSPI_XIP_ADDRLEN_3_BYTE;
  qspi_xip_struct->read_operation_mode = QSPI_OPERATE_MODE_114;
  qspi_xip_struct->read_second_dummy_cycle_num = 8;
  qspi_xip_struct->write_instruction_code = 0x32;
  qspi_xip_struct->write_address_length = QSPI_XIP_ADDRLEN_3_BYTE;
  qspi_xip_struct->write_operation_mode = QSPI_OPERATE_MODE_114;
  qspi_xip_struct->write_second_dummy_cycle_num = 0;
  qspi_xip_struct->write_select_mode = QSPI_XIPW_SEL_MODET;
  qspi_xip_struct->write_time_counter = 0x04;
  qspi_xip_struct->write_data_counter = 0x1F;
  qspi_xip_struct->read_select_mode = QSPI_XIPR_SEL_MODET;
  qspi_xip_struct->read_time_counter = 0x02;
  qspi_xip_struct->read_data_counter = 0x1F;
}

/****************************************************************************************************************************************
 *	函 数 名: QSPI_flash_mode
 *
 *	函数功能: 将qspi更改为flash传输模式
 *
 ****************************************************************************************************************************************/
qspi_xip_type flash_xip_config;
void QSPI_flash_mode(void)
{
  xip_init_flash_config(&flash_xip_config);
  qspi_xip_init(QSPI1, &flash_xip_config);
	qspi_xip_enable(QSPI1, TRUE);
}



/**
 * @brief  w25q64 qspi cmd 111 read config
 * @param  qspi_cmd_struct: the pointer for qspi_cmd_type parameter
 * @param  addr: read start address
 * @param  counter: write data counter
 * @retval none
 */
void w25q64_cmd_read_spi_config(qspi_cmd_type *qspi_cmd_struct, uint32_t addr, uint32_t counter)
{
  qspi_cmd_struct->pe_mode_enable = FALSE;
  qspi_cmd_struct->pe_mode_operate_code = 0;
  qspi_cmd_struct->instruction_code = addr;
  qspi_cmd_struct->instruction_length = QSPI_CMD_INSLEN_1_BYTE;
  //  qspi_cmd_struct->address_code = addr;
  qspi_cmd_struct->address_length = QSPI_CMD_ADRLEN_0_BYTE;
  qspi_cmd_struct->data_counter = counter;
  qspi_cmd_struct->second_dummy_cycle_num = 0; // 八个时钟周期
  qspi_cmd_struct->operation_mode = QSPI_OPERATE_MODE_111;
  qspi_cmd_struct->read_status_config = QSPI_RSTSC_SW_ONCE;
  qspi_cmd_struct->read_status_enable = TRUE;
  qspi_cmd_struct->write_data_enable = FALSE;
}


/**
 * @brief  qspi xip dma set for flash
 * @param  command: the command used to control CO5300
 * @param  buf: the pointer for dma data
 * @param  length: data length
 * @param  inc_en : the dma peripheral(data source buffer) address increase or not .
 *         this parameter can be: TRUE or FALSE.
 * @retval none
 */
void qspi_flash_write_xip_dma_set(uint32_t add, uint8_t *buf, uint32_t length)
{

  dma_init_type dma_init_struct;
  dma_reset(DMA2_CHANNEL1);
  // dma_default_para_init(&dma_init_struct);
  /* dma stream1 configuration */
  dma_init_struct.direction = DMA_DIR_MEMORY_TO_MEMORY;

  dma_init_struct.buffer_size = (uint16_t)length;
  dma_init_struct.peripheral_data_width = DMA_PERIPHERAL_DATA_WIDTH_BYTE;
  dma_init_struct.memory_data_width = DMA_MEMORY_DATA_WIDTH_BYTE;

  dma_init_struct.peripheral_base_addr = (uint32_t)buf;
  dma_init_struct.peripheral_inc_enable = TRUE;
  dma_init_struct.memory_base_addr = (uint32_t)(QSPI1_MEM_BASE + add); // QSPI1_MEM_BASE
  dma_init_struct.memory_inc_enable = TRUE;
  dma_init_struct.priority = DMA_PRIORITY_HIGH;
  dma_init_struct.loop_mode_enable = FALSE;
  dma_init(DMA2_CHANNEL1, &dma_init_struct);

  dmamux_enable(DMA2, TRUE);
  dmamux_init(DMA2MUX_CHANNEL1, DMAMUX_DMAREQ_ID_QSPI1);

  dma_channel_enable(DMA2_CHANNEL1, TRUE);

  /* wait dma completed */
  while (dma_flag_get(DMA2_FDT1_FLAG) == RESET)
    ;
  dma_flag_clear(DMA2_FDT1_FLAG);
  dma_channel_enable(DMA2_CHANNEL1, FALSE);
	qspi_dma_enable(QSPI1, FALSE);
}

/**
 * @brief  qspi xip dma set for flash
 * @param  command: the command used to control CO5300
 * @param  buf: the pointer for dma data
 * @param  length: data length
 * @param  inc_en : the dma peripheral(data source buffer) address increase or not .
 *         this parameter can be: TRUE or FALSE.
 * @retval none
 */
void qspi_flash_read_xip_dma_set(uint32_t add, uint8_t *buf, uint32_t length)
{
  dma_init_type dma_init_struct;

  dma_reset(DMA2_CHANNEL1);
  dma_default_para_init(&dma_init_struct);
  /* dma stream1 configuration */
  dma_init_struct.direction = DMA_DIR_MEMORY_TO_MEMORY;

  dma_init_struct.buffer_size = (uint16_t)length;
  dma_init_struct.peripheral_data_width = DMA_PERIPHERAL_DATA_WIDTH_BYTE;
  dma_init_struct.memory_data_width = DMA_MEMORY_DATA_WIDTH_BYTE;

  dma_init_struct.peripheral_base_addr = (uint32_t)(QSPI1_MEM_BASE + add); // QSPI1_MEM_BASE
  dma_init_struct.peripheral_inc_enable = TRUE;
  dma_init_struct.memory_base_addr = (uint32_t)buf;
  dma_init_struct.memory_inc_enable = TRUE;
  dma_init_struct.priority = DMA_PRIORITY_HIGH;
  dma_init_struct.loop_mode_enable = FALSE;
  dma_init(DMA2_CHANNEL1, &dma_init_struct);

  dmamux_enable(DMA2, TRUE);
  dmamux_init(DMA2MUX_CHANNEL1, DMAMUX_DMAREQ_ID_QSPI1);

  dma_channel_enable(DMA2_CHANNEL1, TRUE);

  /* wait dma completed */
  while (dma_flag_get(DMA2_FDT1_FLAG) == RESET)
    ;
		dma_flag_clear(DMA2_FDT1_FLAG);
		dma_channel_enable(DMA2_CHANNEL1, FALSE);
		qspi_dma_enable(QSPI1, FALSE);
}

void flash_qspi_sendbyte(uint8_t commder)
{
  qspi_xip_enable(QSPI1, FALSE); // FALSE
  // qspi_cmd_type *qspi_cmd_struct;
  (&w25q64_cmd_config)->pe_mode_enable = FALSE;
  (&w25q64_cmd_config)->pe_mode_operate_code = 0;
  (&w25q64_cmd_config)->instruction_code = commder; //
  (&w25q64_cmd_config)->instruction_length = QSPI_CMD_INSLEN_1_BYTE;
  (&w25q64_cmd_config)->address_code = 0;
  (&w25q64_cmd_config)->address_length = QSPI_CMD_ADRLEN_0_BYTE;
  (&w25q64_cmd_config)->data_counter = 0;
  (&w25q64_cmd_config)->second_dummy_cycle_num = 0; // 八个时钟周期
  (&w25q64_cmd_config)->operation_mode = QSPI_OPERATE_MODE_111;
  (&w25q64_cmd_config)->read_status_config = QSPI_RSTSC_SW_ONCE;
  (&w25q64_cmd_config)->read_status_enable = FALSE;
  (&w25q64_cmd_config)->write_data_enable = TRUE;

	
  qspi_cmd_operation_kick(QSPI1, &w25q64_cmd_config);

  while (qspi_flag_get(QSPI1, QSPI_CMDSTS_FLAG) == RESET)
    ;
  qspi_flag_clear(QSPI1, QSPI_CMDSTS_FLAG);
}

/**
 * @brief  启用 Flash 写入功能
 */
void flash_qspi_writeenable(void)
{
  flash_qspi_sendbyte(W25Q64_Write_Enable); 
}

/**
 * @brief  禁用 Flash 写入功能
 */
void flash_qspi_writedisable(void)
{ 
  flash_qspi_sendbyte(W25Q64_Write_Disable); 
}

/**
 * @brief  等待 Flash 写入操作结束
 */
static void flash_qspi_waitforwriteend(void)
{
  uint8_t flashstatus = 0;
  
  qspi_xip_enable(QSPI1, FALSE);

  w25q64_cmd_config.pe_mode_enable = FALSE;
  w25q64_cmd_config.pe_mode_operate_code = 0;
  w25q64_cmd_config.instruction_code = W25Q64_Read_Status_register_1;
  w25q64_cmd_config.instruction_length = QSPI_CMD_INSLEN_1_BYTE;
  w25q64_cmd_config.address_code = 0;
  w25q64_cmd_config.address_length = QSPI_CMD_ADRLEN_0_BYTE;
  w25q64_cmd_config.data_counter = 1;
  w25q64_cmd_config.second_dummy_cycle_num = 0; // 单位是clk周期
  w25q64_cmd_config.operation_mode = QSPI_OPERATE_MODE_111;
  w25q64_cmd_config.read_status_config = QSPI_RSTSC_HW_AUTO;
  w25q64_cmd_config.read_status_enable = FALSE;
  w25q64_cmd_config.write_data_enable = FALSE;

  //* read data
  do
  {
    qspi_flag_clear(QSPI1, QSPI_CMDSTS_FLAG);
    qspi_cmd_operation_kick(QSPI1, &w25q64_cmd_config);
    while (qspi_flag_get(QSPI1, QSPI_RXFIFORDY_FLAG) == RESET)
    {
      ;
    }
    flashstatus = qspi_byte_read(QSPI1);
    qspi_flag_clear(QSPI1, QSPI_CMDSTS_FLAG);
  } while ((flashstatus & 0x01) != RESET); // 检查写入进度标志位

  qspi_flag_clear(QSPI1, QSPI_CMDSTS_FLAG);
  wk_delay_ms(1);
  return;
}

/**
 * @brief  擦除 Flash 芯片
 * @param  void
 */
void flash_qspi_erasechip(void)
{
  // 启用写操作
  flash_qspi_writeenable();

  // 发送芯片擦除指令
  flash_qspi_sendbyte(W25Q64_Chip_Erase);

  //  等待写入完成
  flash_qspi_waitforwriteend();

  // 禁用写操作
  flash_qspi_writedisable();
}

/**
 * @brief  擦除单位为64kb的flash存储区域
 * @param  uint32_t address该地址只能为64k的倍数
 */
void flash_qspi_erase_block(uint32_t address)
{
  qspi_xip_enable(QSPI1, FALSE);
  // 启用写操作
  flash_qspi_writeenable();

  // 判断该地址强行转为为4k的倍数
  address = address - address % 4096;

  // 发送芯片擦除指令
  (&w25q64_cmd_config)->pe_mode_enable = FALSE;
  (&w25q64_cmd_config)->pe_mode_operate_code = 0;
  (&w25q64_cmd_config)->instruction_code = W25Q64_Sector_Erase_4KB; //
  (&w25q64_cmd_config)->instruction_length = QSPI_CMD_INSLEN_1_BYTE;
  (&w25q64_cmd_config)->address_code = address;
  (&w25q64_cmd_config)->address_length = QSPI_CMD_ADRLEN_4_BYTE;
  (&w25q64_cmd_config)->data_counter = 0;
  (&w25q64_cmd_config)->second_dummy_cycle_num = 0;
  (&w25q64_cmd_config)->operation_mode = QSPI_OPERATE_MODE_111;
  (&w25q64_cmd_config)->read_status_config = QSPI_RSTSC_SW_ONCE;
  (&w25q64_cmd_config)->read_status_enable = FALSE;
  (&w25q64_cmd_config)->write_data_enable = TRUE;

  qspi_cmd_operation_kick(QSPI1, &w25q64_cmd_config);

  while (qspi_flag_get(QSPI1, QSPI_CMDSTS_FLAG) == RESET)
    ;
  qspi_flag_clear(QSPI1, QSPI_CMDSTS_FLAG);

  // 等待写入完成
  flash_qspi_waitforwriteend();

  // 禁用写操作
  flash_qspi_writedisable();
}

/**
 * @brief  使用单个写入周期（Page WRITE 序列）向 FLASH 写入多个字节。
 * @note   写入的字节数不能超过 FLASH 页大小。
 * @param  pBuffer: 指向包含要写入 FLASH 数据的缓冲区的指针。
 * @param  WriteAddr: 要写入的 FLASH 内部地址。
 * @param  NumByteToWrite: 要写入 FLASH 的字节数，必须小于或等于 "sFLASH_PAGESIZE" 的值。
 * @retval 无返回值
 */
static void flash_qspi_writepage(uint8_t *pbuffer, uint32_t writeaddr, uint16_t numbytetowrite)
{
  /*!< 启用写入访问到 FLASH */
  flash_qspi_writeenable();
  
  QSPI_flash_mode();
  
  qspi_flash_write_xip_dma_set(writeaddr, pbuffer, numbytetowrite);

  /*!< 等待 FLASH 写入完成 */
  flash_qspi_waitforwriteend();
}

/**
 * @brief  向 FLASH 写入数据块。在此函数中，通过使用 Page WRITE 序列减少写入周期次数。
 * @param  pBuffer: 指向包含要写入 FLASH 数据的缓冲区的指针。
 * @param  WriteAddr: 要写入的 FLASH 内部地址。
 * @param  NumByteToWrite: 要写入 FLASH 的字节数。
 * @retval 无返回值
 */
void flash_qspi_writebuffer(uint8_t *pbuffer, uint32_t writeaddr, uint16_t numbytetowrite)
{
  uint16_t pager;
  
  pager = FLASH_PAGE_SIZE - (writeaddr % FLASH_PAGE_SIZE); // 计算页余量
  if (numbytetowrite <= pager)
  {
    pager = numbytetowrite; // 如果剩余要写入的字节数小于页余量，则调整 pager
  }

  while (1)
  {
    flash_qspi_writepage(pbuffer, writeaddr, pager); // 写入当前页的数据
    if (pager == numbytetowrite)                     // 如果所有数据已写完，则退出
      break;
    else
    {
      pbuffer += pager;                    // 更新缓冲区指针
      writeaddr += pager;                  // 更新写入地址
      numbytetowrite -= pager;             // 更新剩余字节数
      if (numbytetowrite > FLASH_PAGE_SIZE) // 如果剩余字节数大于页大小
        pager = FLASH_PAGE_SIZE;
      else
        pager = numbytetowrite;
    }
  }
}





