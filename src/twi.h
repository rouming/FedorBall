/**************************************************************************
 * TWI (I2C) interrupt-driven implementation
 **************************************************************************/
#ifndef TWI_H
#define TWI_H

#ifdef __cplusplus
extern "C" {
#endif

/* TWI result codes */
enum twi_result
{
  twi_ok   = 0,
  twi_err  = 1,
};

/* TWI event codes */
enum twi_event
{
  twi_send = 0,
  twi_recv = 1,
};

/* TWI events callback */
typedef void (*twi_event_cb)(enum twi_event ev,
							 enum twi_result res,
							 void* data);

/* Init TWI. For master mode slave addr must be set to zero */
twi_result twi_init(uint8_t slave_addr, twi_event_cb cb, void* data);

/* Master send */
twi_result twi_master_send(uint8_t slave_addr, void* data, uint32_t sz);
/* Master receive */
twi_result twi_master_recv(uint8_t slave_addr, void* data, uint32_t sz);
/* Master send and receive */
twi_result twi_master_send_recv(uint8_t slave_addr,
								void* data_wr, uint32_t sz_wr,
								void* data_rd, uint32_t sz_rd);

#ifdef __cplusplus
}
#endif

#endif //TWI_H
