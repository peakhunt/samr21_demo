/*
 * Code generated from Atmel Start.
 *
 * This file will be overwritten when reconfiguring your Atmel Start project.
 * Please copy examples or other code you want to keep to a separate file
 * to avoid losing it when reconfiguring.
 */

#include "driver_examples.h"
#include "driver_init.h"
#include "utils.h"

static void button_on_PA22_pressed(void)
{
}

/**
 * Example of using EXTERNAL_IRQ_0
 */
void EXTERNAL_IRQ_0_example(void)
{

	ext_irq_register(PIN_PA22, button_on_PA22_pressed);
}

/**
 * Example of using RF_SPI to write "Hello World" using the IO abstraction.
 */
static uint8_t example_RF_SPI[12] = "Hello World!";

void RF_SPI_example(void)
{
	struct io_descriptor *io;
	spi_m_sync_get_io_descriptor(&RF_SPI, &io);

	spi_m_sync_enable(&RF_SPI);
	io_write(io, example_RF_SPI, 12);
}

void delay_example(void)
{
	delay_ms(5000);
}
