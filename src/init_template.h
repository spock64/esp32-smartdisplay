// INIT Sequence from tft_espi ...


void ST7789_alt_init()
{
  ST7789_Send_command(TFT_SWRST); // Software reset
  delay(150); // Wait for reset to complete


// This is the command sequence that initialises the ST7789 driver
//
// This setup information uses simple 8 bit SPI ST7789_Send_command() and writedata() functions
//
// See ST7735_Setup.h file for an alternative format
  ST7789_Send_command(ST7789_SLPOUT);   // Sleep out
  delay(120);

  ST7789_Send_command(ST7789_NORON);    // Normal display mode on

  //------------------------------display and color format setting--------------------------------//
  ST7789_Send_command(ST7789_MADCTL, [TFT_MAD_COLOR_ORDER], 1);

  //  JLX240 display datasheet
  // PJR ???
  //ST7789_Send_command(0xB6);
  //writedata(0x0A);
  //writedata(0x82);

  ST7789_Send_command(ST7789_RAMCTRL, [0x00, 0xE0], 2);
  //writedata(0x00);
  //writedata(0xE0); // 5 to 6 bit conversion: r0 = r5, b0 = b5

  ST7789_Send_command(ST7789_COLMOD, [0x55], 1);
  //writedata(0x55);
  delay(10);

  //--------------------------------ST7789V Frame rate setting----------------------------------//
  ST7789_Send_command(ST7789_PORCTRL, [0x0c, 0x0c, 0x00, 0x33, 0x35], 5);
  // writedata(0x0c);
  // writedata(0x0c);
  // writedata(0x00);
  // writedata(0x33);
  // writedata(0x33);

  ST7789_Send_command(ST7789_GCTRL, [0x35], 1);      // Voltages: VGH / VGL
  //writedata(0x35);

  //---------------------------------ST7789V Power setting--------------------------------------//
  ST7789_Send_command(ST7789_VCOMS, [0x28], 1);
  //writedata(0x28);		// JLX240 display datasheet

  ST7789_Send_command(ST7789_LCMCTRL, [0x0c], 1);
  //writedata(0x0C);

  ST7789_Send_command(ST7789_VDVVRHEN, [0x01, 0xFF], 2);
  // writedata(0x01);
  // writedata(0xFF);

  ST7789_Send_command(ST7789_VRHS, [0x10], 1);       // voltage VRHS
  //writedata(0x10);

  ST7789_Send_command(ST7789_VDVSET, [0x20], 1);
  //writedata(0x20);

  ST7789_Send_command(ST7789_FRCTR2, [0x0f], 1);
  //writedata(0x0f);

  ST7789_Send_command(ST7789_PWCTRL1, [0xa4, 0xa1], 2);
  // writedata(0xa4);
  // writedata(0xa1);

  //--------------------------------ST7789V gamma setting---------------------------------------//
  ST7789_Send_command(ST7789_PVGAMCTRL, [0xd0, 0x00, 0x02, 0x07, 0x0a, 0x28, 0x32, 0x44, 0x42, 0x06, 0x12, 0x14, 0x17], 14);
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

  ST7789_Send_command(ST7789_NVGAMCTRL, [0xd0, 0x00, 0x02, 0x07, 0x0a, 0x28, 0x31, 0x54, 0x47, 0x0e, 0x1c, 0x17, 0x1b, 0x1e], 14);
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

  ST7789_Send_command(ST7789_INVON);

  ST7789_Send_command(ST7789_CASET, [0x00, 0x00, 0x00, 0xef], 4);    // Column address set
  // writedata(0x00);
  // writedata(0x00);
  // writedata(0x00);
  // writedata(0xEF);    // 239

  ST7789_Send_command(ST7789_RASET, [0x00, 0x00, 0x01, 0x3f]);    // Row address set
  // writedata(0x00);
  // writedata(0x00);
  // writedata(0x01);
  // writedata(0x3F);    // 319

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  delay(120);

  ST7789_Send_command(ST7789_DISPON);    //Display on
  delay(120);


#ifdef TFT_INVERSION_ON
  ST7789_Send_command(TFT_INVON);
#endif

#ifdef TFT_INVERSION_OFF
  ST7789_Send_command(TFT_INVOFF);
#endif

  //setRotation(rotation);

}


