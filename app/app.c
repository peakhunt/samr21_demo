#include <atmel_start.h>
#include <peripheral_clk_config.h>
#include <hal_delay.h>
#include <hal_gpio.h>

#include "event_dispatcher.h"
#include "mainloop_timer.h"
#include "blinky.h"
#include "shell.h"
#include "phy.h"
#include "app.h"

static void
app_init_f(void)
{
  // runs with IRQ disabled

  event_dispatcher_init();
  mainloop_timer_init();
}

static void
app_init_r(void)
{
  // runs with IRQ enabled
  PHY_Init();
  blinky_init();
  shell_init();
}

void
app_begin(void)
{
  __disable_irq();
  app_init_f();
  __enable_irq();

  app_init_r();

  // atmel ASF4 is quite inconveineit in this aspect.
  // also atmel delay_xx functions screw up 
  // this setup
  SysTick_Config(CONF_CPU_FREQUENCY/1000);

  while(1)
  {
    /*
    delay_ms(50);
    gpio_toggle_pin_level(LED0);
    */
    event_dispatcher_dispatch();
  }
}
