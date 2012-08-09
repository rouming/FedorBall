/*******************************************************************************
 * Logging file
 ******************************************************************************/
#ifndef LOGGING_H
#define LOGGING_H

#include <avr/pgmspace.h>
#include "uart.h"

#define LOG(fmt, ...) uart_printf_pgm_async(PSTR(fmt), ##__VA_ARGS__)

#endif //LOGGING_H
