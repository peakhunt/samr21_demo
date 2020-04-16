#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "app_common.h"
#include "phy.h"

void
wpan_ping_init(void)
{
  PHY_SetChannel(11);
  PHY_SetPanId(0x01);
  PHY_SetShortAddr(0x02);
  PHY_SetRxState(true);
}

static void
wpan_ping_response(PHY_DataInd_t* ind)
{
  static uint8_t pkt_buf[128];

  //
  // | 2:FC | 1:seq | v:addressing | 5:data payload |
  //
  // all we have to do is swap src/dest address
  //
  // sample input
  // size: 14, lqi: 255, rssi: -50
  // 61 88 ce 01 00 02 00 01
  // 00 00 05 00 05 ab
  //
  pkt_buf[0] = 14;

  // FC
  pkt_buf[1] = ind->data[0];
  pkt_buf[2] = ind->data[1];

  // SEQ
  pkt_buf[3] = ind->data[2];

  // dest PAN
  pkt_buf[4] = ind->data[3];
  pkt_buf[5] = ind->data[4];

  // dest addr
  pkt_buf[6] = ind->data[7];
  pkt_buf[7] = ind->data[8];

  // src addr
  pkt_buf[8] = ind->data[5];
  pkt_buf[9] = ind->data[6];

  // data
  pkt_buf[10] = ind->data[9];
  pkt_buf[11] = ind->data[10];
  pkt_buf[12] = ind->data[11];
  pkt_buf[13] = ind->data[12];
  pkt_buf[14] = ind->data[13];

  PHY_SetRxState(false);

  PHY_DataReq(pkt_buf);
}

void
PHY_DataInd(PHY_DataInd_t* ind)
{
  wpan_ping_response(ind);
}

void
PHY_DataConf(uint8_t status)
{
  PHY_SetRxState(true);
}
