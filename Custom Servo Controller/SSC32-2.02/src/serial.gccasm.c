#line 1 "serial.c"
//*********************************************************************
// Serial port functions, TX/RX ISR.
// Copyright 2005-2006, Mike Dvorsky.
//*********************************************************************

#include <stdlib.h>
#line 7 "serial.c"
#include <ctype.h>
#line 8 "serial.c"
#include <string.h>
#line 9 "serial.c"

#include "compiler.h"
#line 11 "serial.c"

#include "numtypes.h"
#line 13 "serial.c"
#include "appdefs.h"
#line 14 "serial.c"
#include "globals.h"
#line 15 "serial.c"
#include "serial.h"
#line 16 "serial.c"

//*********************************************************************
// Global variables.
//*********************************************************************

// NOTE:  No global variables (or static) allowed in this file.  See 
// the note in globals.c for an explanation.

//*********************************************************************
// USART Receive Complete interrupt service routine
//*********************************************************************
#pragma savereg-
#line 28 "serial.c"
#if defined(__CODEVISIONAVR__)
#line 29 "serial.c"
interrupt [USART_RXC] void usart_rx_isr(void)
#elif defined(__GNUC__)
#line 31 "serial.c"
SIGNAL(USART_RX_vect)
#endif
#line 33 "serial.c"
{
	__asm__ __volatile__(";// Save __SREG__ in temporary register R14 and Z in temp registers R7:6");
	__asm__ __volatile__("in	R14, __SREG__");
	__asm__ __volatile__("movw	R6, R30");
	__asm__ __volatile__("");
	__asm__ __volatile__(";// Get serial status from UCSR0A and store in register R30");
	__asm__ __volatile__("lds	R30, 0xC0			; UCSR0A is at 0xC0 (mega168)");
	__asm__ __volatile__("");
	__asm__ __volatile__(";// Get byte from UDR0 and store on queue, using temp register R8");
	__asm__ __volatile__("lds	R8, 0xC6			; UDR0 is at 0xC6 (mega168)");
	__asm__ __volatile__("");
	__asm__ __volatile__(";// If serial status indicates parity, framing, or overrun error,");
	__asm__ __volatile__(";// discard the UDR value.");
	__asm__ __volatile__(";// Bit4 = FramingError, Bit3 = Overrun, Bit2 = Parity.");
	__asm__ __volatile__("andi	R30, 0x1C");
	__asm__ __volatile__("brne	usart_rx_isr_end");
	__asm__ __volatile__("");
	__asm__ __volatile__(";// Load Z with receive queue add pointer in R10/R11.");
	__asm__ __volatile__("movw	R30, R10");
	__asm__ __volatile__("");
	__asm__ __volatile__(";// Store received byte from R8 onto queue");
	__asm__ __volatile__("st	Z+, R8");
	__asm__ __volatile__("");
	__asm__ __volatile__(";// Wrap queue if needed.  Compare the LSB of the add pointer with");
	__asm__ __volatile__(";// address just past the queue.");
#if defined(__CODEVISIONAVR__)
#line 60 "serial.c"
	__asm__ __volatile__("cpi r30, lo8(_rx_queue+128)");
#elif defined(__GNUC__)
#line 62 "serial.c"
	__asm__ __volatile__("cpi r30, lo8(rx_queue+128)");
#endif
#line 64 "serial.c"
	__asm__ __volatile__("brne usart_rx_isr_cleanup");
	__asm__ __volatile__(";// Reset add pointer to beginning of queue");
	__asm__ __volatile__("subi    R30, lo8(128)");
	__asm__ __volatile__("sbci    R31, hi8(128)");
	__asm__ __volatile__("");
	__asm__ __volatile__("usart_rx_isr_cleanup:");
	__asm__ __volatile__(";// Store updated queue add pointer in R10/R11 from Z");
	__asm__ __volatile__("mov	R10, R30");
	__asm__ __volatile__("");
	__asm__ __volatile__("usart_rx_isr_end:");
	__asm__ __volatile__(";// Restore Z and __SREG__");
	__asm__ __volatile__("movw	R30, R6");
	__asm__ __volatile__("out	__SREG__, R14");
	__asm__ __volatile__("");
	__asm__ __volatile__(";// Return");
	__asm__ __volatile__("reti");
#line 81 "serial.c"
}
#pragma savereg+
#line 83 "serial.c"

//*********************************************************************
// USART TX Complete interrupt service routine.  Test the TX queue.
// If the queue is empty, then the last byte has been sent, so do
// nothing.  Otherwise there are more bytes, so set up the OC1B
// interrupt for the next byte.
//*********************************************************************
#pragma savereg-
#line 91 "serial.c"
#if defined(__CODEVISIONAVR__)
#line 92 "serial.c"
interrupt [USART_TXC] void usart_tx_isr(void)
#elif defined(__GNUC__)
#line 94 "serial.c"
ISR(USART_TX_vect)
#endif
#line 96 "serial.c"
{
	__asm__ __volatile__(";// Save __SREG__ in temporary register R14 and Z in temp registers R7:6");
	__asm__ __volatile__("in	R14, __SREG__");
	__asm__ __volatile__("movw	R6, R30");
	__asm__ __volatile__("");
	__asm__ __volatile__(";// If the queue is empty, disable OC1B interrupt and exit.  The condition");
	__asm__ __volatile__(";// for empty queue is that the number of bytes is 0.");
#if defined(__CODEVISIONAVR__)
#line 105 "serial.c"
	__asm__ __volatile__("lds     R8, _txq_nbytes");
#elif defined(__GNUC__)
#line 107 "serial.c"
	__asm__ __volatile__("lds     R8, txq_nbytes");
#endif
#line 109 "serial.c"
	__asm__ __volatile__("tst	R8");
	__asm__ __volatile__("breq	usart_tx_isr_disable");
	__asm__ __volatile__("");
	__asm__ __volatile__(";// Load temporary registers R9:8 with tx_pacing_delay_cycles");
#if defined(__CODEVISIONAVR__)
#line 114 "serial.c"
	__asm__ __volatile__("lds	R8, _tx_pacing_delay_cycles");
	__asm__ __volatile__("lds	R9, _tx_pacing_delay_cycles+1");
#elif defined(__GNUC__)
#line 117 "serial.c"
	__asm__ __volatile__("lds	R8, tx_pacing_delay_cycles");
	__asm__ __volatile__("lds	R9, tx_pacing_delay_cycles+1");
#endif
#line 120 "serial.c"
	__asm__ __volatile__("");
	__asm__ __volatile__(";// Set OCR1B to tx_pacing_delay_cycles in the future (add to TCNT1)");
	__asm__ __volatile__("lds	R30, 0x84			; TCNT1 (0x84 in mega168)");
	__asm__ __volatile__("lds	R31, 0x84+1");
	__asm__ __volatile__("add	R30, R8");
	__asm__ __volatile__("adc	R31, R9");
	__asm__ __volatile__("sts	0x8A+1, R31			; OCR1B (0x8A in mega168)");
	__asm__ __volatile__("sts	0x8A, R30");
	__asm__ __volatile__("");
	__asm__ __volatile__(";// Clear the OCF1B flag and enable the OCIE1B interrupt");
	__asm__ __volatile__("ldi	R30, 0x04			;// OCF1B is bit 2 of TIFR1 (0x16 in mega168)");
	__asm__ __volatile__("out	0x16, R30");
	__asm__ __volatile__("lds	R30, 0x6F			;// OCIE1B is bit 2 of TIMSK1 (0x6F in mega168)");
	__asm__ __volatile__("ori	R30, 0x04");
	__asm__ __volatile__("sts	0x6F, R30");
	__asm__ __volatile__("");
	__asm__ __volatile__("usart_tx_isr_end:");
	__asm__ __volatile__(";// Restore Z");
	__asm__ __volatile__("movw	R30, R6");
	__asm__ __volatile__("");
	__asm__ __volatile__(";// Restore __SREG__");
	__asm__ __volatile__("out	__SREG__, R14");
	__asm__ __volatile__("");
	__asm__ __volatile__(";// Return");
	__asm__ __volatile__("reti");
	__asm__ __volatile__("");
	__asm__ __volatile__("usart_tx_isr_disable:");
	__asm__ __volatile__(";// TX queue is empty, so disable the TXC interrupt and exit");
	__asm__ __volatile__("lds     R30, 0xC1   ;// TXCIE0 is bit 6 of UCSR0B (0xC1 in mega168)");
	__asm__ __volatile__("andi    R30, ~0x40");
	__asm__ __volatile__("sts     0xC1, R30");
	__asm__ __volatile__("rjmp	usart_tx_isr_end");
#line 153 "serial.c"
}
#pragma savereg+
#line 155 "serial.c"

//*********************************************************************
// Timer 1 output compare B interrupt service routine -- used to time
// the delay before and between transmitted bytes.
// This interrupt is only enabled when a byte is placed on the
// transmit queue, so it assumes the queue is not empty.
// Send the byte and disable the OC1B interrupt.
//********************************************************************* 
#pragma savereg-
#line 164 "serial.c"
#if defined(__CODEVISIONAVR__)
#line 165 "serial.c"
interrupt [TIM1_COMPB] void timer1_compb_isr(void)
#elif defined(__GNUC__)
#line 167 "serial.c"
ISR(TIMER1_COMPB_vect)
#endif
#line 169 "serial.c"
{
	__asm__ __volatile__(";// Save __SREG__ in temporary register R14 and Z in temp registers R7:6");
	__asm__ __volatile__("in	R14, __SREG__");
	__asm__ __volatile__("movw	R6, R30");
	__asm__ __volatile__("");
	__asm__ __volatile__(";// Load Z with transmit queue remove pointer in R12/R13.");
	__asm__ __volatile__("movw	R30, R12");
	__asm__ __volatile__("");
	__asm__ __volatile__(";// Get byte from queue and write to UDR0, using temp register R8");
	__asm__ __volatile__("ld	R8, Z+");
	__asm__ __volatile__("sts	0xC6, R8			; UDR0 is at 0xC6 in mega168");
	__asm__ __volatile__("");
	__asm__ __volatile__(";// Wrap queue if needed.  Compare the LSB of the remove pointer with");
	__asm__ __volatile__(";// address just past the queue.");
#if defined(__CODEVISIONAVR__)
#line 185 "serial.c"
	__asm__ __volatile__("cpi r30, lo8(_tx_queue+32)");
#elif defined(__GNUC__)
#line 187 "serial.c"
	__asm__ __volatile__("cpi r30, lo8(tx_queue+32)");
#endif
#line 189 "serial.c"
	__asm__ __volatile__("brne timer1_compb_isr_cleanup");
	__asm__ __volatile__(";// Reset remove pointer to beginning of queue");
	__asm__ __volatile__("subi    R30, lo8(32)");
	__asm__ __volatile__("sbci    R31, hi8(32)");
	__asm__ __volatile__("");
	__asm__ __volatile__("timer1_compb_isr_cleanup:");
	__asm__ __volatile__(";// Store updated queue add pointer in R12/R13 from Z");
	__asm__ __volatile__("movw	R12, R30");
	__asm__ __volatile__("");
	__asm__ __volatile__(";// Decrement the number of bytes on the queue (R13).  It is not");
	__asm__ __volatile__(";// necessary to check for an empty txq before decrementing since");
	__asm__ __volatile__(";// this interrupt is enabled only if the queue is non-empty.");
#if defined(__CODEVISIONAVR__)
#line 202 "serial.c"
	__asm__ __volatile__("lds     R8, _txq_nbytes");
#elif defined(__GNUC__)
#line 204 "serial.c"
	__asm__ __volatile__("lds     R8, txq_nbytes");
#endif
#line 206 "serial.c"
	__asm__ __volatile__("dec	R8");
#if defined(__CODEVISIONAVR__)
#line 208 "serial.c"
	__asm__ __volatile__("sts     _txq_nbytes, R8");
#elif defined(__GNUC__)
#line 210 "serial.c"
	__asm__ __volatile__("sts     txq_nbytes, R8");
#endif
#line 212 "serial.c"
	__asm__ __volatile__("");
	__asm__ __volatile__(";// Disable the interrupt for next time");
	__asm__ __volatile__("lds	R30, 0x6F			;// OCIE1B is bit 2 of TIMSK (0x39)");
	__asm__ __volatile__("andi	R30, ~0x04");
	__asm__ __volatile__("sts	0x6F, R30");
	__asm__ __volatile__("");
	__asm__ __volatile__(";// Restore Z");
	__asm__ __volatile__("movw	R30, R6");
	__asm__ __volatile__("");
	__asm__ __volatile__(";// Restore __SREG__");
	__asm__ __volatile__("out	__SREG__, R14");
	__asm__ __volatile__("");
	__asm__ __volatile__(";// Return");
	__asm__ __volatile__("reti");
#line 227 "serial.c"
}
#pragma savereg+
#line 229 "serial.c"


//*********************************************************************
// If there is a value in the RX queue, remove it and place it in the
// location passed.  If the queue is empty return FALSE, else return
// TRUE.
//*********************************************************************
boolean rx_get_char ( uint8_t * ch_ptr )
{
	// Return FALSE if queue is empty
	if ( rxq_remove_ptr == rxq_add_ptr )
	{
		return FALSE ;
	}

	// Get the character from the queue and store in the passed location.
	* ch_ptr = * rxq_remove_ptr ;

   	// Update the remove pointer, with interrupts disabled.
	cli();
   	if ( ++ rxq_remove_ptr == & rx_queue [ RXQ_NBYTES ] )
   	{
   	    rxq_remove_ptr = & rx_queue [ 0 ] ;
   	}
	sei();
    	 
   	// Set a flag indicating that a byte was received, used to control
   	// the LED.
   	rx_char_status = RX_CHAR_RECEIVED ;	

	// Return TRUE, indicating a valid character
	return TRUE ;
}
 

//*********************************************************************
// Add a character to the TX queue.  Do not check for a full queue.
// For the first byte, enable the Timer 1 Compare B interrupt and set
// the time for the configured delay amount to get the process started.
//
// The TXC interrupt will be enabled whenever a transmission is in
// progress, and disabled when no transmission is in progress.  The
// process for transmitting a string is as follows:
// 1.  tx_char enqueues the first byte, sets the OC1B timer, and
//     enables the OC1B interrupt and TXC interrupt.  tx_char enqueues
//     subsequent bytes but does not change the interrupt enables.
// 2.  The OC1B ISR dequeues a byte, transmits it, then disables the
//     OC1B interrupt.
// 3.  After the byte is transmitted, the TXC ISR is called.  If the
//     queue is empty (indicating the last byte has been transmitted),
//     it disables the TXC interrupt and exits.  Otherwise, it sets the
//     OC1B timer and enables the OC1B interrupt for the next byte.
//
// Any time the OC1B interrupt is enabled, the corresponding flag must
// be cleared.
//*********************************************************************
void tx_char ( uint8_t ch )
{
    uint8_t * txq_add_ptr ;
    uint16_t temp_u16 ;
    
	// Do nothing if txq full.
	if ( txq_nbytes >= TXQ_NBYTES )
	{
		return ;
	}

	// Disable interrupts to update nbytes and test for
	// transmits in progress.
	cli();

	// Store the passed character on the queue.  The add location
	// is not stored directly, but is calculated by adding the
	// remove location to the number of bytes on the queue.
	txq_add_ptr = (uint8_t *)(txq_remove_ptr + txq_nbytes) ;
	if ( txq_add_ptr >= & tx_queue [ TXQ_NBYTES ] )
	{
	    txq_add_ptr -= TXQ_NBYTES ;
	}
	* txq_add_ptr = ch ;

	// Update the number of bytes on the txq.
	++txq_nbytes ;

	// Test the TXC interrupt enable bit to determine if a transmission
	// is in progress.  If there is no transmit in progress, then treat
	// this as the first byte of a new transmission:  set up OCR1B for
	// the pre-transmit delay and enable the interrupt.	
	if ( ( UCSR0B & _BV(TXCIE0) ) == 0 )
	{
		// The interrupts related to serial transmission are disabled,
		// so this is a safe time to re-enable interrupts.
//	    sei();
	    // First byte sent tx_delay_cycles in the future.  When reading 16-bit
	    // registers, read the low byte first; when writing, write the high byte
	    // first.
	    temp_u16 = TCNT1L ;
	    temp_u16 |= ( (uint16_t)TCNT1H << 8 ) ;
	    temp_u16 += tx_delay_cycles ;
	    OCR1BH = temp_u16 >> 8 ;
	    OCR1BL = temp_u16 ;
	    TIFR1 = _BV(OCF1B) ; // Clear OCR1B match flag
	    UCSR0B |= _BV(TXCIE0) ;
		// Disable interrupts again while changing OCR1B interrupt mask
//	    cli();
	    TIMSK1 |= _BV(OCIE1B) ;
	}

	// Re-enable interrupts before returning.
	sei();
}
    
//*********************************************************************
// Set the transmit delay and pacing values.  Minimum values are 10us.
//*********************************************************************
void serial_set_tdl ( uint16_t tdl )
{
    // Clip to minimum of 10us
    if ( tdl < 10 )
    {
        tdl = 10 ;
    }
    
	// Convert command in microseconds to ticks.  Multiply by
	// 14.7456 = 59/4 = 59 >> 2 (approx).
	tx_delay_cycles = ( tdl >> 2 ) * 59 ;
}
void serial_set_tpa ( uint16_t tpa )
{
    // Clip to minimum of 10us
    if ( tpa < 10 )
    {
        tpa = 10 ;
    }
    
	// Convert command in microseconds to ticks.  Multiply by
	// 14.7456 = 59/4 = 59 >> 2 (approx).
	tx_pacing_delay_cycles = ( tpa >> 2 ) * 59 ;
}

//*********************************************************************
// Set the baud rate based on the BAUD inputs.
//*********************************************************************
#define TRY_125000 1
#line 373 "serial.c"

void set_baud (int16_t baudarg)
{
    static uint16_t prev_baud;
    uint16_t baud;
    boolean dbl = FALSE;

    if (baudarg != 0) {

	if (baudarg > 0) {
	    baud = baudarg;
	    baud = (XTAL / 100  +  (baud * 16) / 2) / (baud * 16) - 1;
	    dbl = FALSE;
	} else {
	    baud = -baudarg;
	    baud = (XTAL / 100  +  (baud * 8) / 2) / (baud * 8) - 1;
	    dbl = TRUE;
	}
    } else {
	switch ( BAUD_PIN & BAUD_MASK )
	{
	case BAUD_SELECT_115200:
	    baud = BAUD ( 115200, FALSE ) ;
	    break ;
	case BAUD_SELECT_38400:
	    baud = BAUD ( 38400, FALSE ) ;
	    break ;
	case BAUD_SELECT_9600:
	    baud = BAUD ( 9600, FALSE ) ;
	    break ;
	case BAUD_SELECT_2400:
	    // If both Baud select pins are high, then test whether
	    // they are jumpered together.  If they are jumpered
	    // together, use 115.2 kBaud, else use 2400 Baud.
	    PORT_BIT(BAUD_PORT, BAUD_BIT_1) = 0 ;
	    PORT_BIT(BAUD_DDR, BAUD_BIT_1) = 1 ;
	    PORT_BIT(BAUD_DDR, BAUD_BIT_1) = 1 ;		// 2 cycle delay to settle

	    if ( PORT_BIT(BAUD_PIN, BAUD_BIT_0) )
	    {
		baud = BAUD ( 2400, FALSE ) ;
	    }
	    else
	    {
		baud = BAUD ( 115200, FALSE ) ;
	    }
	    PORT_BIT(BAUD_DDR, BAUD_BIT_1) = 0 ;
	    PORT_BIT(BAUD_PORT, BAUD_BIT_1) = 1 ;
	    break ;
	default:  // BAUD_SELECT_2400 or an invalid value
	    baud = BAUD ( 2400, FALSE ) ;
	    break ;
	}
    }
    
    if ( baud != prev_baud ) {
	cli();
	if (dbl) {
	    PORT_BIT(UCSR0A, U2X0) = 1;
	} else {
	    PORT_BIT(UCSR0A, U2X0) = 0;
	}
	UBRR0H = baud >> 8 ;
	UBRR0L = baud ;
	sei();
    }
    
    prev_baud = baud ;
}
