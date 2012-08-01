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

static struct
{
	uint8_t slave_addr_set;

	struct {
		twi_iov* iov;
		uint8_t iov_cnt;
		uint8_t iov_ind;
		twi_complete_cb cb;
		void* data;
	} io;

} s_twi_state;

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

		s_twi_state.io.iov_cnt = 0;

		if (s_twi_state.io.cb)
			s_twi_state.io.cb(twi_bus_fail, s_twi_state.io.data);
		break;
	}

	/*
	 * Master transmit/receive
	 */

	/* Set slave addr with lsb wr/rd bit on START */
	case TW_START: {
		twi_iov* iov = &s_twi_state.io.iov[s_twi_state.io.iov_ind];
		TWDR = (iov->dev_addr << 1) | iov->op;
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
		twi_iov* iov = &s_twi_state.io.iov[s_twi_state.io.iov_ind];
		TWDR = (iov->dev_addr << 1) | iov->op;
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
		twi_iov* iov = &s_twi_state.io.iov[s_twi_state.io.iov_ind];
		TWDR = iov->ptr[iov->done++];
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

		s_twi_state.io.iov_cnt = 0;

		if (s_twi_state.io.cb)
			s_twi_state.io.cb(twi_mt_sla_nack, s_twi_state.io.data);
		break;
	}
	/* Ack on byte already sent, continue sending data or send stop */
	case TW_MT_DATA_ACK: {
		twi_iov* iov = &s_twi_state.io.iov[s_twi_state.io.iov_ind];
		/* Send restart in case of next valid iov with different
		   dev addr or op */
		uint8_t send_restart = 0;
		/* Send stop in case of no bytes and last iov */
		uint8_t send_stop =
			!(iov->len - iov->done) &&
			/* Advance current iov index */
			s_twi_state.io.iov_cnt == ++s_twi_state.io.iov_ind;
		if (!send_stop) {
			/* Continue sending data for current iov */
			if (iov->len - iov->done)
				TWDR = iov->ptr[iov->done++];
			/* Get next iov */
			else {
				twi_iov* next_iov =
					&s_twi_state.io.iov[s_twi_state.io.iov_ind];
				/* If op or addr differ send restart */
				if (next_iov->op != iov->op ||
					next_iov->dev_addr != iov->dev_addr)
					send_restart = 1;
				/* Continue sending data for next iov */
				else
					TWDR = next_iov->ptr[next_iov->done++];
			}
		}

		TWCR =
			send_restart<<TWSTA|
			send_stop<<TWSTO|
			1<<TWINT|
			s_twi_state.slave_addr_set<<TWEA|
			1<<TWEN|
			1<<TWIE;

		if (send_stop) {
			s_twi_state.io.iov_cnt = 0;

			if (s_twi_state.io.cb)
				s_twi_state.io.cb(twi_ok, s_twi_state.io.data);
		}
		break;
	}
	/* Nack on byte already sent, send stop */
	case TW_MT_DATA_NACK: {
		TWCR =
			0<<TWSTA|
			1<<TWSTO| /* send stop */
			1<<TWINT|
			s_twi_state.slave_addr_set<<TWEA|
			1<<TWEN|
			1<<TWIE;

		s_twi_state.io.iov_cnt = 0;

		if (s_twi_state.io.cb)
			s_twi_state.io.cb(twi_mt_data_nack, s_twi_state.io.data);
		break;
	}
	/* Arbitrage lost for master transmit/receive */
	case TW_MT_ARB_LOST:
  /*case TW_MR_ARB_LOST: */ {
		/* Reset current iov */
		twi_iov* iov = &s_twi_state.io.iov[s_twi_state.io.iov_ind];
		iov->done = 0;

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
		twi_iov* iov = &s_twi_state.io.iov[s_twi_state.io.iov_ind];
		twi_iov* next_iov =
			(s_twi_state.io.iov_cnt == s_twi_state.io.iov_ind + 1 ?
			 NULL :
			 iov + 1);
		/* NACK will be sent on single byte and different next iov */
		uint8_t send_nack =
			(iov->len == 1) &&
			(!next_iov ||
			 next_iov->op != iov->op ||
			 next_iov->dev_addr != iov->dev_addr);

		TWCR =
			0<<TWSTA|
			0<<TWSTO|
			1<<TWINT|
			(!send_nack)<<TWEA|
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

		s_twi_state.io.iov_cnt = 0;

		if (s_twi_state.io.cb)
			s_twi_state.io.cb(twi_mr_sla_nack, s_twi_state.io.data);
		break;
	}
	/* Store already read byte */
	case TW_MR_DATA_ACK: {
		twi_iov* iov = &s_twi_state.io.iov[s_twi_state.io.iov_ind];
		twi_iov* next_iov =
			(s_twi_state.io.iov_cnt == s_twi_state.io.iov_ind + 1 ?
			 NULL :
			 iov + 1);

		iov->ptr[iov->done++] = TWDR;

		/* Check storing of last byte. Next iov must be valid. */
		if (iov->done == iov->len) {
			/* Advance iov */
			++s_twi_state.io.iov_ind;
			/* assert(next_iov); */
			iov = next_iov;
			next_iov =
				(s_twi_state.io.iov_cnt == s_twi_state.io.iov_ind + 1 ?
				 NULL :
				 iov + 1);
		}

		/* NACK will be sent on single byte and different next iov */
		uint8_t send_nack =
			(iov->len - iov->done == 1) &&
			(!next_iov ||
			 next_iov->op != iov->op ||
			 next_iov->dev_addr != iov->dev_addr);

		TWCR =
			0<<TWSTA|
			0<<TWSTO|
			1<<TWINT|
			(!send_nack)<<TWEA|
			1<<TWEN|
			1<<TWIE;

		break;
	}
	/* Receive last byte */
	case TW_MR_DATA_NACK: {
		twi_iov* iov = &s_twi_state.io.iov[s_twi_state.io.iov_ind];
		twi_iov* next_iov =
			/* Advance current iov index */
			(s_twi_state.io.iov_cnt == ++s_twi_state.io.iov_ind ?
			 NULL :
			 iov + 1);

		iov->ptr[iov->done++] = TWDR;
		/* assert(iov->done == iov->len); */

		TWCR =
			(!!next_iov)<<TWSTA| /* send restart if next iov is not null */
			(!next_iov)<<TWSTO|  /* send stop if next iov is null */
			1<<TWINT|
			s_twi_state.slave_addr_set<<TWEA|
			1<<TWEN|
			1<<TWIE;

		if (!next_iov) {
			s_twi_state.io.iov_cnt = 0;

			if (s_twi_state.io.cb)
				s_twi_state.io.cb(twi_ok, s_twi_state.io.data);
		}
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

twi_result twi_init(uint32_t twi_freq)
{
	memset(&s_twi_state, 0, sizeof(s_twi_state));

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
	return twi_not_impl;

	if (s_twi_state.io.iov_cnt)
		return twi_busy;

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

	return twi_ok;
}

twi_result twi_submit_iov(twi_iov* iov, uint8_t iovcnt,
						  twi_complete_cb cb, void* data)
{
	/* Check args */
	for (uint8_t i = 0; i < iovcnt; ++i) {
		twi_iov* iov_ptr = &iov[i];
		if (!iov_ptr->dev_addr || !iov->ptr || !iov->len)
			return twi_inval_args;
		iov_ptr->op &= 1; /* normalize op */
		iov_ptr->done = 0;
	}

	if (s_twi_state.io.iov_cnt)
		return twi_busy;

	/* Init io op */
	s_twi_state.io.iov = iov;
	s_twi_state.io.iov_cnt = iovcnt;
	s_twi_state.io.iov_ind = 0;
	s_twi_state.io.cb = cb;
	s_twi_state.io.data = data;

	/* Start op */
	TWCR =
		1<<TWSTA|
		0<<TWSTO|
		1<<TWINT|
		s_twi_state.slave_addr_set<<TWEA|
		1<<TWEN|
		1<<TWIE;

	return twi_ok;
}

/*************************************************************************/
