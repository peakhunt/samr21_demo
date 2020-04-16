//
// ToDo
// a) IRQ disable during SPI transaction. Is it really necessary?
// b) hookup IRQ handler
//
#include <stdbool.h>
#include "driver_init.h"
#include "app_common.h"
#include "phy.h"
#include "at86rf233.h"
#include "event_list.h"
#include "event_dispatcher.h"

#define PHY_CRC_SIZE    2

#define AT86RF233_CMD_REG_READ(_r)      (0x80 | (0x3f & _r))
#define AT86RF233_CMD_REG_WRITE(_r)     (0xc0 | (0x3f & _r))
#define TRX_CMD_FW                      (0x60)
#define TRX_CMD_FR                      (0x20)

static uint8_t _spi_rx_buf[16];
static uint8_t _spi_tx_buf[16];
static struct spi_xfer _xfer = 
{
  .txbuf = _spi_tx_buf,
  .rxbuf = _spi_rx_buf,
  .size = 0,
};

typedef enum
{
  PHY_STATE_INITIAL,
  PHY_STATE_IDLE,
  PHY_STATE_SLEEP,
  PHY_STATE_TX_WAIT_END,
} PhyState_t;

static void phyWriteRegister(uint8_t reg, uint8_t value);
static void phyWaitState(uint8_t state);
static void phyTrxSetState(uint8_t state);
static void phySetRxState(void);

static PhyState_t phyState = PHY_STATE_INITIAL;
static uint8_t phyRxBuffer[128];
static bool phyRxState;

#define RST_HIGH()      gpio_set_pin_level(RF_RSTN, true)
#define RST_LOW()       gpio_set_pin_level(RF_RSTN, false)

#define SLP_TR_LOW()    gpio_set_pin_level(RF_SLP_TR, false)
#define SLP_TR_HIGH()    gpio_set_pin_level(RF_SLP_TR, true)

#define TRX_SLP_TR_LOW()      SLP_TR_LOW()
#define TRX_SLP_TR_HIGH()     SLP_TR_HIGH()
#define TRX_TRIG_DELAY()  { __NOP(); __NOP(); }

//
// FIXME IRQ disable/enable???
//
#define PHY_DISABLE_IRQ()
#define PHY_ENABLE_IRQ()

static void
__at86rf233_irq_handler(void)
{
  event_set(1 << DISPATCH_EVENT_RF_IRQ);
}

static void
at86rf233_irq_handler(uint32_t event)
{
  PHY_TaskHandler();
}

static void
at86rf233_cs(uint8_t sel)
{
  if(sel)
  {
    // select : LOW
    gpio_set_pin_level(RF_SEL, false);
  }
  else
  {
    // unselect : HIGH
    gpio_set_pin_level(RF_SEL, true);
  }
}

static void
trx_spi_init(void)
{
  at86rf233_cs(false);    // chip select high
  spi_m_sync_enable(&RF_SPI);
}

static void
PhyReset(void)
{
  RST_HIGH();
  SLP_TR_LOW();

  delay_us(330);

  RST_LOW();
  delay_us(10);
  RST_HIGH();
}

static void
trx_reg_write(uint8_t addr, uint8_t data)
{
  PHY_DISABLE_IRQ();

  //
  // MOSI     | cmd(1)        | data(1) |
  // MISO     | PHY STATUS(1) | XX(1)   |
  //
  _xfer.size = 2;

  _xfer.txbuf[0] = AT86RF233_CMD_REG_WRITE(addr);
  _xfer.txbuf[1] = data;

  at86rf233_cs(true);

  spi_m_sync_transfer(&RF_SPI, &_xfer);

  at86rf233_cs(false);

  PHY_ENABLE_IRQ();
}

static uint8_t
trx_reg_read(uint8_t addr)
{
  PHY_DISABLE_IRQ();

  //
  // MOSI     | cmd(1)        | XX(1)   |
  // MISO     | PHY STATUS(1) | DATA(1) |
  //
  _xfer.size = 2;

  _xfer.txbuf[0] = AT86RF233_CMD_REG_READ(addr);
  _xfer.txbuf[1] = 0xff;

  at86rf233_cs(true);

  spi_m_sync_transfer(&RF_SPI, &_xfer);

  at86rf233_cs(false);

  // data[0] = _xfer.rxbuf[0];
  // data[1] = _xfer.rxbuf[1];

  PHY_ENABLE_IRQ();

  return _xfer.rxbuf[1];
}

static void
trx_frame_read(uint8_t* buf, uint8_t size)
{
  PHY_DISABLE_IRQ();

  struct spi_xfer xfer = 
  {
    .txbuf = NULL,
    .rxbuf = buf,
    .size = size,
  };

  at86rf233_cs(true);

  // send command and read unnecessary data first
  _xfer.size      = 1;
  _xfer.txbuf[0]  = TRX_CMD_FR;
  spi_m_sync_transfer(&RF_SPI, &_xfer);

  // read data. data[0] = length
  spi_m_sync_transfer(&RF_SPI, &xfer);

  at86rf233_cs(false);

  PHY_ENABLE_IRQ();
}

static void
trx_frame_write(uint8_t* buf, uint8_t size)
{
  PHY_DISABLE_IRQ();

  struct spi_xfer xfer = 
  {
    .txbuf = buf,
    .rxbuf = NULL,
    .size = size,
  };

  at86rf233_cs(true);

  // send command and read unnecessary data first
  _xfer.size      = 1;
  _xfer.txbuf[0]  = TRX_CMD_FW;
  spi_m_sync_transfer(&RF_SPI, &_xfer);

  // write data. data[0] = actual 
  spi_m_sync_transfer(&RF_SPI, &xfer);

  at86rf233_cs(false);

  PHY_ENABLE_IRQ();
}

void PHY_Init(void)
{
	trx_spi_init();
	PhyReset();
	phyRxState = false;
	phyState = PHY_STATE_IDLE;

	do
  {
    phyWriteRegister(TRX_STATE_REG, TRX_CMD_TRX_OFF);
	} while (TRX_STATUS_TRX_OFF != (phyReadRegister(TRX_STATUS_REG) & TRX_STATUS_MASK));

	phyWriteRegister(TRX_CTRL_1_REG,
			(1 << TX_AUTO_CRC_ON) | (3 << SPI_CMD_MODE) |
			(1 << IRQ_MASK_MODE));

	phyWriteRegister(TRX_CTRL_2_REG,
			(1 << RX_SAFE_MODE) | (1 << OQPSK_SCRAM_EN));

  ext_irq_register(RF_IRQ, __at86rf233_irq_handler);
  event_register_handler(at86rf233_irq_handler, DISPATCH_EVENT_RF_IRQ);
  ext_irq_enable(RF_IRQ);
  
  phyWriteRegister(IRQ_MASK_REG, 0x08);
}

void PHY_SetRxState(bool rx)
{
	phyRxState = rx;
	phySetRxState();
}

void PHY_SetChannel(uint8_t channel)
{
	uint8_t reg;

	reg = phyReadRegister(PHY_CC_CCA_REG) & ~0x1f;
	phyWriteRegister(PHY_CC_CCA_REG, reg | channel);
}

void PHY_SetPanId(uint16_t panId)
{
	uint8_t *d = (uint8_t *)&panId;

	phyWriteRegister(PAN_ID_0_REG, d[0]);
	phyWriteRegister(PAN_ID_1_REG, d[1]);
}

void PHY_SetShortAddr(uint16_t addr)
{
	uint8_t *d = (uint8_t *)&addr;

	phyWriteRegister(SHORT_ADDR_0_REG, d[0]);
	phyWriteRegister(SHORT_ADDR_1_REG, d[1]);
	phyWriteRegister(CSMA_SEED_0_REG, d[0] + d[1]);
}

void PHY_SetTxPower(uint8_t txPower)
{
	uint8_t reg;

	reg = phyReadRegister(PHY_TX_PWR_REG) & ~0x0f;
	phyWriteRegister(PHY_TX_PWR_REG, reg | txPower);
}

void PHY_Sleep(void)
{
	phyTrxSetState(TRX_CMD_TRX_OFF);
	TRX_SLP_TR_HIGH();
	phyState = PHY_STATE_SLEEP;
}

void PHY_Wakeup(void)
{
	TRX_SLP_TR_LOW();
	phySetRxState();
	phyState = PHY_STATE_IDLE;
}

void PHY_DataReq(uint8_t *data)
{
	phyTrxSetState(TRX_CMD_TX_ARET_ON);

	phyReadRegister(IRQ_STATUS_REG);

	/* size of the buffer is sent as first byte of the data
	 * and data starts from second byte.
	 */
#if 1 // XXX hkim! Why +2 then -1? frame length can remain same!
	data[0] += 2;
	trx_frame_write(data, (data[0] - 1) /* length value*/);
#else
	trx_frame_write(data, data[0] + 1); // data[0] = length of pdu, rest is pdu
#endif

	phyState = PHY_STATE_TX_WAIT_END;

#if 1 // this is much faster
	TRX_SLP_TR_HIGH();
	TRX_TRIG_DELAY();
	TRX_SLP_TR_LOW();
#else
  phyWriteRegister(TRX_STATE_REG, TRX_CMD_TX_START);
#endif
}

uint16_t PHY_RandomReq(void)
{
	uint16_t rnd = 0;
	uint8_t rndValue;

	phyTrxSetState(TRX_CMD_RX_ON);

	for (uint8_t i = 0; i < 16; i += 2)
  {
		delay_us(RANDOM_NUMBER_UPDATE_INTERVAL);
		rndValue = (phyReadRegister(PHY_RSSI_REG) >> RND_VALUE) & 3;
		rnd |= rndValue << i;
	}

	phySetRxState();

	return rnd;
}

void PHY_EncryptReq(uint8_t *text, uint8_t *key)
{
#if 0 // FIXME
	sal_aes_setup(key, AES_MODE_ECB, AES_DIR_ENCRYPT);
#if (SAL_TYPE == AT86RF2xx)
	sal_aes_wrrd(text, NULL);
#else
	sal_aes_exec(text);
#endif
	sal_aes_read(text);
#endif
}

int8_t PHY_EdReq(void)
{
	uint8_t ed;

	phyTrxSetState(TRX_CMD_RX_ON);
	phyWriteRegister(PHY_ED_LEVEL_REG, 0);

	while (0 == (phyReadRegister(IRQ_STATUS_REG) & (1 << CCA_ED_DONE)))
  {
	}

	ed = (int8_t)phyReadRegister(PHY_ED_LEVEL_REG);

	phySetRxState();

	return ed + PHY_RSSI_BASE_VAL;
}

static void phyWriteRegister(uint8_t reg, uint8_t value)
{
	trx_reg_write(reg, value);
}

uint8_t phyReadRegister(uint8_t reg)
{
	uint8_t value;

	value = trx_reg_read(reg);

	return value;
}

static void phyWaitState(uint8_t state)
{
	while (state != (phyReadRegister(TRX_STATUS_REG) & TRX_STATUS_MASK))
  {
	}
}

static void phySetRxState(void)
{
	phyTrxSetState(TRX_CMD_TRX_OFF);

	phyReadRegister(IRQ_STATUS_REG);

	if (phyRxState)
  {
		phyTrxSetState(TRX_CMD_RX_AACK_ON);
	}
}

static void phyTrxSetState(uint8_t state)
{
	do
  {
    phyWriteRegister(TRX_STATE_REG, TRX_CMD_FORCE_TRX_OFF);
	} while (TRX_STATUS_TRX_OFF != (phyReadRegister(TRX_STATUS_REG) & TRX_STATUS_MASK));

	do
  {
    phyWriteRegister(TRX_STATE_REG, state);
  } while (state != (phyReadRegister(TRX_STATUS_REG) & TRX_STATUS_MASK));
}

void PHY_SetIEEEAddr(uint8_t *ieee_addr)
{
	uint8_t *ptr_to_reg = ieee_addr;
	for (uint8_t i = 0; i < 8; i++)
  {
		trx_reg_write((IEEE_ADDR_0_REG + i), *ptr_to_reg);
		ptr_to_reg++;
	}
}

void
PHY_TaskHandler(void)
{
  if (PHY_STATE_SLEEP == phyState)
  {
    return;
  }

  if (phyReadRegister(IRQ_STATUS_REG) & (1 << TRX_END))
  {
    if (PHY_STATE_IDLE == phyState)
    {
      PHY_DataInd_t ind;
      uint8_t size;
      int8_t rssi;

      rssi = (int8_t)phyReadRegister(PHY_ED_LEVEL_REG);

      trx_frame_read(&size, 1);

      trx_frame_read(phyRxBuffer, size + 2);

      ind.data = phyRxBuffer + 1;

      ind.size = size - PHY_CRC_SIZE;
      ind.lqi  = phyRxBuffer[size + 1];
      ind.rssi = rssi + PHY_RSSI_BASE_VAL;

      phyWaitState(TRX_STATUS_RX_AACK_ON);

      PHY_DataInd(&ind);
    }
    else if (PHY_STATE_TX_WAIT_END == phyState)
    {
      uint8_t status = (phyReadRegister(TRX_STATE_REG) >> TRAC_STATUS) & 7;

      if (TRAC_STATUS_SUCCESS == status)
      {
        status = PHY_STATUS_SUCCESS;
      }
      else if (TRAC_STATUS_CHANNEL_ACCESS_FAILURE == status)
      {
        status = PHY_STATUS_CHANNEL_ACCESS_FAILURE;
      }
      else if (TRAC_STATUS_NO_ACK == status)
      {
        status = PHY_STATUS_NO_ACK;
      }
      else
      {
        status = PHY_STATUS_ERROR;
      }

      phySetRxState();
      phyState = PHY_STATE_IDLE;

      PHY_DataConf(status);
    }
  }
}
