/**************************************************************************
 * TWI (I2C) interrupt-driven implementation
 **************************************************************************/

#include <stddef.h>
#include <string.h>

#include <util/twi.h>
#include <util/atomic.h>
#include <avr/interrupt.h>

#include "twi.h"
#include "logging.h"

/*************************************************************************/

typedef enum
{
	twi_stop       = 0,
	twi_send       = 1,
	twi_recv       = 2,
	twi_send_recv  = 3

} twi_state;

static struct
{
	twi_event_cb ev_cb;
	void* ev_data;
	twi_state curr_st;
	uint8_t slave_addr_set;
	uint8_t op_slave_addr;

	uint8_t* tx_ptr;
	uint32_t tx_len;
	uint32_t tx_len_init;

	uint8_t* rx_ptr;
	uint32_t rx_len;
	uint32_t rx_len_init;

} s_twi_state;

#define TWI_INIT_OP(st_, sl_addr_, tx_ptr_, tx_sz_, rx_ptr_, rx_sz_)	\
	do {																\
		s_twi_state.curr_st = st_;										\
		s_twi_state.tx_ptr = (uint8_t*)tx_ptr_;							\
		s_twi_state.tx_len = tx_sz_;									\
		s_twi_state.tx_len_init = tx_sz_;								\
		s_twi_state.rx_ptr = (uint8_t*)rx_ptr_;							\
		s_twi_state.rx_len = rx_sz_;									\
		s_twi_state.rx_len_init = rx_sz_;								\
		s_twi_state.op_slave_addr = (sl_addr_ << 1);					\
	} while(0)

/*************************************************************************/

/* TWI interrupt */
ISR(TWI_vect)
{
	switch(TWSR & TW_STATUS_MASK) {
	case TW_BUS_ERROR: {
		TWCR =
			0<<TWSTA|
			1<<TWSTO| /* send stop to bus in case of err */
			1<<TWINT|
			s_twi_state.slave_addr_set<<TWEA|
			1<<TWEN|
			1<<TWIE;

		s_twi_state.curr_st = twi_stop;

		if (s_twi_state.ev_cb)
			s_twi_state.ev_cb(twi_bus_fail, s_twi_state.ev_data);
		break;
	}

	/*
	 * Master transmit/receive
	 */

	/* Set slave addr with lsb wr/rd bit on START */
	case TW_START: {
		TWDR = (s_twi_state.curr_st == twi_recv ?
				s_twi_state.op_slave_addr | 0x01 :
				s_twi_state.op_slave_addr);
		TWCR =
			0<<TWSTA|
			0<<TWSTO|
			1<<TWINT|
			s_twi_state.slave_addr_set<<TWEA|
			1<<TWEN|
			1<<TWIE;
		break;
	}
	/* Set slave addr with lsb _rd_ bit on REPEATED START */
	case TW_REP_START: {
		TWDR = s_twi_state.op_slave_addr | 0x01;
		TWCR =
			0<<TWSTA|
			0<<TWSTO|
			1<<TWINT|
			s_twi_state.slave_addr_set<<TWEA|
			1<<TWEN|
			1<<TWIE;
		break;
	}
	/* Start data send on acked START of write op */
	case TW_MT_SLA_ACK: {
		TWDR = *s_twi_state.tx_ptr;

		++s_twi_state.tx_ptr;
		--s_twi_state.tx_len;

		TWCR =
			0<<TWSTA|
			0<<TWSTO|
			1<<TWINT|
			s_twi_state.slave_addr_set<<TWEA|
			1<<TWEN|
			1<<TWIE;
		break;
	}
	/* Rise error on nack of START of write op */
	case TW_MT_SLA_NACK: {
		TWCR =
			0<<TWSTA|
			1<<TWSTO| /* send stop to bus in case of err */
			1<<TWINT|
			s_twi_state.slave_addr_set<<TWEA|
			1<<TWEN|
			1<<TWIE;

		s_twi_state.curr_st = twi_stop;

		if (s_twi_state.ev_cb)
			s_twi_state.ev_cb(twi_mt_sla_nack, s_twi_state.ev_data);
		break;
	}
	/* Ack on byte already sent, continue sending data or send stop */
	case TW_MT_DATA_ACK: {
		uint8_t send_restart = 0;
		uint8_t send_stop = !!s_twi_state.tx_len;
		if (!send_stop) {
			TWDR = *s_twi_state.tx_ptr;

			++s_twi_state.tx_ptr;
			--s_twi_state.tx_len;
		}
		else
			send_restart = (s_twi_state.curr_st == twi_send_recv);

		TWCR =
			send_restart<<TWSTA|
			send_stop<<TWSTO|
			1<<TWINT|
			s_twi_state.slave_addr_set<<TWEA|
			1<<TWEN|
			1<<TWIE;

		if (send_stop) {
			s_twi_state.curr_st = twi_stop;

			if (s_twi_state.ev_cb)
				s_twi_state.ev_cb(twi_ok, s_twi_state.ev_data);
		}
		break;
	}
	/* Nack on byte already send, send stop */
	case TW_MT_DATA_NACK: {
		TWCR =
			0<<TWSTA|
			1<<TWSTO| /* send stop */
			1<<TWINT|
			s_twi_state.slave_addr_set<<TWEA|
			1<<TWEN|
			1<<TWIE;

		s_twi_state.curr_st = twi_stop;

		if (s_twi_state.ev_cb)
			s_twi_state.ev_cb(twi_mt_data_nack, s_twi_state.ev_data);
		break;
	}
	/* Arbitrage lost for master transmit/receive */
	case TW_MT_ARB_LOST:
  /*case TW_MR_ARB_LOST: */ {
		/* Reset tx pointer */
		s_twi_state.tx_ptr -=
			(s_twi_state.tx_len_init - s_twi_state.tx_len);
		s_twi_state.tx_len = s_twi_state.tx_len_init;
		/* Reset rx pointer */
		s_twi_state.rx_ptr -=
			(s_twi_state.rx_len_init - s_twi_state.rx_len);
		s_twi_state.rx_len = s_twi_state.rx_len_init;

		/* Retry start */
		TWCR =
			1<<TWSTA| /* start again on free bus */
			0<<TWSTO|
			1<<TWINT|
			s_twi_state.slave_addr_set<<TWEA|
			1<<TWEN|
			1<<TWIE;
		break;
	}
	/* Start data receive on acked START of read op */
	case TW_MR_SLA_ACK: {
		TWCR =
			0<<TWSTA|
			0<<TWSTO|
			1<<TWINT|
			(s_twi_state.rx_len > 1)<<TWEA| /* send nack on single byte */
			1<<TWEN|
			1<<TWIE;
		break;
	}
	/* Rise error on nack of START of read op */
	case TW_MR_SLA_NACK: {
		TWCR =
			0<<TWSTA|
			1<<TWSTO| /* send stop */
			1<<TWINT|
			s_twi_state.slave_addr_set<<TWEA|
			1<<TWEN|
			1<<TWIE;

		s_twi_state.curr_st = twi_stop;

		if (s_twi_state.ev_cb)
			s_twi_state.ev_cb(twi_mr_sla_nack, s_twi_state.ev_data);
		break;
	}
	/* Store already read byte */
	case TW_MR_DATA_ACK: {
		*s_twi_state.rx_ptr = TWDR;

		++s_twi_state.rx_ptr;
		--s_twi_state.rx_len;

		uint8_t send_nack = (s_twi_state.rx_len > 1);

		TWCR =
			0<<TWSTA|
			0<<TWSTO|
			1<<TWINT|
			send_nack<<TWEA|
			1<<TWEN|
			1<<TWIE;

		break;
	}
	/* Receive last byte */
	case TW_MR_DATA_NACK: {
		*s_twi_state.rx_ptr = TWDR;

		++s_twi_state.rx_ptr;
		--s_twi_state.rx_len;

		TWCR =
			0<<TWSTA|
			1<<TWSTO| /* send stop */
			1<<TWINT|
			s_twi_state.slave_addr_set<<TWEA|
			1<<TWEN|
			1<<TWIE;

		s_twi_state.curr_st = twi_stop;

		if (s_twi_state.ev_cb)
			s_twi_state.ev_cb(twi_ok, s_twi_state.ev_data);
		break;
	}

	/*
	 * TODO: Slave transmit, slave receive, multi-master
	 */

	default:
		LOG("Unknown state: %x\n", TWSR & TW_STATUS_MASK);
		return;
	}
}

/*************************************************************************/

twi_result twi_init(uint32_t twi_freq, twi_event_cb cb, void* data)
{
	memset(&s_twi_state, 0, sizeof(s_twi_state));
	s_twi_state.ev_cb = cb;
	s_twi_state.ev_data = data;

	/* setup TWI and pull-up resistors */
	TWI_PORT |= 1<<TWI_SCL|1<<TWI_SDA;
	TWI_DDR &= ~(1<<TWI_SCL|1<<TWI_SDA);

	/* bitrate */
	TWBR = F_CLK/(2*twi_freq) - 8; /* atmega16 datasheet, p178 */
	TWSR = 0;

	return twi_ok;
}

twi_result twi_listen(uint8_t slave_addr, uint8_t is_broadcast,
					  twi_recv_cb cb)
{
	//TODO:
	(void)cb;

	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		if (s_twi_state.curr_st != twi_stop)
			return twi_err;

		s_twi_state.slave_addr_set = (slave_addr || is_broadcast);
		if (s_twi_state.slave_addr_set) {
			TWAR = (slave_addr << 1)|(!!is_broadcast);

			/* Start listen to bus */
			TWCR =
				0<<TWSTA|
				0<<TWSTO|
				0<<TWINT|
				1<<TWEA|
				1<<TWEN|
				1<<TWIE;
		}
	}

	return twi_ok;
}

twi_result twi_master_send(uint8_t slave_addr, void* data, uint32_t sz)
{
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		if (s_twi_state.curr_st != twi_stop)
			return twi_err;
		if (!slave_addr || slave_addr & (1<<7))
			return twi_err;
		if (!data || sz == 0)
			return twi_err;

		TWI_INIT_OP(twi_send, slave_addr, data, sz, NULL, 0);

		/* Start op */
		TWCR =
			1<<TWSTA|
			0<<TWSTO|
			1<<TWINT|
			s_twi_state.slave_addr_set<<TWEA|
			1<<TWEN|
			1<<TWIE;
	}

	return twi_ok;
}

twi_result twi_master_recv(uint8_t slave_addr, void* data, uint32_t sz)
{
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		if (s_twi_state.curr_st != twi_stop)
			return twi_err;
		if (!slave_addr || slave_addr & (1<<7))
			return twi_err;
		if (!data || sz == 0)
			return twi_err;

		TWI_INIT_OP(twi_send, slave_addr, NULL, 0, data, sz);

		/* Start op */
		TWCR =
			1<<TWSTA|
			0<<TWSTO|
			1<<TWINT|
			s_twi_state.slave_addr_set<<TWEA|
			1<<TWEN|
			1<<TWIE;
	}

	return twi_ok;
}

twi_result twi_master_send_recv(uint8_t slave_addr,
								void* tx_ptr, uint32_t tx_sz,
								void* rx_ptr, uint32_t rx_sz)
{
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		if (s_twi_state.curr_st != twi_stop)
			return twi_err;
		if (!slave_addr || slave_addr & (1<<7))
			return twi_err;
		if (!tx_ptr || tx_sz == 0)
			return twi_err;
		if (!rx_ptr || rx_sz == 0)
			return twi_err;

		TWI_INIT_OP(twi_send_recv, slave_addr, tx_ptr, tx_sz, rx_ptr, rx_sz);

		/* Start op */
		TWCR =
			1<<TWSTA|
			0<<TWSTO|
			1<<TWINT|
			s_twi_state.slave_addr_set<<TWEA|
			1<<TWEN|
			1<<TWIE;
	}

	return twi_ok;
}

twi_result twi_submit_iov(twi_iov* iov, uint32_t iovcnt)
{
	(void)iov;
	(void)iovcnt;
	return twi_ok;
}

/*************************************************************************/
