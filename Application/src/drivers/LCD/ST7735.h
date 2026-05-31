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

#define CONF_MASTER_SPI_MODULE      SERCOM5
#define CONF_MASTER_MUX_SETTING     SPI_SIGNAL_MUX_SETTING_C   // PAD0=MOSI, PAD1=SCK
#define CONF_MASTER_PINMUX_PAD0     PINMUX_PB02D_SERCOM5_PAD0  // MOSI
#define CONF_MASTER_PINMUX_PAD1     PINMUX_PB03D_SERCOM5_PAD1  // SCK
#define CONF_MASTER_PINMUX_PAD2     PINMUX_UNUSED
#define CONF_MASTER_PINMUX_PAD3     PINMUX_UNUSED
#define CONF_MASTER_SS_PIN          PIN_PA02                   // CS
#define LCD_DC						PIN_PA03
#define LCD_LITE					PIN_PA11                     //PCBA PA06
#define LCD_RST						PIN_PA10


#define ST7735_NOP     0x00
#define ST7735_SWRESET 0x01
#define ST7735_RDDID   0x04
#define ST7735_RDDST   0x09
#define ST7735_SLPIN   0x10
#define ST7735_SLPOUT  0x11
#define ST7735_PTLON   0x12
#define ST7735_NORON   0x13
#define ST7735_INVOFF  0x20
#define ST7735_INVON   0x21
#define ST7735_DISPOFF 0x28
#define ST7735_DISPON  0x29
#define ST7735_CASET   0x2A
#define ST7735_RASET   0x2B
#define ST7735_RAMWR   0x2C
#define ST7735_RAMRD   0x2E
#define ST7735_PTLAR   0x30
#define ST7735_COLMOD  0x3A
#define ST7735_MADCTL  0x36
#define ST7735_FRMCTR1 0xB1
#define ST7735_FRMCTR2 0xB2
#define ST7735_FRMCTR3 0xB3
#define ST7735_INVCTR  0xB4
#define ST7735_DISSET5 0xB6
#define ST7735_PWCTR1  0xC0
#define ST7735_PWCTR2  0xC1
#define ST7735_PWCTR3  0xC2
#define ST7735_PWCTR4  0xC3
#define ST7735_PWCTR5  0xC4
#define ST7735_VMCTR1  0xC5
#define ST7735_RDID1   0xDA
#define ST7735_RDID2   0xDB
#define ST7735_RDID3   0xDC
#define ST7735_RDID4   0xDD
#define ST7735_PWCTR6  0xFC
#define ST7735_GMCTRP1 0xE0
#define ST7735_GMCTRN1 0xE1

#define MADCTL_MY  0x80
#define MADCTL_MX  0x40
#define MADCTL_MV  0x20
#define MADCTL_ML  0x10
#define MADCTL_RGB 0x00
#define MADCTL_BGR 0x08
#define MADCTL_MH  0x04


void ST7735_WriteBuffer(uint8_t *data, uint32_t len);
void lcd_init(void);
void sendCommands(const uint8_t *cmds, size_t length);
void LCD_setAddr(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1);
