#include "gpio.h"
#include <unistd.h>

#define P24 24 // transmit
#define P25 25 // receive

gpiod_line *line24;
gpiod_line *line25;

bool val24, val25; // BOTH CANNOT BE ZERO at same time

static gpiod_chip *chip;

uint8_t init_gpio()
{
    chip = gpiod_chip_open_by_name("gpiochip4");
    if (!chip)
        return GPIO_ERROR;

    return NO_ERROR;
}

uint8_t close_gpio() // italain brainrot // water bucket
{
    gpiod_chip_close(chip);
    return NO_ERROR;
}

uint8_t init_pins(const char *mode)
{
    if (!chip)
        cerr << "Error: GPIO chip not initialized." << endl;

    line24 = gpiod_chip_get_line(chip, P24);
    if (!line24)
    {
        cerr << "Error: Could not access GPIO pin " << P24 << "." << endl;
        return GPIO_ERROR;
    }

    if (gpiod_line_request_output(line24, mode, 0) < 0)
    {
        cerr << "Error: Could not set GPIO pin mode to " << mode << "." << endl;
        return GPIO_ERROR;
    }

    line25 = gpiod_chip_get_line(chip, P25);
    if (!line25)
    {
        cerr << "Error: Could not access GPIO pin " << P25 << "." << endl;
        return GPIO_ERROR;
    }

    if (gpiod_line_request_output(line25, mode, 0) < 0)
    {
        cerr << "Error: Could not set GPIO pin mode to " << mode << "." << endl;
        return GPIO_ERROR;
    }

    return NO_ERROR;
}

uint8_t on_24()
{
    if (val25)
    {
        cerr << "Error: Unable to set pin 24 to HIGH when pin 25 is HIGH." << endl;
        return STATE_ERROR;
    }

    val24 = true;
    if (gpiod_line_set_value(line24, val24))
    {
        val24 = false;
        return GPIO_ERROR;
    }
    return NO_ERROR;
}

uint8_t off_24()
{
    val24 = false;
    if (gpiod_line_set_value(line24, val24))
    {
        val24 = true;
        return GPIO_ERROR;
    }

    return NO_ERROR;
}

uint8_t on_25()
{
    if (val24)
    {
        cerr << "Error: Unable to set pin 25 to HIGH when pin 24 is HIGH." << endl;
        return STATE_ERROR;
    }

    val25 = true;
    if (gpiod_line_set_value(line25, val25))
    {
        val25 = false;
        return GPIO_ERROR;
    }
    return NO_ERROR;
}

uint8_t off_25()
{
    val25 = false;
    if (gpiod_line_set_value(line25, val25))
    {
        val25 = true;
        return GPIO_ERROR;
    }
    return NO_ERROR;
}

uint8_t set_gpio_mode(uint8_t mode)
{
    uint8_t err = NO_ERROR;
    if (mode == RX_MODE)
    {
        err = off_24();
        usleep(5);
        if (!err)
            err = on_25();
    }
    else if (mode == TX_MODE)
    {
        err = off_25();
        usleep(5);
        if (!err)
            err = on_24();
    }
    else
    {
        err = off_24();
        if (!err)
            err = off_25();
    }

    return err;
}

// int main()
// {

//     init_gpio();
//     init_pins("toggle");

//     on_24();
//     on_25();

//     off_24();
//     on_25();
//     on_24();

//     // bool value = false;
//     // while (true)
//     // {
//     //     value = !value;
//     // }

//     close_gpio();
//     return 0;
// }
