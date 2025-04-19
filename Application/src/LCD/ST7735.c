/**************************************************************************//**
 * @file      main.c
 * @brief     Main application entry point for Smart Secure Access System
 * @author    Tony Yan & Yue Zhang
 * @date      2025-04-06
 ******************************************************************************/

/******************************************************************************
 * Includes
 ******************************************************************************/
 #include <errno.h>

 #include "CliThread/CliThread.h"
 #include "FreeRTOS.h"
 #include "I2cDriver\I2cDriver.h"
 #include "SerialConsole.h"
 #include "WifiHandlerThread/WifiHandler.h"
 #include "asf.h"
 #include "driver/include/m2m_wifi.h"
 #include "main.h"
 #include "stdio_serial.h"
 #include "SerialConsole/SerialConsole.h"
 #include "imu_driver/adxl345_imu.h"
#include "MCHP_ATWx.h"

#include "ST7735.h"


#define TEST_SPI_BAUDRATE 1000000UL
#define SLAVE_SELECT_PIN CONF_MASTER_SS_PIN

#define BUF_LENGTH 22
//
uint8_t buffer_tx[BUF_LENGTH] = {
	////0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A,
	////0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14,
};

struct spi_module spi_master_instance;
volatile bool transfer_tx_is_done = false;
struct spi_slave_inst slave;
struct dma_resource example_resource_tx;

COMPILER_ALIGNED(16)
DmacDescriptor example_descriptor_tx;

#define CONF_PERIPHERAL_TRIGGER_TX   SERCOM5_DMAC_ID_TX

struct dma_resource example_resource_tx;
struct spi_config config_spi_master;
DmacDescriptor example_descriptor_tx SECTION_DMAC_DESCRIPTOR;

static void configure_dma_resource_tx(struct dma_resource *tx_resource)
{
	struct dma_resource_config tx_config;
	dma_get_config_defaults(&tx_config);
	tx_config.peripheral_trigger = CONF_PERIPHERAL_TRIGGER_TX;
	tx_config.trigger_action = DMA_TRIGGER_ACTION_BEAT;
	dma_allocate(tx_resource, &tx_config);
}


static void setup_transfer_descriptor_tx(DmacDescriptor *tx_descriptor, uint32_t len)
{	
	struct dma_descriptor_config tx_descriptor_config;
	dma_descriptor_get_config_defaults(&tx_descriptor_config);
	tx_descriptor_config.beat_size = DMA_BEAT_SIZE_BYTE;
	tx_descriptor_config.dst_increment_enable = false;
	tx_descriptor_config.block_transfer_count = len;
	tx_descriptor_config.source_address = (uint32_t)&buffer_tx[len - 1] + 1;
	//tx_descriptor_config.source_address = (uint32_t)&dma_tx_buffer[len - 1] + 1;
	tx_descriptor_config.destination_address = (uint32_t)&spi_master_instance.hw->SPI.DATA.reg;
	dma_descriptor_create(tx_descriptor, &tx_descriptor_config);
}

static void transfer_tx_done(struct dma_resource* const resource )
{
	transfer_tx_is_done = true;
}

static void configure_spi_master(void)
{
	struct spi_config config_spi_master;
	struct spi_slave_inst_config slave_dev_config;
	/* Configure and initialize software device instance of peripheral slave */
	spi_slave_inst_get_config_defaults(&slave_dev_config);
	slave_dev_config.ss_pin = SLAVE_SELECT_PIN;
	spi_attach_slave(&slave, &slave_dev_config);
	/* Configure, initialize and enable SERCOM SPI module */
	spi_get_config_defaults(&config_spi_master);
	config_spi_master.mode_specific.master.baudrate = TEST_SPI_BAUDRATE;
	config_spi_master.mux_setting = CONF_MASTER_MUX_SETTING;
	config_spi_master.pinmux_pad0 = CONF_MASTER_PINMUX_PAD0;
	config_spi_master.pinmux_pad1 = CONF_MASTER_PINMUX_PAD1;
	config_spi_master.pinmux_pad2 = CONF_MASTER_PINMUX_PAD2;
	config_spi_master.pinmux_pad3 = CONF_MASTER_PINMUX_PAD3;
	spi_init(&spi_master_instance, CONF_MASTER_SPI_MODULE, &config_spi_master);
	spi_enable(&spi_master_instance);
}

void lcd_cs_init(void)
{
	struct port_config cs_pin_conf;
	port_get_config_defaults(&cs_pin_conf);
	cs_pin_conf.direction = PORT_PIN_DIR_OUTPUT;
	port_pin_set_config(CONF_MASTER_SS_PIN, &cs_pin_conf);
	port_pin_set_output_level(CONF_MASTER_SS_PIN, true);
}

static void lcd_pin_init(void){
	
	struct port_config pin_conf;
	port_get_config_defaults(&pin_conf);
	pin_conf.direction = PORT_PIN_DIR_OUTPUT;
	
	port_pin_set_config(LCD_RST, &pin_conf);
	port_pin_set_config(LCD_DC, &pin_conf);

	port_pin_set_output_level(LCD_LITE, true);
}

static uint8_t dma_tx_buffer[320];  
void ST7735_WriteBuffer(uint8_t *data, uint32_t len)
{
	memcpy(dma_tx_buffer, data, len);  

	struct dma_descriptor_config tx_descriptor_config;
	dma_descriptor_get_config_defaults(&tx_descriptor_config);

	tx_descriptor_config.beat_size = DMA_BEAT_SIZE_BYTE;
	tx_descriptor_config.src_increment_enable = true;
	tx_descriptor_config.dst_increment_enable = false;
	tx_descriptor_config.block_transfer_count = len;
	//tx_descriptor_config.source_address = (uint32_t)(dma_tx_buffer + len);
	tx_descriptor_config.source_address = (uint32_t)(dma_tx_buffer + len);
	tx_descriptor_config.destination_address = (uint32_t)&spi_master_instance.hw->SPI.DATA.reg;

	dma_descriptor_create(&example_descriptor_tx, &tx_descriptor_config);
	dma_add_descriptor(&example_resource_tx, &example_descriptor_tx);
	example_resource_tx.descriptor = &example_descriptor_tx;

	transfer_tx_is_done = false;
	spi_select_slave(&spi_master_instance, &slave, true);
	dma_start_transfer_job(&example_resource_tx);
	while (!transfer_tx_is_done);
	
	while (!(spi_master_instance.hw->SPI.INTFLAG.reg & SERCOM_SPI_INTFLAG_TXC));
	spi_select_slave(&spi_master_instance, &slave, false);
	
	dma_abort_job(&example_resource_tx);     
	example_resource_tx.descriptor = NULL;     
	
	//SerialConsoleWriteString("DMA done\n"); 
}


void sendCommands(const uint8_t *cmds, size_t length)
{
	size_t i = 0;

	while (i < length)
	{
		
		uint8_t cmd = cmds[i++];
		uint8_t numData = cmds[i++];
		const uint8_t *data = &cmds[i];
		i += numData;
		uint8_t delay = cmds[i++];

		//send command
		port_pin_set_output_level(LCD_DC, false);         // DC = 0 ¡ú Command
		transfer_tx_is_done = false;
		ST7735_WriteBuffer(&cmd, 1);
		while (!transfer_tx_is_done);

		//sent data
		if (numData > 0) {
			port_pin_set_output_level(LCD_DC, true);      // DC = 1 ¡ú Data
			transfer_tx_is_done = false;
			ST7735_WriteBuffer(data, numData);
			while (!transfer_tx_is_done);
		}

		//delay
		if (delay > 0)
		delay_cycles_ms(delay == 255 ? 500 : delay);
	}
}

void lcd_init(void){
	
	lcd_pin_init();
	
	port_pin_set_output_level(LCD_RST, false);
	delay_cycles_ms(20);
	port_pin_set_output_level(LCD_RST, true);
	delay_cycles_ms(120);
	
	configure_spi_master();
	configure_dma_resource_tx(&example_resource_tx);
	dma_register_callback(&example_resource_tx, transfer_tx_done, DMA_CALLBACK_TRANSFER_DONE);
	dma_enable_callback(&example_resource_tx, DMA_CALLBACK_TRANSFER_DONE);
	
	delay_cycles_ms(5); 
	
	
	static const uint8_t ST7735_cmds[]  =
	{
		ST7735_SWRESET, 0, 150,
		ST7735_SLPOUT, 0, 255,
		ST7735_FRMCTR1, 3, 0x01, 0x2C, 0x2D, 0,
		ST7735_FRMCTR2, 3, 0x01, 0x2C, 0x2D, 0,
		ST7735_FRMCTR3, 6, 0x01, 0x2C, 0x2D, 0x01, 0x2C, 0x2D, 0,
		ST7735_INVCTR, 1, 0x07, 0,
		ST7735_PWCTR1, 3, 0x0A, 0x02, 0x84, 5,
		ST7735_PWCTR2, 1, 0xC5, 5,
		ST7735_PWCTR3, 2, 0x0A, 0x00, 5,
		ST7735_PWCTR4, 2, 0x8A, 0x2A, 5,
		ST7735_PWCTR5, 2, 0x8A, 0xEE, 5,
		ST7735_VMCTR1, 1, 0x0E, 0,
		ST7735_INVOFF, 0, 0,
		ST7735_MADCTL, 1, 0xC8, 0,
		ST7735_COLMOD, 1, 0x05, 0,
		ST7735_CASET, 4, 0x00, 0x00, 0x00, 0x7F, 0,
		ST7735_RASET, 4, 0x00, 0x00, 0x00, 0x9F, 0,
		ST7735_GMCTRP1, 16, 0x02, 0x1C, 0x07, 0x12, 0x37, 0x32, 0x29, 0x2D,
		0x29, 0x25, 0x2B, 0x39, 0x00, 0x01, 0x03, 0x10, 0,
		ST7735_GMCTRN1, 16, 0x03, 0x1D, 0x07, 0x06, 0x2E, 0x2C, 0x29,0x2D,
		0x2E, 0x2E, 0x37, 0x3F, 0x00, 0x00, 0x02, 0x10, 0,
		ST7735_NORON, 0, 10,
		ST7735_DISPON, 0, 100,
		//ST7735_MADCTL, 1, MADCTL_MX | MADCTL_MV | MADCTL_RGB, 10
		ST7735_MADCTL, 1, MADCTL_MV | MADCTL_MY | MADCTL_RGB, 10
	};

	sendCommands(ST7735_cmds, sizeof(ST7735_cmds));
	
}

void LCD_setAddr(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1)
{
	uint8_t ST7735_cmds[]  =
	{
		ST7735_CASET, 4, 0x00, x0, 0x00, x1, 0,		// Column
		ST7735_RASET, 4, 0x00, y0, 0x00, y1, 0,		// Page
		ST7735_RAMWR, 0, 5				// Into RAM
	};
	sendCommands(ST7735_cmds, sizeof(ST7735_cmds));
}


