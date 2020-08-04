#ifndef __PERIPH_I2C_H__
#define __PERIPH_I2C_H__
#include <stdint.h>
#include <stdbool.h>

#include "peripherals.h"
#include "cmd_parser.h"

void init_i2c_it(void);
void i2c_write(PERIPH_CB_WRITE_ARGUMENTS);
bool i2c_read(PERIPH_CB_READ_ARGUMENTS);

void hdmi(PROTOTYPE_ARGS_HANDLER);

#endif
