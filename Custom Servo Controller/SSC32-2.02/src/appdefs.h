/***************************************************************************** 
** Copyright 2005-2006, Mike Dvorsky.
**	appdefs.h
**
**	Contains application-level definitions such as I/O definitions,
**	task times, etc.
**
**	SSC-32 has the following pin/function assignments:
**
**  Port Bit DIR Data  Function
**  ---- --- --- ----  ----------------------------------------------------
**
**    B   0  In    T   ICP, alternate receive
**    B   1  Out   0   Bank 1 RCK
**    B   2  Out   0   Bank 2 RCK
**    B   3  Out   0   MOSI
**    B   4  Out   0   Unused (debug)
**    B   5  Out   0   SCK
**    B   6  In    T   XTAL1
**    B   7  In    T   XTAL2
**
**    C   0  In    P   Spare A
**    C   1  In    P   Spare B
**    C   2  In    P   Spare C
**    C   3  In    P   Spare D
**    C   4  I/O   0   SDA
**    C   5  Out   0   SCL
**
**    D   0  In    T   RXD
**    D   1  Out   0   TXD
**    D   2  Out   1   LED control (0=on, 1=off)
**    D   3  In    P   Baud Select 1
**    D   4  In    P   Baud Select 0
**    D   5  Out   0   Unused (debug)
**    D   6  Out   0   Bank 3 RCK
**    D   7  Out   0   Bank 4 RCK
**
*****************************************************************************/

#ifndef _appdefs_h_
#define _appdefs_h_

#include "numtypes.h"

#include "compiler.h"

// Define for XTAL frequency
#define XTAL 14745600

// Defines to control conditional compilation
#define DEBUGCMD_ENABLE 0
#define REGRW_ENABLE 1
#define EEX_IS_EEI DEBUGCMD_ENABLE
#define FRACTION_ARG_ENABLE 0
#define HEXAPOD_SEQUENCER_ENABLE 0
#define BINARY_COMMAND_ENABLE 1
#define INTERNAL_EEPROM_ENABLE 1
#define EXTERNAL_EEPROM_ENABLE 1
#define GENERAL_SEQUENCER_ENABLE 1
#define BANKVAL_ENABLE 1
#define STARTUP_STRING_ENABLE 1
#define MODIFIER_ENABLE 1

#define EEPROM_ENABLE \
	( INTERNAL_EEPROM_ENABLE || EXTERNAL_EEPROM_ENABLE )

// Define to enable extended servo range.  Normal range is 750-2250, extended
// range is 500-2500 microseconds.
#define EXTENDED_RANGE 1

// Define to enable servo addresses 32-63.
#define EMUL_SERVO_32_63 0

// Define the delay inserted before starting to send a query result.
// 600us = 8847 cycles
#define DEFAULT_TX_DELAY_CYCLES 8847

// Define the delay inserted between bytes of a query result.
// 70us = 1032 cycles
#define DEFAULT_TX_PACING_DELAY_CYCLES 1032

// Number of sequence players
#define NUM_SEQ_PLAYERS 2

// Number of bytes in the RX and TX queues.  If these values are changed,
// the RX and TX ISRs must be updated to match.
#define RXQ_NBYTES 128
#define TXQ_NBYTES 32

// The number of bytes allocated for the startup string
#define STARTUP_STRING_MAX_BYTES 100

// Defines for LED control
#define LED_BIT PORT_BIT(PORTD, 2)
#define LED_ON 0
#define LED_OFF 1 

// Define for Baud Rate divisor rounded to the nearest integer
#define BAUD(rate, dbl) \
  ( (uint16_t)( XTAL / ((dbl) ? 8.0 : 16.0) / ( rate ) - 0.5 ) )

// Define to convert from microseconds to ticks, 16-bit
#define US_TO_TICKS(us) ( (uint16_t)( ( us ) / 1000000.0 * XTAL + 0.5 ) )

// Defines for Baud Select pins, PIND.3 and PIND.4.
#define BAUD_PIN PIND
#define BAUD_DDR DDRD
#define BAUD_PORT PORTD
#define BAUD_BIT_1 4
#define BAUD_BIT_0 3
#define BAUD_MASK 0x18
#define BAUD_SELECT_2400 0x18
#define BAUD_SELECT_9600 0x10
#define BAUD_SELECT_38400 0x08
#define BAUD_SELECT_115200 0x00

// Defines for shift register latches
#define BANK_1_RCK PORTB.1
#define BANK_2_RCK PORTB.2
#define BANK_3_RCK PORTD.6
#define BANK_4_RCK PORTD.7

#if defined(__CODEVISIONAVR__)

// Defines for EECR bit masks
/* EECT - EEPROM Control Register */
#define EEPM1   5
#define EEPM0   4
#define EERIE   3
#define EEMPE   2
#define EEPE    1
#define EERE    0

// Defines for I2C bit masks
#define TWINT   7
#define TWEA    6
#define TWSTA   5
#define TWSTO   4
#define TWWC    3
#define TWEN    2
#define TWIE    0

// Defines for Timer 1
#define ICF1    5
#define OCF1B   2
#define OCF1A   1
#define TOV1    0

/* TIMSK1 */
#define ICIE1   5
#define OCIE1B  2
#define OCIE1A  1
#define TOIE1   0

#define _BV(bit) (1 << bit)

// Defines for serial I/O
/* UCSR0A */
#define RXC0    7
#define TXC0    6
#define UDRE0   5
#define FE0     4
#define DOR0    3
#define UPE0    2
#define U2X0    1
#define MPCM0   0

#define RXC_BIT PORT_BIT(UCSR0A, RXC0)

#endif

// Mask for TOV0 bit of TIFR, bit 0
#define TOV0_MASK 0x01

// Port, pin, and mask defines for the ABCD pins
#define IN_A_PORT PORTC
#define IN_B_PORT PORTC
#define IN_C_PORT PORTC
#define IN_D_PORT PORTC
#define IN_A_PIN PINC
#define IN_B_PIN PINC
#define IN_C_PIN PINC
#define IN_D_PIN PINC
#define IN_A_MASK 0x01
#define IN_B_MASK 0x02
#define IN_C_MASK 0x04
#define IN_D_MASK 0x08

// Define for ADC
#define ADIF_MASK 0x10
#define ADSC_MASK 0x40

// Defines for external I2C EEPROM access
#define EEPROM_BUS_ADDRESS 0xa0
#define I2C_WRITE 0
#define I2C_READ 1
#define EEX_SLA_R_ACK  0x40
#define EEX_SLA_R_NACK  0x48
#define EEX_READ_ACK  0x50
#define TWI_READ_ACK  ( _BV(TWINT) | _BV(TWEN) | _BV(TWEA) )
#define TWI_READ_NACK ( _BV(TWINT) | _BV(TWEN) )


#endif // _appdefs_h_
