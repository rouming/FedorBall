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

/* Init UART with baud rate */
void uart_init(uint16_t bauds,
			   uart_rx_cb cb,
			   void* user_data);

/* Get UART RX buffer with its size available to read */
void uart_rx_ptr(void** ptr, uint32_t* sz);
/* Get UART TX buffer with its size available to write */
void uart_tx_ptr(void** ptr, uint32_t* sz);

/* Commit read ptr after reading data from RX buffer */
void uart_rx_advance(uint32_t sz);
/* Commit write ptr after writing data to TX buffer */
void uart_tx_advance(uint32_t sz);

/* Print formatted string to UART */
uint16_t uart_printf(const char* fmt, ...);

#ifdef __cplusplus
}
#endif

#endif //UART_H
