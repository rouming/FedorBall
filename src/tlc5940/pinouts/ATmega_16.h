#ifndef TLC_ATMEGA_16_H
#define TLC_ATMEGA_16_H

/** \file
    SPI and timer pins for the ATmega16(L).  Don't edit these.  All
    changeable pins are defined in tlc_config.h */

/** VPRG pin 20 -> VPRG (TLC pin 27) */
#define DEFAULT_VPRG_PIN    PD6
#define DEFAULT_VPRG_PORT   PORTD
#define DEFAULT_VPRG_DDR    DDRD

/** XERR pin 1 -> XERR (TLC pin 16) */
#define DEFAULT_XERR_PIN    PB0
#define DEFAULT_XERR_PORT   PORTB
#define DEFAULT_XERR_DDR    DDRB
#define DEFAULT_XERR_PINS   PINB

/** SIN pin 6 -> SIN (TLC pin 26) */
#define DEFAULT_BB_SIN_PIN      PB5
#define DEFAULT_BB_SIN_PORT     PORTB
#define DEFAULT_BB_SIN_DDR      DDRB
/** SCLK pin 8 -> SCLK (TLC pin 25) */
#define DEFAULT_BB_SCLK_PIN     PB7
#define DEFAULT_BB_SCLK_PORT    PORTB
#define DEFAULT_BB_SCLK_DDR     DDRB

/** MOSI pin 6 -> SIN (TLC pin 26) */
#define TLC_MOSI_PIN     PB5
#define TLC_MOSI_PORT    PORTB
#define TLC_MOSI_DDR     DDRB

/** SCK pin 8 -> SCLK (TLC pin 25) */
#define TLC_SCK_PIN      PB7
#define TLC_SCK_PORT     PORTB
#define TLC_SCK_DDR      DDRB

/** SS will be set to output as to not interfere with SPI master operation.
    If you have changed the pin-outs and the library doesn't seem to work
    or works intermittently, make sure this pin is set correctly.  This pin
    will not be used by the library other than setting its direction to
    output. */
#define TLC_SS_PIN       PB4
#define TLC_SS_DDR       DDRB

/** OC1A pin 19 -> XLAT (TLC pin 24) */
#define XLAT_PIN     PD5
#define XLAT_PORT    PORTD
#define XLAT_DDR     DDRD

/** OC1B pin 18 -> BLANK (TLC pin 23) */
#define BLANK_PIN    PD4
#define BLANK_PORT   PORTD
#define BLANK_DDR    DDRD

/** OC2 pin 21 -> GSCLK (TLC pin 18) */
#define GSCLK_PIN    PD7
#define GSCLK_PORT   PORTD
#define GSCLK_DDR    DDRD

#endif

