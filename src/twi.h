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
	twi_err          = 1,
	twi_bus_fail     = 2,
	twi_mt_sla_nack  = 3,
	twi_mt_data_nack = 4,
	twi_mr_sla_nack  = 5,

} twi_result;

/* TWI events callback */
typedef void (*twi_event_cb)(twi_result res, void* data);
/*
 * TWI slave receive callback.
 * If callback returns error NACK will be send to master, ACK otherwise.
 */
typedef twi_result (*twi_recv_cb)(uint8_t byte);

/* Init TWI */
twi_result twi_init(uint32_t twi_freq, twi_event_cb cb, void* data);
/* To clear slave_addr or broadcast pass zero to all the params */
twi_result twi_listen(uint8_t slave_addr, uint8_t is_broadcast,
					  twi_recv_cb cb);

/* TO BE REMOVED. IOV MUST EXIST INSTEAD */

/* Master send */
twi_result twi_master_send(uint8_t slave_addr, void* data, uint32_t sz);
/* Master receive */
twi_result twi_master_recv(uint8_t slave_addr, void* data, uint32_t sz);
/* Master send and receive */
twi_result twi_master_send_recv(uint8_t slave_addr,
								void* data_wr, uint32_t sz_wr,
								void* data_rd, uint32_t sz_rd);

/* TODO */

typedef enum
{
	twi_write = 0,
	twi_read  = 1

} twi_iov_op;

typedef struct
{
	uint8_t dev_addr;
	uint8_t op;
	uint8_t* ptr;
	uint32_t len;
	uint32_t idx;

} twi_iov;

/* TWI submit IO vec op */
twi_result twi_submit_iov(twi_iov* iov, uint32_t iovcnt);

#ifdef __cplusplus
}
#endif

#endif //TWI_H
