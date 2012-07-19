/*******************************************************************************
 * Logging file
 ******************************************************************************/
#ifndef LOGGING_H
#define LOGGING_H

#include "uart.h"

#define LOG(...) uart_printf(__VA_ARGS__)

#endif //LOGGING_H
