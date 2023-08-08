#include <esp32_smartdisplay.h>

// PJR - Needs to understand ST7789 inversion !

#ifdef GT911

#define GT911_I2C_SLAVE_ADDR 0x5D
#define GT911_MAX_CONTACTS 5

#define GT911_PRODUCT_ID1 0x8140
#define GT911_REG_COORD_ADDR 0x814E
#define GT911_TRACK_ID1 0x814F

#define GT911_PRODUCT_ID_LEN 4

struct __attribute__((packed)) GTPoint
{
  // 0x814F-0x8156, ... 0x8176 (5 points)
  uint8_t trackId;
  uint16_t x;
  uint16_t y;
  uint16_t area;
  uint8_t reserved;
};

#if !defined(TFT_ORIENTATION_PORTRAIT) && !defined(TFT_ORIENTATION_LANDSCAPE) && !defined(TFT_ORIENTATION_PORTRAIT_INV) && !defined(TFT_ORIENTATION_LANDSCAPE_INV)
#error Please define orientation: TFT_ORIENTATION_PORTRAIT, TFT_ORIENTATION_LANDSCAPE, TFT_ORIENTATION_PORTRAIT_INV or TFT_ORIENTATION_LANDSCAPE_INV
#endif

bool gt911_write_register(uint16_t reg, const uint8_t buf[], int len)
{
  i2c_gt911.beginTransmission(GT911_I2C_SLAVE_ADDR);
  if (!i2c_gt911.write(reg >> 8) || !i2c_gt911.write(reg & 0xFF))
    return false;

  auto sent = i2c_gt911.write(buf, len);
  i2c_gt911.endTransmission();
  return sent == len;
}

bool gt911_read_register(uint16_t reg, uint8_t buf[], int len)
{
  i2c_gt911.beginTransmission(GT911_I2C_SLAVE_ADDR);
  if (!i2c_gt911.write(reg >> 8) || !i2c_gt911.write(reg & 0xFF))
    return false;

  i2c_gt911.endTransmission(false);
  auto requested = i2c_gt911.requestFrom(GT911_I2C_SLAVE_ADDR, len);
  if (requested != len)
    return false;

  while (i2c_gt911.available() && len--)
    *buf++ = i2c_gt911.read();

  return len == 0;
}

int8_t gt911_num_points_available()
{
  uint8_t coord_addr;
  if (!gt911_read_register(GT911_REG_COORD_ADDR, &coord_addr, sizeof(coord_addr)))
  {
    log_e("Unable to read COORD_ADDR register");
    return 0;
  }

  if ((coord_addr & 0x80) && ((coord_addr & 0x0F) < GT911_MAX_CONTACTS))
  {
    uint8_t zero = 0;
    if (!gt911_write_register(GT911_REG_COORD_ADDR, &zero, sizeof(zero)))
    {
      log_e("Unable to reset COORD_ADDR register");
      return 0;
    }

    return coord_addr & 0x0F;
  }

  return 0;
}

bool gt911_read_touches(GTPoint *points, uint8_t numPoints = GT911_MAX_CONTACTS)
{
  if (!gt911_read_register(GT911_TRACK_ID1, (uint8_t *)points, sizeof(GTPoint) * numPoints))
  {
    log_e("Unable to read GTPoints");
    return false;
  }

  log_d("Raw Touch: (x=%d,y=%d)", points[0].x, points[0].y);


#ifdef TFT_ORIENTATION_PORTRAIT
  for (uint8_t i = 0; i < numPoints; ++i)
  {
    points[i].x = TFT_WIDTH - points[i].x;
    points[i].y = TFT_HEIGHT - points[i].y;
  }
#else
// Kludge ... assume we are in landscape mode ...
#if defined(TFT_ORIENTATION_LANDSCAPE) || defined (ESP32_2432S032C)
  uint16_t swap;
  for (uint8_t i = 0; i < numPoints; ++i)
  {
    swap = points[i].x;
    points[i].x = points[i].y;
    points[i].y = TFT_WIDTH - swap;
  }
#else
#ifdef TFT_ORIENTATION_PORTRAIT_INV
  for (uint8_t i = 0; i < numPoints; ++i)
  {
    points[i].x = points[i].x;
    points[i].y = points[i].y;
  }
#else
#ifdef TFT_ORIENTATION_LANDSCAPE_INV
  // PJR - this may need to depend on the actual device too?
  // Why is this different on my Sunton vs the author's ?
  // Need to make this compile time configurable
  // So it is actually TFT_ORIENTATION_LANDSCAPE that is needed ...
  // Something wrong with the Display driver?
  
  uint16_t swap;
  for (uint8_t i = 0; i < numPoints; ++i)
  {
    swap = points[i].x;
    points[i].x = TFT_HEIGHT - points[i].y;
    points[i].y = swap;
  }
#else
#error TFT_ORIENTATION not defined!
#endif
#endif
#endif
#endif
  return true;
}

void lvgl_touch_init()
{
  // i2c_gt911.begin(GT911_IIC_SDA , GT911_IIC_SCL );

  // // PJR - should RESET first ... ?
  // pinMode(GT911_IIC_INT, OUTPUT);
  // pinMode(GT911_IIC_RST, OUTPUT);
  // digitalWrite(GT911_IIC_INT, 0);
  // digitalWrite(GT911_IIC_RST, 0);
  // delay(10);
  // digitalWrite(GT911_IIC_INT, 1); // from I2C address ... addr==GT911_ADDR2);
  // delay(1);
  // digitalWrite(GT911_IIC_RST, 1);
  // delay(5);
  // digitalWrite(GT911_IIC_INT, 0);
  // delay(50);
  // pinMode(GT911_IIC_INT, INPUT);
  // // attachInterrupt(pinInt, TAMC_GT911::onInterrupt, RISING);
  // delay(50);

  uint8_t productId[GT911_PRODUCT_ID_LEN];
  if (!gt911_read_register(GT911_PRODUCT_ID1, productId, GT911_PRODUCT_ID_LEN))
  {
    log_e("No GT911 touch device found");
    return;
  }

  log_i("DeviceId: %s", productId);
}

void lvgl_touch_read(lv_indev_drv_t *drv, lv_indev_data_t *data)
{
  static int16_t last_x = 0, last_y = 0;
  // Ignore multi-touch
  auto points_available = gt911_num_points_available();
  if (points_available == 1)
  {
    log_d("Touches detected: %d", points_available);
    GTPoint point;
    if (gt911_read_touches(&point, 1))
    {
      log_d("Touch: (%d,%d)", point.x, point.y);
      data->state = LV_INDEV_STATE_PR;
      last_x = data->point.x = point.x;
      last_y = data->point.y = point.y;
    }
  }
  else
  {
    data->point.x = last_x;
    data->point.y = last_y;
    data->state = LV_INDEV_STATE_REL;
  }
}

#endif