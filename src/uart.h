/**************************************************************************
 * UART interrupt-driven implementation
 **************************************************************************/
#ifndef UART_H
#define UART_H

#ifdef __cplusplus
extern "C" {
#endif

/* UART RX callback called when some data is available to read */
typedef void (*uart_rx_cb)(void* user_data);

/* Init UART with baud rate. Sizes must be power of two */
void uart_init(uint16_t bauds,
			   uint8_t* rx_buff, uint16_t rx_sz,
			   uint8_t* tx_buff, uint16_t tx_sz,
			   uart_rx_cb cb, void* user_data);

/* Get UART RX buffer with its size available to read */
void uart_rx_ptr(void** ptr, uint16_t* sz);
/* Get UART TX buffer with its size available to write */
void uart_tx_ptr(void** ptr, uint16_t* sz);

/* Commit read ptr after reading data from RX buffer */
void uart_rx_advance(uint16_t sz);
/* Commit write ptr after writing data to TX buffer */
void uart_tx_advance(uint16_t sz);

/* Print formatted string that resides in program memory to UART.
   This function is totally async, i.e. if there is no place in
   ring buffer, string will be truncated. */
uint16_t uart_printf_pgm_async(const char* fmt, ...);

#ifdef __cplusplus
}
#endif

#endif //UART_H
