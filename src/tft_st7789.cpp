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

void ST7789_send_init_commands()
{
  ST7789_send_command(CMD_SWRESET); // Software reset
  delay(100);

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