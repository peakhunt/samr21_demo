/*
 * Code generated from Atmel Start.
 *
 * This file will be overwritten when reconfiguring your Atmel Start project.
 * Please copy examples or other code you want to keep to a separate file
 * to avoid losing it when reconfiguring.
 */
#ifndef ATMEL_START_PINS_H_INCLUDED
#define ATMEL_START_PINS_H_INCLUDED

#include <hal_gpio.h>

// SAMR21 has 8 pin functions

#define GPIO_PIN_FUNCTION_A 0
#define GPIO_PIN_FUNCTION_B 1
#define GPIO_PIN_FUNCTION_C 2
#define GPIO_PIN_FUNCTION_D 3
#define GPIO_PIN_FUNCTION_E 4
#define GPIO_PIN_FUNCTION_F 5
#define GPIO_PIN_FUNCTION_G 6
#define GPIO_PIN_FUNCTION_H 7

/*
#define PA12 GPIO(GPIO_PORTA, 12)
#define PA13 GPIO(GPIO_PORTA, 13)
#define PA14 GPIO(GPIO_PORTA, 14)
*/
#define LED0 GPIO(GPIO_PORTA, 19)
#define RF_SLP_TR GPIO(GPIO_PORTA, 20)
#define PA24 GPIO(GPIO_PORTA, 24)
#define PA25 GPIO(GPIO_PORTA, 25)
#define RF_RSTN GPIO(GPIO_PORTB, 15)
#define RF_SEL GPIO(GPIO_PORTB, 31)

#define PB30 GPIO(GPIO_PORTB, 30)
#define PC19 GPIO(GPIO_PORTC, 19)
#define PC18 GPIO(GPIO_PORTC, 18)

#endif // ATMEL_START_PINS_H_INCLUDED
