// Config for the blue boards:
// front: 1.77' TFT 160(RGB)x128
// back:  1.77' TFT and two rows of pins; 8..1 and 9..11
//
// https://www.arthurwiz.com/software-development/177-inch-tft-lcd-display-with-st7735s-on-arduino-mega-2560
// https://www.hackster.io/ocnarf/cheap-1-77-inch-tft-screen-on-esp32-ebe208

#define ST7735_DRIVER     

#define TFT_WIDTH  128
#define TFT_HEIGHT 160

#define ST7735_BLACKTAB
#define TFT_RGB_ORDER TFT_RGB  
#define TFT_INVERSION_OFF

#define TFT_MISO 19  // not broken out on this board.
#define TFT_SCLK 18  // TX2
#define TFT_MOSI 23  // RX2
#define TFT_RST   4  // D4
#define TFT_DC    2  // D2
#define TFT_CS   15  //D15
                     // requires pin 8 (LED ANODE) to be wired to 3V3 or a digital out.

#define TFT_BACKLIGHT_ON HIGH 

#define LOAD_GLCD   // Font 1. Original Adafruit 8 pixel font needs ~1820 bytes in FLASH
#define LOAD_FONT2  // Font 2. Small 16 pixel high font, needs ~3534 bytes in FLASH, 96 characters
#define LOAD_FONT4  // Font 4. Medium 26 pixel high font, needs ~5848 bytes in FLASH, 96 characters
#define LOAD_FONT6  // Font 6. Large 48 pixel font, needs ~2666 bytes in FLASH, only characters 1234567890:-.apm
#define LOAD_FONT7  // Font 7. 7 segment 48 pixel font, needs ~2438 bytes in FLASH, only characters 1234567890:.
#define LOAD_FONT8  // Font 8. Large 75 pixel font needs ~3256 bytes in FLASH, only characters 1234567890:-.
#define LOAD_FONT8N // Font 8. Alternative to Font 8 above, slightly narrower, so 3 digits fit a 160 pixel TFT
#define LOAD_GFXFF  // FreeFonts. Include access to the 48 Adafruit_GFX free fonts FF1 to FF48 and custom fonts

#define SMOOTH_FONT

#define SPI_FREQUENCY  27000000
#define SPI_READ_FREQUENCY  20000000
#define SPI_TOUCH_FREQUENCY  2500000
//#endif