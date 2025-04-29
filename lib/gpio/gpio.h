#ifndef __GPIO__
#define __GPIO__

#include <gpiod.h>
#include <chrono>
#include <iostream>
#include "../control.h"

using namespace std;

#define RX_MODE 0
#define TX_MODE 1

uint8_t init_gpio();

uint8_t close_gpio();

uint8_t init_pins(const char *mode);

uint8_t on_24();
uint8_t off_24();

uint8_t on_25();
uint8_t off_25();

uint8_t set_gpio_mode(uint8_t mode);

#endif // __GPIO__