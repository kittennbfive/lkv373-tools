#ifndef __PERIPH_UNKNOWN_H__
#define __PERIPH_UNKNOWN_H__
#include <stdint.h>
#include <stdbool.h>

#include "peripherals.h"

void unknown_write(PERIPH_CB_WRITE_ARGUMENTS);
bool unknown_read(PERIPH_CB_READ_ARGUMENTS);
void unknown_init(void);

#endif
