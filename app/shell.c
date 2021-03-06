#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "app_common.h"
#include "shell.h"
#include "shellif_usb.h"
#include "phy.h"

////////////////////////////////////////////////////////////////////////////////
//
// private definitions
//
////////////////////////////////////////////////////////////////////////////////

#define SHELL_MAX_COLUMNS_PER_LINE      128
#define SHELL_COMMAND_MAX_ARGS          4

#define VERSION       "SAMR21 Shell V0.1\r\nAll rights reserved 2019\r\n"

typedef void (*shell_command_handler)(ShellIntf* intf, int argc, const char** argv);

typedef struct
{
  const char*           command;
  const char*           description;
  shell_command_handler handler;
} ShellCommand;

////////////////////////////////////////////////////////////////////////////////
//
// private prototypes
//
////////////////////////////////////////////////////////////////////////////////
static void shell_command_help(ShellIntf* intf, int argc, const char** argv);
static void shell_command_version(ShellIntf* intf, int argc, const char** argv);
static void shell_command_rf_reg_read(ShellIntf* intf, int argc, const char** argv);
static void shell_command_rf_set_rx_state(ShellIntf* intf, int argc, const char** argv);
static void shell_command_rf_set_channel(ShellIntf* intf, int argc, const char** argv);
static void shell_command_rf_set_panid(ShellIntf* intf, int argc, const char** argv);
static void shell_command_rf_set_shortaddr(ShellIntf* intf, int argc, const char** argv);
static void shell_command_rf_set_ieeeaddr(ShellIntf* intf, int argc, const char** argv);
static void shell_command_rf_send_long_frame(ShellIntf* intf, int argc, const char** argv);
static void shell_command_rf_send_short_frame(ShellIntf* intf, int argc, const char** argv);

////////////////////////////////////////////////////////////////////////////////
//
// private variables
//
////////////////////////////////////////////////////////////////////////////////
const uint8_t                 _welcome[] = "\r\n**** Welcome ****\r\n";
const uint8_t                 _prompt[]  = "\r\nSAMR21> ";

static char                   _print_buffer[SHELL_MAX_COLUMNS_PER_LINE + 1];

static ShellIntf*             _intf_cb = NULL;
static ShellIntf*             _intf_rx = NULL;

static LIST_HEAD(_shell_intf_list);

static ShellCommand     _commands[] = 
{
  {
    "help",
    "show this command",
    shell_command_help,
  },
  {
    "version",
    "show program version",
    shell_command_version,
  },
  {
    "rf_reg_read",
    "read register from rf",
    shell_command_rf_reg_read,
  },
  {
    "rf_set_rx_state",
    "set rx state to true or false",
    shell_command_rf_set_rx_state,
  },
  {
    "rf_set_channel",
    "set rf channel",
    shell_command_rf_set_channel,
  },
  {
    "rf_set_panid",
    "set RF PAN ID",
    shell_command_rf_set_panid,
  },
  {
    "rf_set_shortaddr",
    "set RF short address",
    shell_command_rf_set_shortaddr,
  },
  {
    "rf_set_ieeeaddr",
    "set RF IEEE address",
    shell_command_rf_set_ieeeaddr,
  },
  {
    "rf_send_long_frame",
    "send long frame",
    shell_command_rf_send_long_frame,
  },
  {
    "rf_send_short_frame",
    "send short frame",
    shell_command_rf_send_short_frame,
  },
};

////////////////////////////////////////////////////////////////////////////////
//
// shell utilities
//
////////////////////////////////////////////////////////////////////////////////
static inline void
shell_prompt(ShellIntf* intf)
{
  intf->put_tx_data(intf, (uint8_t*)_prompt, sizeof(_prompt) -1);
}

////////////////////////////////////////////////////////////////////////////////
//
// shell command handlers
//
////////////////////////////////////////////////////////////////////////////////
static void
shell_command_help(ShellIntf* intf, int argc, const char** argv)
{
  size_t i;

  shell_printf(intf, "\r\n");

  for(i = 0; i < sizeof(_commands)/sizeof(ShellCommand); i++)
  {
    shell_printf(intf, "%-20s: ", _commands[i].command);
    shell_printf(intf, "%s\r\n", _commands[i].description);
  }
}

static void
shell_command_version(ShellIntf* intf, int argc, const char** argv)
{
  shell_printf(intf, "\r\n");
  shell_printf(intf, "%s\r\n", VERSION);
}

static void
shell_command_rf_reg_read(ShellIntf* intf, int argc, const char** argv)
{
  uint8_t reg;
  uint8_t r;

  if(argc != 2)
  {
    shell_printf(intf, "Syntax error %s 0x<reg>\r\n", argv[0]);
    return;
  }

  reg = strtol(argv[1], NULL, 16);

  r = phyReadRegister(reg);
  shell_printf(intf, "read from 0x%02x: 0x%02x\r\n", reg, r);
}

static void
shell_command_rf_set_rx_state(ShellIntf* intf, int argc, const char** argv)
{
  bool v = false;

  if(argc != 2)
  {
    shell_printf(intf, "Syntax error %s [true|false]\r\n", argv[0]);
    return;
  }

  if(strcmp(argv[1], "true") == 0)
  {
    v = true;
    _intf_rx = intf;
  }
  else
  {
    _intf_rx = NULL;
  }

  PHY_SetRxState(v);
  shell_printf(intf, "setting RX state to %s\r\n", v ? "true" : "false");
}

static void
shell_command_rf_set_channel(ShellIntf* intf, int argc, const char** argv)
{
  uint8_t chnl;

  if(argc != 2)
  {
    shell_printf(intf, "Syntax error %s <channel-number>\r\n", argv[0]);
    return;
  }

  chnl = strtol(argv[1], NULL, 10);

  shell_printf(intf, "setting channel to %d\r\n", chnl);
  PHY_SetChannel(chnl);
}

static void
shell_command_rf_set_panid(ShellIntf* intf, int argc, const char** argv)
{
  uint16_t pan_id;

  if(argc != 2)
  {
    shell_printf(intf, "Syntax error %s 0x<pan-id>\r\n", argv[0]);
    return;
  }

  pan_id = strtol(argv[1], NULL, 16);

  shell_printf(intf, "setting PAN ID to 0x%x\r\n", pan_id);
  PHY_SetPanId(pan_id);
}

static void
shell_command_rf_set_shortaddr(ShellIntf* intf, int argc, const char** argv)
{
  uint16_t addr;

  if(argc != 2)
  {
    shell_printf(intf, "Syntax error %s 0x<short addr>\r\n", argv[0]);
    return;
  }

  addr = strtol(argv[1], NULL, 16);

  shell_printf(intf, "setting short address to 0x%x\r\n", addr);
  PHY_SetShortAddr(addr);
}

static uint8_t
check_ieee_addr(char* str, uint8_t addr[8])
{
  //
  // format is
  // XX:XX:XX:XX:XX:XX:XX:XX
  //
  char *s, *t;
  int i = 0;

  for(i = 0; i < 8; i++)
  {
    s = strtok_r(i == 0 ? str : NULL, ":", &t);
    if(s == NULL ||
      strlen(s) != 2 ||
      isxdigit((uint8_t)s[0]) == 0 ||
      isxdigit((uint8_t)s[1]) == 0)
    {
      return 1;
    }

    addr[i] = strtol(s, NULL, 16);
  }
  return 0;
}

static void
shell_command_rf_set_ieeeaddr(ShellIntf* intf, int argc, const char** argv)
{
  uint8_t addr[8];

  if(argc != 2)
  {
    shell_printf(intf, "Syntax error %s 0x<ieee addr>\r\n", argv[0]);
    return;
  }

  if(check_ieee_addr((char*)argv[1], addr) != 0)
  {
    shell_printf(intf, "invalid IEEE address %s\r\n", argv[1]);
    return;
  }

  shell_printf(intf, "setting IEEE address to %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x\r\n",
      addr[0], addr[1], addr[2], addr[3], addr[4], addr[5], addr[6], addr[7]);
  PHY_SetIEEEAddr(addr);
}

static void
shell_command_rf_send_long_frame(ShellIntf* intf, int argc, const char** argv)
{
  static uint8_t data[64];
  uint8_t ack = false;

  //
  // data[0]      : frame length
  // data[1 ... ] : actual data
  //
  if(argc != 2)
  {
    shell_printf(intf, "Syntax error %s [ack|noack]\r\n", argv[0]);
    return;
  }

  if(strcmp(argv[1], "ack") == 0)
  {
    ack = true;
  }

  // length
  data[0] = 28;

  // FCF: first byte
  data[1] = ((0x01) << 0)               |   // FCF: frame type
            ((0x00) << 3)               |   // FCF: sec enabled
            ((0x00) << 4)               |   // FCF: frame pending
            ((ack ? 0x01 : 0x00) << 5)  |   // FCF: ack request
            ((0x00) << 6)                   // FCF: pan ID compression
  ;

  // FCF: second byte
  data[2] = ((0x03) << 2)               |   // FCF: dest address mode. long
            ((0x01) << 4)               |   // FCF: frame version. 2006
            ((0x03) << 6)                   // FCF: source address mode. long
  ;

  // sequence
  data[3] = 0x33;

  // destination PAN ID
  data[4] = 0x01;
  data[5] = 0x02;

  // destination address
  data[6] = 0x01;
  data[7] = 0x02;
  data[8] = 0x03;
  data[9] = 0x04;
  data[10] = 0x05;
  data[11] = 0x06;
  data[12] = 0x07;
  data[13] = 0x08;

  // source PAN ID
  data[14] = 0x01;
  data[15] = 0x02;

  // source address
  data[16] = 0x10;
  data[17] = 0x20;
  data[18] = 0x30;
  data[19] = 0x40;
  data[20] = 0x50;
  data[21] = 0x60;
  data[22] = 0x70;
  data[23] = 0x80;

  // MSDU
  data[24] = 0xaa;
  data[25] = 0xbb;
  data[26] = 0xcc;
  data[27] = 0xdd;
  data[28] = 0xee;

  // CRC
  // not necessary

  shell_printf(intf, "sending long frame... %s\r\n", ack ? "ack" : "noack");
  _intf_cb = intf;

  PHY_DataReq(data);
}

static void
shell_command_rf_send_short_frame(ShellIntf* intf, int argc, const char** argv)
{
  static uint8_t data[64];
  uint8_t ack = false;

  //
  // data[0]      : frame length
  // data[1 ... ] : actual data
  //
  if(argc != 2)
  {
    shell_printf(intf, "Syntax error %s [ack|noack]\r\n", argv[0]);
    return;
  }

  if(strcmp(argv[1], "ack") == 0)
  {
    ack = true;
  }

  // length
  data[0] = 16;

  // FCF: first byte
  data[1] = ((0x01) << 0)               |   // FCF: frame type
            ((0x00) << 3)               |   // FCF: sec enabled
            ((0x00) << 4)               |   // FCF: frame pending
            ((ack ? 0x01 : 0x00) << 5)  |   // FCF: ack request
            ((0x00) << 6)                   // FCF: pan ID compression
  ;

  // FCF: second byte
  data[2] = ((0x02) << 2)               |   // FCF: dest address mode. long
            ((0x01) << 4)               |   // FCF: frame version. 2006
            ((0x02) << 6)                   // FCF: source address mode. long
  ;

  // sequence
  data[3] = 0x33;

  // destination PAN ID
  data[4] = 0x01;
  data[5] = 0x02;

  // destination address
  data[6] = 0x01;
  data[7] = 0x02;

  // source PAN ID
  data[8] = 0x01;
  data[9] = 0x02;

  // source address
  data[10] = 0x10;
  data[11] = 0x20;

  // MSDU
  data[12] = 0xaa;
  data[13] = 0xbb;
  data[14] = 0xcc;
  data[15] = 0xdd;
  data[16] = 0xee;
  data[17] = 0xff;

  // CRC
  // not necessary

  shell_printf(intf, "sending short frame... %s\r\n", ack ? "ack" : "noack");
  _intf_cb = intf;

  PHY_DataReq(data);
}

void
__PHY_DataConf(uint8_t status)
{
  if(_intf_cb == NULL)
  {
    return;
  }

  shell_printf(_intf_cb, "\r\n--> PHY_DataConf %d\r\n", status);
  shell_prompt(_intf_cb);
}

void
__PHY_DataInd(PHY_DataInd_t *ind)
{
  if(_intf_rx == NULL)
  {
    return;
  }

  shell_printf(_intf_rx, "\r\n --> PHY_DataInd\r\n");
  shell_printf(_intf_rx, "\r\n size: %d, lqi: %d, rssi: %d\r\n",
      ind->size, ind->lqi, ind->rssi);

  for(uint8_t i = 0; i < ind->size; )
  {
    uint8_t chunk_size;

    chunk_size= (i + 8) <= ind->size ? 8 : (ind->size - i);

    for(uint8_t j = 0; j < chunk_size; j++)
    {
      shell_printf(_intf_rx, " %02x", ind->data[i + j]);
    }
    shell_printf(_intf_rx, "\r\n");

    i += chunk_size;
  }
  shell_prompt(_intf_cb);
}

////////////////////////////////////////////////////////////////////////////////
//
// shell core
//
////////////////////////////////////////////////////////////////////////////////
static void
shell_execute_command(ShellIntf* intf, char* cmd)
{
  static const char*    argv[SHELL_COMMAND_MAX_ARGS];
  int                   argc = 0;
  size_t                i;
  char                  *s, *t;

  while((s = strtok_r(argc  == 0 ? cmd : NULL, " \t", &t)) != NULL)
  {
    if(argc >= SHELL_COMMAND_MAX_ARGS)
    {
      shell_printf(intf, "\r\nError: too many arguments\r\n");
      return;
    }
    argv[argc++] = s;
  }

  if(argc == 0)
  {
    return;
  }

  for(i = 0; i < sizeof(_commands)/sizeof(ShellCommand); i++)
  {
    if(strcmp(_commands[i].command, argv[0]) == 0)
    {
      shell_printf(intf, "\r\nExecuting %s\r\n", argv[0]);
      _commands[i].handler(intf, argc, argv);
      return;
    }
  }
  shell_printf(intf, "%s", "\r\nUnknown Command: ");
  shell_printf(intf, "%s", argv[0]);
  shell_printf(intf, "%s", "\r\n");
}


void
shell_printf(ShellIntf* intf, const char* fmt, ...)
{
  va_list   args;
  int       len;

  va_start(args, fmt);
  len = vsnprintf(_print_buffer, SHELL_MAX_COLUMNS_PER_LINE, fmt, args);
  va_end(args);

  intf->put_tx_data(intf, (uint8_t*)_print_buffer, len);
}


////////////////////////////////////////////////////////////////////////////////
//
// public interface
//
////////////////////////////////////////////////////////////////////////////////
void
shell_init(void)
{
  // shell_if_usart_init();
  shellif_usb_init();
}

void
shell_start(void)
{
  ShellIntf* intf;

  list_for_each_entry(intf, &_shell_intf_list, lh)
  {
    intf->put_tx_data(intf, (uint8_t*)_welcome, sizeof(_welcome) -1);
    shell_prompt(intf);
  }
}


void
shell_if_register(ShellIntf* intf)
{
  list_add_tail(&intf->lh, &_shell_intf_list);
}

void
shell_handle_rx(ShellIntf* intf)
{
  uint8_t   b;

  while(1)
  {
    if(intf->get_rx_data(intf, &b) == false)
    {
      return;
    }

    if(b != '\r' && intf->cmd_buffer_ndx < SHELL_MAX_COMMAND_LEN)
    {
      if(b == '\b' || b == 0x7f)
      {
        if(intf->cmd_buffer_ndx > 0)
        {
          shell_printf(intf, "%c%c%c", b, 0x20, b);
          intf->cmd_buffer_ndx--;
        }
      }
      else
      {
        shell_printf(intf, "%c", b);
        intf->cmd_buffer[intf->cmd_buffer_ndx++] = b;
      }
    }
    else if(b == '\r')
    {
      intf->cmd_buffer[intf->cmd_buffer_ndx++] = '\0';

      shell_execute_command(intf, (char*)intf->cmd_buffer);

      intf->cmd_buffer_ndx = 0;
      shell_prompt(intf);
    }
  }
}

struct list_head*
shell_get_intf_list(void)
{
  return &_shell_intf_list;
}
