#include <atmel_start.h>
#include <hal_delay.h>
#include <hal_gpio.h>

#include "app_common.h"
#include "blinky.h"
#include "mainloop_timer.h"

#define BLINKY_INTERVAL         50

static SoftTimerElem    _blinky_timer;

static void
blinky_callback(SoftTimerElem* te)
{
  gpio_toggle_pin_level(LED0);
  mainloop_timer_schedule(&_blinky_timer, BLINKY_INTERVAL);
}

void
blinky_init(void)
{
  soft_timer_init_elem(&_blinky_timer);
  _blinky_timer.cb    = blinky_callback;
  mainloop_timer_schedule(&_blinky_timer, BLINKY_INTERVAL);
}
