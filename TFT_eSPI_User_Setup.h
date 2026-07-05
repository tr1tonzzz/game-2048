// Copy these settings into the TFT_eSPI library User_Setup.h file
// when using Arduino IDE with ESP32 + ILI9341 TFT + XPT2046 touch.

#define USER_SETUP_LOADED 1

#define ILI9341_DRIVER 1
#define TFT_WIDTH 240
#define TFT_HEIGHT 320

#define TFT_MISO 19
#define TFT_MOSI 23
#define TFT_SCLK 18
#define TFT_CS   5
#define TFT_DC   2
#define TFT_RST  4

#define TOUCH_CS  15
#define TOUCH_IRQ 27

#define LOAD_GLCD 1
#define LOAD_FONT2 1
#define LOAD_FONT4 1
#define SMOOTH_FONT 1

#define SUPPORT_TRANSACTIONS 1
#define SPI_FREQUENCY 40000000
#define SPI_READ_FREQUENCY 20000000
#define SPI_TOUCH_FREQUENCY 2500000
