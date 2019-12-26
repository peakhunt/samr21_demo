#include "driver_init.h"
#include "app_common.h"
#include "at86rf233.h"

#define AT86RF233_CMD_REG_READ(_r)     (0x80 | (0x3f & _r))
#define AT86RF233_CMD_REG_WRITE(_r)    (0xc0 | (0x3f & _r))

#define AT86RF233_CMD_FB_READ           0x20
#define AT86RF233_CMD_FB_WRITE          0x60
#define AT86RF233_CMD_SRAM_READ         0x00
#define AT86RF233_CMD_SRAM_WRITE        0x40

static uint8_t _spi_rx_buf[16];
static uint8_t _spi_tx_buf[16];
static struct spi_xfer _xfer = 
{
  .txbuf = _spi_tx_buf,
  .rxbuf = _spi_rx_buf,
  .size = 0,
};

static void
at86rf233_cs(uint8_t sel)
{
  if(sel)
  {
    // select
    gpio_set_pin_level(RF_SEL, false);
  }
  else
  {
    // unselect
    gpio_set_pin_level(RF_SEL, true);
  }
}

uint8_t
at86rf233_read_reg(uint8_t reg, uint8_t data[2])
{
  //
  // MOSI     | cmd(1)        | XX(1)   |
  // MISO     | PHY STATUS(1) | DATA(1) |
  //
  int32_t rc;
  _xfer.size = 2;

  _xfer.txbuf[0] = AT86RF233_CMD_REG_READ(reg);
  _xfer.txbuf[1] = 0xff;

  at86rf233_cs(true);

  rc = spi_m_sync_transfer(&RF_SPI, &_xfer);

  at86rf233_cs(false);

  data[0] = _xfer.rxbuf[0];
  data[1] = _xfer.rxbuf[1];

  if(rc != 2)
  {
    return false;
  }

  return true;
}

uint8_t
at86rf233_write_reg(uint8_t reg, uint8_t data)
{
  //
  // MOSI     | cmd(1)        | data(1) |
  // MISO     | PHY STATUS(1) | XX(1)   |
  //
  _xfer.size = 2;

  _xfer.txbuf[0] = AT86RF233_CMD_REG_WRITE(reg);
  _xfer.txbuf[1] = data;

  at86rf233_cs(true);

  spi_m_sync_transfer(&RF_SPI, &_xfer);

  at86rf233_cs(false);

  return true;
}

uint8_t
at86rf233_read_frame(at86rf233_frame_t* frame)
{
  // FIXME
  // returns PHY STATUS
  return 0;
}

uint8_t
at86rf233_write_frame(at86rf233_frame_t* frame)
{
  // FIXME
  // returns PHY STATUS
  return 0;
}

void
at86rf233_read_sram(uint8_t addr, uint8_t len, uint8_t* buf)
{
  // FIXME
}

void
at86rf233_write_sram(uint8_t addr, uint8_t len, uint8_t* buf)
{
  // FIXME
}

void
at86rf233_reset(void)
{
  gpio_set_pin_level(RF_RSTN, true);
  app_delay_ms(100);
  gpio_set_pin_level(RF_RSTN, false);
  app_delay_ms(100);
  gpio_set_pin_level(RF_RSTN, true);
  app_delay_ms(100);
}

void
at86rf233_init(void)
{
  //
  // enable SPI synchronous master
  //
  at86rf233_cs(false);    // chip select high
  spi_m_sync_enable(&RF_SPI);

  // reset AT86RF233
  at86rf233_reset();

  gpio_set_pin_level(RF_SLP_TR, true);
}
