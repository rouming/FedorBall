/**************************************************************************
 * TWI (I2C) interrupt-driven implementation
 **************************************************************************/
#ifndef TWI_H
#define TWI_H

#ifdef __cplusplus
extern "C" {
#endif

#define TWI_PORT PORTC
#define TWI_DDR  DDRC
#define TWI_SCL  0
#define TWI_SDA  1

#define TWI_FREQ 100000

/* TWI result codes */
typedef enum
{
	twi_ok           = 0,
	twi_bus_fail     = 1,
	twi_mt_sla_nack  = 2,
	twi_mt_data_nack = 3,
	twi_mr_sla_nack  = 4,
	twi_inval_args   = 5,
	twi_not_impl     = 6,
	twi_busy         = 7

} twi_result;

/* TWI iov operation */
typedef enum
{
	twi_write = 0,
	twi_read  = 1

} twi_iov_op;

/* TWI iov struct */
typedef struct
{
	uint8_t dev_addr;
	uint8_t op;
	uint8_t* ptr;
	uint16_t len;
	uint16_t done;

} twi_iov;

/* TWI completion callback */
typedef void (*twi_complete_cb)(twi_result res, void* data);
/*
 * TWI slave receive callback.
 * If callback returns error NACK will be send to master, ACK otherwise.
 */
typedef twi_result (*twi_recv_cb)(uint8_t byte);

/* Init TWI */
twi_result twi_init(uint32_t twi_freq);
/* To clear slave_addr or broadcast pass zero to all the params */
twi_result twi_listen(uint8_t slave_addr, uint8_t is_broadcast,
					  twi_recv_cb cb);

/* TWI submit IO vec op */
twi_result twi_submit_iov(twi_iov* iov, uint8_t iovcnt,
						  twi_complete_cb cb, void* data);

#ifdef __cplusplus
}
#endif

#endif //TWI_H
