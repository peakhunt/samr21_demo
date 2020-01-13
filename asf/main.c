#include <atmel_start.h>
#include "app.h"

int main(void)
{
	/* Initializes MCU, drivers and middleware */
	atmel_start_init();

  app_begin();
}
