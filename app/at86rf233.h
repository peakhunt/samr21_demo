#ifndef __AT86RF233_DEF_H__
#define __AT86RF233_DEF_H__

#include "app_common.h"

#define AT86RF233_MAX_FRAME_LEN     255

//
// list of AT86RF233 registers
//
#define AT86RF233_TRX_STATUS        0x01
#define AT86RF233_TRX_STATE         0x02
#define AT86RF233_TRX_CTRL_0        0x03
#define AT86RF233_TRX_CTRL_1        0x04
#define AT86RF233_PHY_TX_PWR        0x05
#define AT86RF233_PHY_RSSI          0x06
#define AT86RF233_PHY_ED_LEVEL      0x07
#define AT86RF233_PHY_CC_CCA        0x08
#define AT86RF233_CCA_THRES         0x09
#define AT86RF233_RX_CTRL           0x0a
#define AT86RF233_SFD_VALUE         0x0b
#define AT86RF233_TRX_CTRL_2        0x0c
#define AT86RF233_ANT_DIV           0x0d
#define AT86RF233_IRQ_MASK          0x0e
#define AT86RF233_IRQ_STATUS        0x0f
#define AT86RF233_VREG_CTRL         0x10
#define AT86RF233_BATMON            0x11
#define AT86RF233_XOSC_CTRL         0x12
#define AT86RF233_CC_CTRL_0         0x13
#define AT86RF233_CC_CTRL_1         0x14
#define AT86RF233_RX_SYN            0x15
#define AT86RF233_TRX_RPC           0x16
#define AT86RF233_XAH_CTRL_1        0x17
#define AT86RF233_FTN_CTRL          0x18
#define AT86RF233_XAH_CTRL_2        0x19
#define AT86RF233_PLL_CF            0x1a
#define AT86RF233_PLL_DCU           0x1b
#define AT86RF233_PART_NUM          0x1c
#define AT86RF233_VERSION_NUM       0x1d
#define AT86RF233_MAN_ID_0          0x1e
#define AT86RF233_MAN_ID_1          0x1f
#define AT86RF233_SHORT_ADDR_0      0x20
#define AT86RF233_SHORT_ADDR_1      0x21
#define AT86RF233_PAN_ID_0          0x22
#define AT86RF233_PAN_ID_1          0x23
#define AT86RF233_IEEE_ADDR_0       0x24
#define AT86RF233_IEEE_ADDR_1       0x25
#define AT86RF233_IEEE_ADDR_2       0x26
#define AT86RF233_IEEE_ADDR_3       0x27
#define AT86RF233_IEEE_ADDR_4       0x28
#define AT86RF233_IEEE_ADDR_5       0x29
#define AT86RF233_IEEE_ADDR_6       0x2a
#define AT86RF233_IEEE_ADDR_7       0x2b
#define AT86RF233_XAH_CTRL_0        0x2c
#define AT86RF233_CSMA_SEED_0       0x2d
#define AT86RF233_CSMA_SEED_1       0x2e
#define AT86RF233_CSMA_BE           0x2f
#define AT86RF233_TST_CTRL_DIGI     0x36
#define AT86RF233_TST_AGC           0x3c
#define AT86RF233_TST_SDM           0x3d
#define AT86RF233_PHY_TX_TIME       0x3b
#define AT86RF233_PHY_PMU_VALUE     0x3b

typedef struct
{
  uint8_t   phr;
  uint8_t   psdu[AT86RF233_MAX_FRAME_LEN];
  uint8_t   lqi;
  uint8_t   ed;
  uint8_t   rx_status;
} at86rf233_frame_t;

extern void at86rf233_init(void);
extern void at86rf233_reset(void);
extern uint8_t at86rf233_read_reg(uint8_t reg, uint8_t data[2]);
extern uint8_t at86rf233_write_reg(uint8_t reg, uint8_t data);
extern uint8_t at86rf233_read_frame(at86rf233_frame_t* frame);
extern uint8_t at86rf233_write_frame(at86rf233_frame_t* frame);
extern void at86rf233_read_sram(uint8_t addr, uint8_t len, uint8_t* buf);
extern void at86rf233_write_sram(uint8_t addr, uint8_t len, uint8_t* buf);

#endif //!__AT86RF233_DEF_H__
