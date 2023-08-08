#include <esp32_smartdisplay.h>

#ifdef ST7789

#define CMD_SWRESET 0x01 // Software Reset
#define CMD_SLPIN 0x10   // Sleep in
#define CMD_SLPOUT 0x11  // Sleep out
#define CMD_NORON 0x13   // Normal Display Mode On
#define CMD_INVOFF 0x20  // Display Inversion Off
#define CMD_DISPON 0x29  // Display On
#define CMD_CASET 0x2A   // Column Address Set
#define CMD_RASET 0x2B   // Row Address Set
#define CMD_RAMWR 0x2C   // Memory Write
#define CMD_MADCTL 0x36  // Memory Data Access Control
#define CMD_COLMOD 0x3A  // Interface Pixel Format
#define CMD_PGC 0xE0     // Positive Gamma Control
#define CMD_NGC 0xE1     // Negative Gamma Control
// PJR - not in ST7789 ?
//#define CMD_CSCON 0xF0   // Command Set Control

// MADCTL Looks very different?
#define MADCTL_MY 0x80  // Row Address Order - 0=Increment (Top to Bottom), 1=Decrement (Bottom to Top)
#define MADCTL_MX 0x40  // Column Address Order - 0=Increment (Left to Right), 1=Decrement (Right to Left)
// PJR - ST7789 - Page/Column order ???
#define MADCTL_MV 0x20  // Row/Column exchange - 0=Normal, 1=Row/Column exchanged
#define MADCTL_ML 0x10  // Vertical Refresh Order
#define MADCTL_BGR 0x08 // RGB/BGR Order - BGR
#define MADCTL_MH 0x10  // Horizontal Refresh Order
#define MADCTL_RGB 0x00 // RGB/BGR Order - RGB

#define COLMOD_RGB_16BIT 0x50
#define COLMOD_CTRL_16BIT 0x05
#define COLMOD_RGB656 (COLMOD_RGB_16BIT | COLMOD_CTRL_16BIT)

#if !defined(TFT_ORIENTATION_PORTRAIT) && !defined(TFT_ORIENTATION_LANDSCAPE) && !defined(TFT_ORIENTATION_PORTRAIT_INV) && !defined(TFT_ORIENTATION_LANDSCAPE_INV)
#error Please define orientation: TFT_ORIENTATION_PORTRAIT, TFT_ORIENTATION_LANDSCAPE, TFT_ORIENTATION_PORTRAIT_INV or TFT_ORIENTATION_LANDSCAPE_INV
#endif

#if !defined(TFT_PANEL_ORDER_RGB) && !defined(TFT_PANEL_ORDER_BGR)
#error Please define RGB order: TFT_PANEL_ORDER_BGR or
#endif

#ifdef TFT_PANEL_ORDER_RGB
#define MADCTL_PANEL_ORDER MADCTL_RGB
#else
#ifdef TFT_PANEL_ORDER_BGR
#define MADCTL_PANEL_ORDER MADCTL_BGR
#else
#error TFT_PANEL_ORDER not defined!
#endif
#endif

void ST7789_send_command(const uint8_t command, const uint8_t data[] = nullptr, const ushort length = 0)
{
  digitalWrite(ST7789_PIN_DC, LOW); // Command mode => command
  spi_st7789.beginTransaction(SPISettings(ST7789_SPI_FREQ, MSBFIRST, SPI_MODE0));
  digitalWrite(ST7789_PIN_CS, LOW); // Chip select => enable
  spi_st7789.write(command);
  if (length > 0)
  {
    digitalWrite(ST7789_PIN_DC, HIGH); // Command mode => data
    spi_st7789.writeBytes(data, length);
  }
  digitalWrite(ST7789_PIN_CS, HIGH); // Chip select => disable
  spi_st7789.endTransaction();
}

void ST7789_send_pixels(const uint8_t command, const lv_color_t data[], const ushort length)
{
  digitalWrite(ST7789_PIN_DC, LOW); // Command mode => command
  spi_st7789.beginTransaction(SPISettings(ST7789_SPI_FREQ, MSBFIRST, SPI_MODE0));
  digitalWrite(ST7789_PIN_CS, LOW); // Chip select => enable
  spi_st7789.write(command);
  if (length > 0)
  {
    digitalWrite(ST7789_PIN_DC, HIGH); // Command mode => data
    spi_st7789.writePixels(data, sizeof(lv_color_t) * length);
  }
  digitalWrite(ST7789_PIN_CS, HIGH); // Chip select => disable
  spi_st7789.endTransaction();
}

#define CMD_RAMCTRL		0xB0  
#define CMD_PORCTRL		0xB2
#define CMD_GCTRL		0xB7
#define CMD_VCOMS		0xBB
#define CMD_LCMCTRL		0xC0
#define CMD_VDVVRHEN		0xC2
#define CMD_VRHS			0xC3
#define CMD_VDVSET		0xC4 
#define CMD_CASET		0x2A
#define CMD_FRCTR2		0xC6
#define CMD_PWCTRL1		0xD0
#define CMD_PVGAMCTRL	0xE0
#define CMD_NVGAMCTRL	0xE1
#define CMD_INVON		0x21


void ST7789_send_init_commands()
{
  ST7789_send_command(CMD_SWRESET); // Software reset
  delay(150); // Wait for reset to complete


// This is the command sequence that initialises the ST7789 driver
//
// This setup information uses simple 8 bit SPI ST7789_send_init_commands() and writedata() functions
//
// See ST7735_Setup.h file for an alternative format
  ST7789_send_command(CMD_SLPOUT);   // Sleep out
  delay(120);

  ST7789_send_command(CMD_NORON);    // Normal display mode on

  //------------------------------display and color format setting--------------------------------//

  static const uint8_t cmd_madctl[] = {0x00};
  ST7789_send_command(CMD_MADCTL, cmd_madctl, 1);

  //  JLX240 display datasheet
  // PJR ???
  //ST7789_send_init_commands(0xB6);
  //writedata(0x0A);
  //writedata(0x82);

static const uint8_t cmd_ramctrl[] = {0x00, 0xE0};
  ST7789_send_command(CMD_RAMCTRL, cmd_ramctrl , 2);
  //writedata(0x00);
  //writedata(0xE0); // 5 to 6 bit conversion: r0 = r5, b0 = b5

static const uint8_t cmd_colmod[] = {0x55};
  ST7789_send_command(CMD_COLMOD, cmd_colmod, 1);
  //writedata(0x55);
  delay(10);

  //--------------------------------ST7789V Frame rate setting----------------------------------//
  static const uint8_t cmd_porctl[] = {0x0c, 0x0c, 0x00, 0x33, 0x35};
  ST7789_send_command(CMD_PORCTRL, cmd_porctl, 5);
  // writedata(0x0c);
  // writedata(0x0c);
  // writedata(0x00);
  // writedata(0x33);
  // writedata(0x33);

static const uint8_t cmd_gctrl[] = {0x35};
  ST7789_send_command(CMD_GCTRL, cmd_gctrl, 1);      // Voltages: VGH / VGL
  //writedata(0x35);

  //---------------------------------ST7789V Power setting--------------------------------------//
  static uint8_t cmd_vcoms[] = {0x28};
  ST7789_send_command(CMD_VCOMS, cmd_vcoms, 1);
  //writedata(0x28);		// JLX240 display datasheet

static uint8_t cmd_lcmctrl[] = {0x0c};
  ST7789_send_command(CMD_LCMCTRL, cmd_lcmctrl , 1);
  //writedata(0x0C);

  static uint8_t cmd_vdvvrhen[] = {0x01, 0xFF};
  ST7789_send_command(CMD_VDVVRHEN, cmd_vdvvrhen , 2);
  // writedata(0x01);
  // writedata(0xFF);

static uint8_t cmd_vrhs[] = {0x10};

  ST7789_send_command(CMD_VRHS, cmd_vrhs, 1);       // voltage VRHS
  //writedata(0x10);

static uint8_t cmd_vdvset[] = {0x20};
  ST7789_send_command(CMD_VDVSET, cmd_vdvset , 1);
  //writedata(0x20);

static uint8_t cmd_frctr2[] = {0x0f};
  ST7789_send_command(CMD_FRCTR2, cmd_frctr2 , 1);
  //writedata(0x0f);

static uint8_t cmd_pwctrl1[] =  {0xa4, 0xa1};
  ST7789_send_command(CMD_PWCTRL1, cmd_pwctrl1, 2);
  // writedata(0xa4);
  // writedata(0xa1);

static uint8_t cmd_pvgamctrl[] = {0xd0, 0x00, 0x02, 0x07, 0x0a, 0x28, 0x32, 0x44, 0x42, 0x06, 0x12, 0x14, 0x17};
  ST7789_send_command(CMD_PVGAMCTRL, cmd_pvgamctrl , 14);
  //--------------------------------ST7789V gamma setting---------------------------------------//
  // writedata(0xd0);
  // writedata(0x00);
  // writedata(0x02);
  // writedata(0x07);
  // writedata(0x0a);
  // writedata(0x28);
  // writedata(0x32);
  // writedata(0x44);
  // writedata(0x42);
  // writedata(0x06);
  // writedata(0x0e);
  // writedata(0x12);
  // writedata(0x14);
  // writedata(0x17);

  static uint8_t cmd_nvgamctrl[] = {0xd0, 0x00, 0x02, 0x07, 0x0a, 0x28, 0x31, 0x54, 0x47, 0x0e, 0x1c, 0x17, 0x1b, 0x1e};
ST7789_send_command(CMD_NVGAMCTRL, cmd_nvgamctrl , 14);
  // writedata(0xd0);
  // writedata(0x00);
  // writedata(0x02);
  // writedata(0x07);
  // writedata(0x0a);
  // writedata(0x28);
  // writedata(0x31);
  // writedata(0x54);
  // writedata(0x47);
  // writedata(0x0e);
  // writedata(0x1c);
  // writedata(0x17);
  // writedata(0x1b);
  // writedata(0x1e);

  ST7789_send_command(CMD_INVON);

static uint8_t cmd_caset[] = {0x00, 0x00, 0x00, 0xef};
  ST7789_send_command(CMD_CASET, cmd_caset , 4);    // Column address set
  // writedata(0x00);
  // writedata(0x00);
  // writedata(0x00);
  // writedata(0xEF);    // 239

static uint8_t cmd_raset[] = {0x00, 0x00, 0x01, 0x3f};
  ST7789_send_command(CMD_RASET, cmd_raset , 4);    // Row address set
  // writedata(0x00);
  // writedata(0x00);
  // writedata(0x01);
  // writedata(0x3F);    // 319

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  delay(120);

  ST7789_send_command(CMD_DISPON);    //Display on
  delay(120);


#ifdef TFT_INVERSION_ON
  ST7789_send_command(TFT_INVON);
#endif

#ifdef TFT_INVERSION_OFF
  ST7789_send_command(TFT_INVOFF);
#endif

  //setRotation(rotation);

  // Needed ?
  ST7789_send_command(CMD_INVOFF); // Inversion off
  ST7789_send_command(CMD_NORON);  // Normal display on
  ST7789_send_command(CMD_SLPOUT); // Out of sleep mode
  ST7789_send_command(CMD_DISPON); // Main screen turn on

}


void XST7789_send_init_commands()
{
  ST7789_send_command(CMD_SWRESET); // Software reset
  delay(150); // It's what TFT_eSPI does ?

  // PJR - no CMD_CSCON in ST7789
  // static const uint8_t cscon1[] = {0xC3}; // Enable extension command 2 part I
  // ST7789_send_command(CMD_CSCON, cscon1, sizeof(cscon1));
  // static const uint8_t cscon2[] = {0x96}; // Enable extension command 2 part II
  // ST7789_send_command(CMD_CSCON, cscon2, sizeof(cscon2));

  static const uint8_t colmod[] = {COLMOD_RGB656};         // 16 bits R5G6B5
  ST7789_send_command(CMD_COLMOD, colmod, sizeof(colmod)); // Set color mode

#ifdef TFT_ORIENTATION_PORTRAIT
  static const uint8_t madctl[] = {MADCTL_MY | MADCTL_PANEL_ORDER}; // Portrait 0 Degrees
#else
#ifdef TFT_ORIENTATION_LANDSCAPE
  static const uint8_t madctl[] = {MADCTL_MV | MADCTL_PANEL_ORDER}; // Landscape 90 Degrees
#else
#ifdef TFT_ORIENTATION_PORTRAIT_INV
  static const uint8_t madctl[] = {MADCTL_MX | MADCTL_PANEL_ORDER}; // Portrait inverted 180 Degrees
#else
#ifdef TFT_ORIENTATION_LANDSCAPE_INV
//  static const uint8_t madctl[] = {MADCTL_MY | MADCTL_MX | MADCTL_MV | MADCTL_PANEL_ORDER}; // Landscape inverted 270 Degrees
// PJR - to make this work remove MADCTL_MY !!!
// Need to adjust touch to suit ...
  static const uint8_t madctl[] = {MADCTL_MX | MADCTL_MV | MADCTL_PANEL_ORDER}; // Landscape inverted 270 Degrees
#else
#error TFT_ORIENTATION not defined!
#endif
#endif
#endif
#endif
  ST7789_send_command(CMD_MADCTL, madctl, sizeof(madctl));

  static const uint8_t pgc[] = {0xF0, 0x09, 0x0B, 0x06, 0x04, 0x15, 0x2F, 0x54, 0x42, 0x3C, 0x17, 0x14, 0x18, 0x1B};
  ST7789_send_command(CMD_PGC, pgc, sizeof(pgc));
  static const uint8_t ngc[] = {0xE0, 0x09, 0x0B, 0x06, 0x04, 0x03, 0x2B, 0x43, 0x42, 0x3B, 0x16, 0x14, 0x17, 0x1B};
  ST7789_send_command(CMD_NGC, ngc, sizeof(ngc));

  // static const uint8_t cscon3[] = {0x3C}; // Disable extension command 2 part I
  // ST7789_send_command(CMD_CSCON, cscon3, sizeof(cscon3));
  // static const uint8_t cscon4[] = {0x69}; // Disable extension command 2 part II
  // ST7789_send_command(CMD_CSCON, cscon4, sizeof(cscon4));

  ST7789_send_command(CMD_INVOFF); // Inversion off
  ST7789_send_command(CMD_NORON);  // Normal display on
  ST7789_send_command(CMD_SLPOUT); // Out of sleep mode
  ST7789_send_command(CMD_DISPON); // Main screen turn on
}

void lvgl_tft_init()
{
  pinMode(ST7789_PIN_DC, OUTPUT); // Data or Command
  pinMode(ST7789_PIN_CS, OUTPUT); // Chip Select
  digitalWrite(ST7789_PIN_CS, HIGH);

  pinMode(ST7789_PIN_BL, OUTPUT); // Backlight
  ledcSetup(ST7789_PWM_CHANNEL_BL, ST7789_PWM_FREQ_BL, ST7789_PWM_BITS_BL);
  ledcAttachPin(ST7789_PIN_BL, ST7789_PWM_CHANNEL_BL);

  ST7789_send_init_commands();

  smartdisplay_tft_set_backlight(ST7789_PWM_MAX_BL); // Backlight on
}

void lvgl_tft_flush(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map)
{
  // Column addresses
  const uint8_t caset[] = {
      static_cast<uint8_t>(area->x1 >> 8),
      static_cast<uint8_t>(area->x1),
      static_cast<uint8_t>(area->x2 >> 8),
      static_cast<uint8_t>(area->x2)};
  ST7789_send_command(CMD_CASET, caset, sizeof(caset));
  // Page addresses
  const uint8_t raset[] = {
      static_cast<uint8_t>(area->y1 >> 8),
      static_cast<uint8_t>(area->y1),
      static_cast<uint8_t>(area->y2 >> 8),
      static_cast<uint8_t>(area->y2)};
  ST7789_send_command(CMD_RASET, raset, sizeof(raset));
  // Memory write
  const auto size = lv_area_get_width(area) * lv_area_get_height(area);
  ST7789_send_pixels(CMD_RAMWR, color_map, size);
  lv_disp_flush_ready(drv);
}

void smartdisplay_tft_set_backlight(uint16_t duty)
{
  ledcWrite(ST7789_PWM_CHANNEL_BL, duty);
}

void smartdisplay_tft_sleep()
{
  static const uint8_t slpin[] = {0x08};
  ST7789_send_command(CMD_SLPIN, slpin, sizeof(slpin));
}

void smartdisplay_tft_wake()
{
  static const uint8_t splout[] = {0x08};
  ST7789_send_command(CMD_SLPOUT, splout, sizeof(splout));
}

#endif
