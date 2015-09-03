//*********************************************************************
// Pulse and edge processing, and edge ISR, for SSC-32.
// Copyright 2005-2006, Mike Dvorsky.
//*********************************************************************

#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "compiler.h"

#include "numtypes.h"
#include "appdefs.h"
#include "globals.h"
#include "pulsedge.h"
#include "serial.h"

//*********************************************************************
// Global variables.
//*********************************************************************

// NOTE:  No global variables (or static) allowed in this file.  See 
// the note in globals.c for an explanation.

//*********************************************************************
// Timer 1 output compare A interrupt service routine
//*********************************************************************
#pragma savereg-
#if defined(__CODEVISIONAVR__)
interrupt [TIM1_COMPA] void timer1_compa_isr(void)
#elif defined(__GNUC__)
    __asm__(".equ	_edge_array, edge_array");
ISR(TIMER1_COMPA_vect)
#endif
{
#asm
	;// Save R29:28, R31:30, and SREG in R6 to R14
	movw	R6, R28
	movw	R8, R30
	in		R14, SREG

	;// Load Y with the pointer to the next edge item in R3:2
	movw	R28, R2

	;// Load and output the next SPI data byte
	ld		R30, Y+		; SPI data byte
timer1_compa_isr_spi_output:
	out		0x2E, R30	; @ SPDR = 0x2E (mega168)

	;// Wait until edge time (+/- 2 cycles)
	ld		R30, Y+		; Low byte of edge time
timer1_compa_isr_wait_timer:
	lds		R31, 0x84	; Low byte of current time @ TCNT1L = 0x84 (mega168)
	sub		R31, R30
	brmi	timer1_compa_isr_wait_timer
	
	;// Load Z with function pointer and jump to function
	ld		R30, Y+		; Throw away high byte of edge time
	ld		R30, Y+		; Low byte of function pointer
	ld		R31, Y+		; High byte of function pointer
	ijmp

	.global bank1_pulse_loop
bank1_pulse_loop:
 	;// Pulse PORTB (0x05 in mega168) bit 1, the bank1 RCK pin
 	cbi		0x05, 1
	ld		R30, Y+		; Load next SPI data byte, 2 cycle delay		
 	sbi		0x05, 1
	rjmp	timer1_compa_isr_spi_output

	.global bank1_pulse
bank1_pulse:
 	;// Pulse PORTB (0x05 in mega168) bit 1, the bank1 RCK pin
 	cbi		0x05, 1
	ldd		R30, Y+0	; 2 cycle delay		
 	sbi		0x05, 1
	rjmp	timer1_compa_isr_finish

	.global bank2_pulse_loop
bank2_pulse_loop:
 	;// Pulse PORTB (0x05 in mega168) bit 2, the bank2 RCK pin
 	cbi		0x05, 2
	ld		R30, Y+		; Load next SPI data byte, 2 cycle delay		
	sbi		0x05, 2
	rjmp	timer1_compa_isr_spi_output

	.global bank2_pulse
bank2_pulse:
 	;// Pulse PORTB (0x05 in mega168) bit 2, the bank2 RCK pin
 	cbi		0x05, 2
	ldd		R30, Y+0	; 2 cycle delay		
	sbi		0x05, 2
	rjmp	timer1_compa_isr_finish

	.global bank3_pulse_loop
bank3_pulse_loop:
 	;// Pulse PORTD (0x0B in mega168) bit 6, the bank3 RCK pin
 	cbi		0x0B, 6
	ld		R30, Y+		; Load next SPI data byte, 2 cycle delay		
	sbi		0x0B, 6
	rjmp	timer1_compa_isr_spi_output

	.global bank3_pulse
bank3_pulse:
 	;// Pulse PORTD (0x0B in mega168) bit 6, the bank3 RCK pin
 	cbi		0x0B, 6
	ldd		R30, Y+0	; 2 cycle delay		
	sbi		0x0B, 6
	rjmp	timer1_compa_isr_finish

	.global bank4_pulse_loop
bank4_pulse_loop:
 	;// Pulse PORTD (0x0B in mega168) bit 7, the bank4 RCK pin
 	cbi		0x0B, 7
	ld		R30, Y+		; Load next SPI data byte, 2 cycle delay		
	sbi		0x0B, 7
	rjmp	timer1_compa_isr_spi_output

	.global bank4_pulse
bank4_pulse:
 	;// Pulse PORTD (0x0B in mega168) bit 7, the bank4 RCK pin
 	cbi		0x0B, 7
	ldd		R30, Y+0	; 2 cycle delay		
	sbi		0x0B, 7
	rjmp	timer1_compa_isr_finish

	.global no_pulse
no_pulse:
	;// If this is the final element of the array, advance all edges
	;// by 20ms % 65536 = 32768 clocks; then reset pointer.  The
	;// edge array has 40 entries of 5 bytes each.  It is only necessary
	;// to test the low byte, since the array is less than 256 bytes.
	cpi		R28, low ( _edge_array + 40*5 )
	brne	timer1_compa_isr_finish
	ldi		R28, low ( _edge_array )
	ldi		R29, high ( _edge_array )
update_ptrs_loop:
	ldd		R31, Y+2			; Load high byte of edge time
	subi	R31, high ( 32768 )	; Add 32768
	std		Y+2, R31			; Store
	adiw	R28, 5				; Advance pointer to next element
	cpi		R28, low ( _edge_array + 40*5 )
	brne	update_ptrs_loop
	;// Edge times all updated, reset pointer to start of edge_array
	ldi		R28, low ( _edge_array )
	ldi		R29, high ( _edge_array )	
	
timer1_compa_isr_finish:
	;// Get edge time of next pulse
	ldd		R30, Y+1
	ldd		R31, Y+2
	
	;// Store OCR1A time, which is 64 cycles before edge time of next pulse
	subi	R30, low ( 64 )		;// NCYLES_ISR_PRE_EDGE
	sbci	R31, high ( 64 )	;// NCYLES_ISR_PRE_EDGE
	sts		0x89, R31			;// OCR1AH = 0x89 (mega168)
	sts		0x88, R30			;// OCR1AL = 0x89 (mega168)
	
	;// Store edge array pointer from Y back into R3:2
	movw	R2, R28
	
	;// Save R29:28, R31:30, and SREG in R6 to R14
	movw	R28, R6
	movw	R30, R8
	out		SREG, R14

	;// Return from the interrupt	
	reti
	
#endasm
}
#pragma savereg+


//*********************************************************************
// Get the edge data pointer in an interrupt-safe manner.
//*********************************************************************
uint16_t get_edge_data_pointer ( void )
{
	uint16_t temp ;

    cli();
    temp = (uint16_t) edge_data_ptr ;
    sei();
    
    return temp ;
}

//*********************************************************************
// Move the pulse widths to the edge array, sorting by edge time.  Set
// the edge data for each edge, and compute the overall data for all
// edges in the bank.  The arguments to this function are the 
// index of the first pulse and first edge of the bank.
//*********************************************************************
void copy_pulses_to_sorted_edges (
	uint8_t starting_pulse_index,
	uint8_t starting_edge_index )
{
	pulse_def_t * ptr_pulse ;
	edge_def_t  * ptr_edge ;
	edge_def_t edge_temp ;
	uint8_t all_edges ;
	uint8_t edge_data ;
	uint8_t pulse_count ;
	uint8_t edge_counter ;
	uint16_t pulse_width ;
	
	// Pre-load edges with edge_time = 65535 (sentinel), and edge_data = 255.
	// This is done by filling the portion of the edge array with 0xFF.
	// During initial calculations, edge times are actually the pulse widths
	// in units of clock cycles, so an edge time of 65535 is invalid as an
	// actual time.  Only do the off-edges (1-8).
	edge_counter = starting_edge_index + 1 ;
	memset (
		& edge_array [ edge_counter ],
		0xFF,
		8 * sizeof ( edge_def_t ) ) ;

	// Insert pulse widths in edge array, in sorted order, using insertion
	// sort.  If two pulses have the same width, combine them into one
	// edge, with the data being the logical-OR of the individual edges.
    all_edges = 0xFF ;  // 0-bit for each channel that will have a pulse
	for (
		pulse_count = 0 ,
			ptr_pulse = & pulse_array [ starting_pulse_index ] ;
		pulse_count < 8 ;
		++ pulse_count, ++ ptr_pulse
		)
	{
		// Get the current pulse width for this pulse.
		pulse_width = ptr_pulse -> current_width ;

		// A pulse width of 0 is a signal to turn off the pulse, so in this
		// case skip this pulse.
		if ( pulse_width == 0 )
		{
			continue ;
		}

		// At this point we know that this pulse will be present.  Get the
		// edge data for this edge (just a single 0-bit), then AND the edge
		// data for this edge into the overall edge data.
		pulse_count += starting_pulse_index ;
		edge_data = pgm_byte(pulse_data_array[pulse_count]);
		pulse_count -= starting_pulse_index ;
		all_edges &= edge_data ;

		// A pulse width of 0xFFFF is a signal to turn the pulse on
		// continuously, so in this case skip the turn-off for this pulse.
		if ( pulse_width == 0xFFFF )
		{
			continue ;
		}
    		
   		// Round current_width to nearest multiple of MIN_EDGE_DELTA
   		pulse_width =
   			MIN_EDGE_DELTA *
   			( ( pulse_width + ( MIN_EDGE_DELTA / 2 ) ) / MIN_EDGE_DELTA ) ;
		
		// Insert the pulse using insertion sort.  First find the location
		// where it should be inserted.  Look for a location with edge_time
		// greater than or equal to the pulse being inserted.  Since the edge
		// array was pre-loaded with edge_time=65535, the search is guaranteed
		// to end.
		edge_counter = starting_edge_index + 1 ;
		ptr_edge = & edge_array [ edge_counter ] ;
		edge_counter = 8 ;
		while ( ptr_edge -> edge_time < pulse_width )
		{
			-- edge_counter, ++ ptr_edge ;
		}

		// Now ptr_edge points to the insertion location.  Shift the other
		// items out of the way and insert the new edge.
		if ( ptr_edge -> edge_time == pulse_width )
		{
			ptr_edge -> edge_data &= edge_data ;
		}
		else
		{
    		for ( ; edge_counter != 0 ; -- edge_counter, ++ ptr_edge )
    		{
    			// Save the values at the current location
    			edge_temp = * ptr_edge ;
    			// Load new values into current location
    			ptr_edge -> edge_time = pulse_width ;
    			ptr_edge -> edge_data = edge_data ;
    			// Get ready for the next iteration
    			pulse_width = edge_temp . edge_time ;
    			edge_data = edge_temp . edge_data ;
    		}
		}
	}

	// Set "on" edge data, which needs to have a 1-bit for each channel
	// that will have a pulse.
	edge_array [ starting_edge_index ] .edge_data = ~ all_edges ;
}

//*********************************************************************
// Continue processing edges.  Fix the edge data for each edge so it
// turns off the channel it is supposed to but does not affect the
// other channels.  Set the correct function pointer for each edge.
// Offset the edges from the on-edge time.  Arguments to this function
// are the index of the on-edge, and the index of the "normal"
// function pointer for the bank.
//*********************************************************************
void set_edge_data_and_function_ptr(uint8_t starting_edge_index, 
				    uint8_t function_ptr_normal_index )
{
	edge_def_t  * ptr_edge ;
	uint16_t edge_time ;
	uint8_t edge_data ;
	uint8_t edge_counter ;
	uint16_t starting_time ;

	// Initialize a pointer to the on-edge and retrieve the edge
	// time and edge data for the on-edge.  The edge data will have
	// a 1-bit for each channel that will have a pulse.
	ptr_edge = & edge_array [ starting_edge_index ] ;
	starting_time = ptr_edge -> edge_time ;	// Save "on" edge time
	edge_data = ptr_edge -> edge_data ;
	ptr_edge -> edge_time = 0 ;		// Edge times are 0-based
	
	// Adjust the edge data to be the logical-AND of this and all previous
	// edges.  Fix edge time for all unused off-edges.  Assume that prior
	// to this loop, ptr_edge points to the on-edge.  At the end of the
	// loop, ptr_edge will point to the last off_edge.
	for (
		edge_counter = 8, edge_time = 0 ;
		edge_counter != 0 ;
		-- edge_counter
		)
	{
		++ ptr_edge ;
	
		// The edge data for this edge has 0-bits for each channel that the
		// edge is supposed to turn off.  The overall edge_data variable has
		// the state of all channels prior to this edge.  Perform a logical
		// AND to get the state of all channels after this edge.
		edge_data &= ptr_edge -> edge_data ;
		ptr_edge -> edge_data = edge_data ;
                                                   
		// Any unused "dummy" edges will still have
		// the sentinel value of 65535.  For these dummy edges, just advance the
		// time by MIN_EDGE_DELTA cycles from the previous edge.  This will cause
		// the edges to be processed quickly by the ISR.
		if ( ptr_edge -> edge_time == 65535 )
		{
			ptr_edge -> edge_time = edge_time + MIN_EDGE_DELTA ;
		}
		edge_time = ptr_edge -> edge_time ;
	}
	
	// Set the function pointer for all edges based on the time between
	// edges.  Work backwards through edges.  Edge times are initially
	// 0-based.  Add the starting edge time to offset the edges from the "on"
	// edge time.  Assume ptr_edge initially points to the last off-edge.
	edge_time = 65535 ;	// Init to a value far beyond the last edge time
	for (
		edge_counter = 9 ;	// Do all edges in the bank, including on-edge
		edge_counter != 0 ;
		-- edge_counter , -- ptr_edge
		)
	{
		if ( ( edge_time - ptr_edge -> edge_time ) >= MIN_EDGE_DELTA_FOR_ISR_RET )
		{
		    // This edge is far enough from the next edge for the ISR to
		    // do a normal return after processing this edge.
#if defined(__CODEVISIONAVR__)
		    ptr_edge->function_ptr =
		        oc1a_isr_function_ptrs[function_ptr_normal_index];
#elif defined(__GNUC__)
		    ptr_edge->function_ptr =
			pgm_word(oc1a_isr_function_ptrs[
				     function_ptr_normal_index]);
#endif
		}
		else
		{
		    // This edge is too close to the next edge for a normal ISR
		    // return, so use the "loop" option.
#if defined(__CODEVISIONAVR__)                       
                    function_ptr_normal_index += 1;
		    ptr_edge->function_ptr =
			oc1a_isr_function_ptrs[function_ptr_normal_index];
                    function_ptr_normal_index -= 1;
#elif defined(__GNUC__)
		    ptr_edge->function_ptr =
			pgm_word(oc1a_isr_function_ptrs[
				 function_ptr_normal_index+1]);

#endif
		}
		edge_time = ptr_edge -> edge_time ;
		ptr_edge -> edge_time = edge_time + starting_time ;
	}
}

//*********************************************************************
// Initialize timer 1 and the edge_array structure to output pulses.
//*********************************************************************
void pulse_and_edge_init ( void )
{
  uint16_t temp_u16 ;
    
  // Initial edge array setup.
  // Init edge times for all on-edges.
  edge_array [  0 ] . edge_time = US_TO_CYCLES(BANK_TO_BANK_DELTA_US * 0);
  edge_array [  9 ] . edge_time = US_TO_CYCLES(BANK_TO_BANK_DELTA_US * 1);
  edge_array [ 18 ] . edge_time = (uint16_t)US_TO_CYCLES(BANK_TO_BANK_DELTA_US * 2);
  edge_array [ 27 ] . edge_time = (uint16_t)US_TO_CYCLES(BANK_TO_BANK_DELTA_US * 3);
  // Init edge times for no-pulse edges (these edges mark time during
  // the 10ms interval without pulses).
  edge_array [ 36 ] . edge_time = (uint16_t)US_TO_CYCLES(BANK_TO_BANK_DELTA_US * 4);
  edge_array [ 37 ] . edge_time = (uint16_t)US_TO_CYCLES(12000);
  edge_array [ 38 ] . edge_time = (uint16_t)US_TO_CYCLES(16000);
  edge_array [ 39 ] . edge_time = (uint16_t)US_TO_CYCLES(19900);
  // Init the function pointers for the no-pulse edges
  edge_array[36].function_ptr =
    edge_array[37].function_ptr =
    edge_array[38].function_ptr =
    edge_array[39].function_ptr = (uint16_t)no_pulse;

  // Init remaining edges based on no pulses
  copy_pulses_to_sorted_edges ( 0, 0 ) ;
  set_edge_data_and_function_ptr ( 0, 0 ) ;
  copy_pulses_to_sorted_edges ( 8, 9 ) ;
  set_edge_data_and_function_ptr ( 9, 2 ) ;
  copy_pulses_to_sorted_edges ( 16, 18 ) ;
  set_edge_data_and_function_ptr ( 18, 4 ) ;
  copy_pulses_to_sorted_edges ( 24, 27 ) ;
  set_edge_data_and_function_ptr ( 27, 6 ) ;

  // First OCR1A is at the 10ms point, after all servo pulses are done
  edge_data_ptr = (uint16_t) ( & edge_array [ 36 ] ) ;

  // Init TCNT1 and OCR1A.  Note that the order of accesses for high/low
  // bytes of these 16-bit registers is important.  High bytes must be
  // written first.  Assume interrupts are disabled.
  temp_u16 = edge_array [ 36 ] . edge_time - NCYLES_ISR_PRE_EDGE ;
  TCNT1H = temp_u16 >> 8 ;
  TCNT1L = temp_u16 ;
  OCR1AH = temp_u16 >> 8 ;
  OCR1AL = temp_u16 ;

  // Reset the output compare flag if set
  TIFR1 = _BV(OCF1A);
	
  // Enable the output compare interrupt
  TIMSK1 |= _BV(OCIE1A);
}

//*********************************************************************
// Scan through the pulse array, updating the pulse width for each
// pulse.  If the current pulse width is not equal to the target
// pulse width, add the delta value to the current pulse width.
//*********************************************************************
void update_pulse_array ( void )
{
	// These declarations must be placed in the order specified in order to
	// ensure that they are assigned to the registers specified.
	uint16_t current_width ;		// R17:16
	uint16_t max_delta ;			// R19:18
	uint8_t current_width_frac ;	// R20
	uint8_t max_delta_frac ;		// R21
	uint16_t target_width ;
	uint8_t counter ;
	pulse_def_t * ptr_pulse ;
	
	for (
		counter = 0, ptr_pulse = & pulse_array [ 0 ] ;
		counter < 32 ;
		++ counter, ++ ptr_pulse )
	{
		// Copy pulse data to local scalar variables
		target_width = ptr_pulse -> target_width ;
		current_width = ptr_pulse -> current_width ;
		current_width_frac = ptr_pulse -> current_width_frac ;
		max_delta = ptr_pulse -> max_delta ;
		max_delta_frac = ptr_pulse -> max_delta_frac ;
        
		// If target_width is 0 or 65535, that indicates that the channel
		// should be continuous 0 or 65535.  In this case, just set the
		// current_width to 0 or 65535 and continue to the next pulse.
		if ( target_width == 0 || target_width == 65535 )
		{
			current_width = target_width ;
		}
		// For all other target widths, move the current_width closer
		// to target_width by the value in max_delta.
		else
		{
			// Add the offset to the pulse width.  The offset is in us, so
			// convert to cycles by multiplying by 14.7456:
			//   off_cyc = offs * 14.7456 * 4 / 4
			//           = ( offs * 59 ) >> 2
			target_width += ( (int16_t) pulse_offset_array [ counter ] * 59 ) >> 2 ;
			
			// Clip target width to min/max range
			if ( target_width < MIN_PULSE_WIDTH_CYCLES )
			{
				target_width = MIN_PULSE_WIDTH_CYCLES ;                     
			}
			else if ( target_width > MAX_PULSE_WIDTH_CYCLES )
			{
				target_width = MAX_PULSE_WIDTH_CYCLES ;
			}
			// If current width is 0 or 65535, meaning that the pulse is just
			// being started, then set current width to target width.
			if ( current_width == 0 || current_width == 65535 )
			{
				current_width = target_width ;
			}
			// Move current width towards target_width
    		if ( target_width > current_width )
    		{
    			if ( ( target_width - current_width ) < max_delta )
	   			{
    				current_width = target_width ;
    				current_width_frac = 0 ;
    			}
    			else
    			{
	    			// current_width += max_delta
	    			// Add max_delta to current_width, using 24-bit addition with
	    			// current_width_frac and max_delta_frac being the LSB.
#if defined(__CODEVISIONAVR__)
				#asm
					add		R20, R21
					adc		R16, R18
					adc		R17, R19
				#endasm
#elif defined(__GNUC__)
/* With gcc we can do this in a cleaner way: */
__asm__ __volatile__("add\t%0,%2\n\t"
		     "adc\t%A1,%A3\n\t"
		     "adc\t%B1,%B3"
		     : "=r" (current_width_frac), "=r" (current_width)
		     : "r" (max_delta_frac), "r" (max_delta),
		       "0" (current_width_frac), "1" (current_width));
#endif
	    		}
    		}
    		else // current_width >= target_width
    		{
    			if ( ( current_width - target_width ) < max_delta )
    			{
    				current_width = target_width ;
    				current_width_frac = 0 ;
    			}
    			else
    			{
	    			// current_width -= max_delta
	    			// Subtract max_delta from current_width, using 24-bit math with
	    			// current_width_frac and max_delta_frac being the LSB.
#if defined(__CODEVISIONAVR__)
				#asm
					sub		R20, R21
					sbc		R16, R18
					sbc		R17, R19
				#endasm
#elif defined(__GNUC__)
/* With gcc we can do this in a cleaner way: */
__asm__ __volatile__("sub\t%0,%2\n\t"
		     "sbc\t%A1,%A3\n\t"
		     "sbc\t%B1,%B3"
		     : "=r" (current_width_frac), "=r" (current_width)
		     : "r" (max_delta_frac), "r" (max_delta),
		       "0" (current_width_frac), "1" (current_width));
#endif
	    		}
    		}
    	}
    			
   		// Write current_width back to pulse_array
   		ptr_pulse -> current_width = current_width ;
   		ptr_pulse -> current_width_frac = current_width_frac ;
	}
}
