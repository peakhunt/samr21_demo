#include <string.h>

#include "atmel_start.h"
#include "usb_start.h"

#include "app_common.h"

// #include "shell_if_usb.h"
#include "shell.h"

#include "event_list.h"
#include "circ_buffer.h"
#include "event_list.h"
#include "event_dispatcher.h"

#define TX_BUFFER_SIZE      (512)

static void shellif_usb_tx_usb(uint8_t from_isr);

static IRQn_Type              _irqn  = USB_IRQn;
static CircBuffer             _rx_cb;
static volatile uint8_t       _rx_buffer[CLI_RX_BUFFER_LENGTH];
static ShellIntf              _shell_usb_if;
static uint8_t                _rx_bounce_buf[64];

static volatile uint8_t       _tx_buffer[TX_BUFFER_SIZE];
static CircBuffer             _tx_cb;
static uint8_t                _tx_in_prog = false;

////////////////////////////////////////////////////////////////////////////////
//
// circular buffer callback
//
////////////////////////////////////////////////////////////////////////////////
static void
shellif_usb_enter_critical(CircBuffer* cb)
{
  NVIC_DisableIRQ(_irqn);
  __DSB();
  __ISB();
}

static void
shellif_usb_leave_critical(CircBuffer* cb)
{
  NVIC_EnableIRQ(_irqn);
}

////////////////////////////////////////////////////////////////////////////////
//
// USB IRQ Callbacks
//
////////////////////////////////////////////////////////////////////////////////
static bool
shellif_usb_tx_complete(const uint8_t ep, const enum usb_xfer_code rc, const uint32_t count)
{
  // event_set(1 << DISPATCH_EVENT_USB_CLI_TX);
  shellif_usb_tx_usb(true);

  /* No error. */
  return false;
}

static bool
shellif_usb_rx(const uint8_t ep, const enum usb_xfer_code rc, const uint32_t count)
{
  if(circ_buffer_enqueue(&_rx_cb, _rx_bounce_buf, count, true) == false)
  {
    // fucked up. overflow mostly.
    // do something here
  }

  event_set(1 << DISPATCH_EVENT_USB_CLI_RX);

  cdcdf_acm_read((uint8_t*)_rx_bounce_buf, sizeof(_rx_bounce_buf));

  /* No error. */
  return false;
}

static bool
shellif_usb_lc(usb_cdc_control_signal_t state)
{
	if(state.rs232.DTR)
  {
    cdcdf_acm_register_callback(CDCDF_ACM_CB_WRITE, (FUNC_PTR)shellif_usb_tx_complete);
    cdcdf_acm_register_callback(CDCDF_ACM_CB_READ, (FUNC_PTR)shellif_usb_rx);

    // start RX
    cdcdf_acm_read((uint8_t*)_rx_bounce_buf, sizeof(_rx_bounce_buf));
  }
  else if(!state.rs232.DTR)
  {
    // reset all buffers
  }
  return false;
}

////////////////////////////////////////////////////////////////////////////////
//
// shell callback
//
////////////////////////////////////////////////////////////////////////////////
static bool
shellif_usb_get_rx_data(ShellIntf* intf, uint8_t* data)
{
  if(circ_buffer_dequeue(&_rx_cb, data, 1, false) == false)
  {
    return false;
  }
  return true;
}

static void
shellif_usb_tx_usb(uint8_t from_isr)
{
  int num_bytes;
  static uint8_t  buf[64];

  if(circ_buffer_is_empty(&_tx_cb, from_isr))
  {
    _tx_in_prog = false;
    return;
  }

  num_bytes = _tx_cb.num_bytes > 64 ? 64 : _tx_cb.num_bytes;
  circ_buffer_dequeue(&_tx_cb, buf, num_bytes, from_isr);

  cdcdf_acm_write(buf, num_bytes);
  _tx_in_prog = true;
}

static void
shellif_usb_put_tx_data(ShellIntf* intf, uint8_t* data, uint16_t len)
{
  if(_tx_cb.capacity < len)
  {
    // BUG
    while(1)
      ;
  }

  while(1)
  {
    shellif_usb_enter_critical(NULL);

    if(circ_buffer_enqueue(&_tx_cb, data, (uint8_t)len, false) == true)
    {
      break;
    }

    shellif_usb_leave_critical(NULL);
  }

  // still locked
  if(_tx_in_prog == false)
  {
    shellif_usb_tx_usb(false);
  }
  shellif_usb_leave_critical(NULL);
}

static void
shellif_usb_rx_event_handler(uint32_t event)
{
  shell_handle_rx(&_shell_usb_if);
}

////////////////////////////////////////////////////////////////////////////////
//
// public interfaces
//
////////////////////////////////////////////////////////////////////////////////
void
shellif_usb_init(void)
{
  _shell_usb_if.cmd_buffer_ndx    = 0;
  _shell_usb_if.get_rx_data       = shellif_usb_get_rx_data;
  _shell_usb_if.put_tx_data       = shellif_usb_put_tx_data;

  INIT_LIST_HEAD(&_shell_usb_if.lh);

  circ_buffer_init(&_rx_cb, _rx_buffer, CLI_RX_BUFFER_LENGTH,
      shellif_usb_enter_critical,
      shellif_usb_leave_critical);

  circ_buffer_init(&_tx_cb, _tx_buffer, TX_BUFFER_SIZE, NULL, NULL);

  shell_if_register(&_shell_usb_if);

  event_register_handler(shellif_usb_rx_event_handler, DISPATCH_EVENT_USB_CLI_RX);

  cdcdf_acm_register_callback(CDCDF_ACM_CB_STATE_C, (FUNC_PTR)shellif_usb_lc);
}
