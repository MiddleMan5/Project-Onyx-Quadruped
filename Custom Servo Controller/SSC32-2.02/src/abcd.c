//*********************************************************************
// Routines to manage the ABCD input pins in SSC-32.
// Copyright 2005, Mike Dvorsky.
//*********************************************************************

#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "compiler.h"

#include "numtypes.h"
#include "appdefs.h"
#include "globals.h"
#include "abcd.h"


//*********************************************************************
// Global variables.
//*********************************************************************

// NOTE:  No global variables (or static) allowed in this file.  See 
// the note in globals.c for an explanation.

//*********************************************************************
// Debounce the pin.  The arguments are a pointer to a flags byte, and
// the current pin value.  The flags byte is laid out as follows:
//   Bit 7 = debounced value
//   Bit 6 = 1 if changed
//   Bit 5 = 1 if falling edge
//   Bit 4 = 1 if rising edge
//   Bits 3:0 = debounce counter, counts from 15 to 0
//*********************************************************************
void debounce_pin ( uint8_t * ptr_flags, uint8_t raw_value )
{
	uint8_t flags ;
	
	flags = * ptr_flags ;

	// Test raw value against debounced value	
	if ( ( ( flags & DBNC_VALUE_MASK ) == 0 )
		== ( raw_value == 0 ) )
	{
		// Raw and debounced values match, reset count
		flags |= DBNC_COUNT_MASK ;
	}
	else
	{
		// Raw and debounced value different, decrement count
		if ( ( -- flags & DBNC_COUNT_MASK ) == 0 )
		{
			// Count reached 0, new value has been present for 15
			// consecutive times, so it becomes the new debounced
			// value
			if ( raw_value == 0 )
			{
				// Debounced to 0
				flags &= ~ DBNC_VALUE_MASK ;
				flags |= DBNC_CHANGED_MASK
					| DBNC_FALLING_EDGE_MASK
					| DBNC_COUNT_MASK ;
			}
			else
			{
				// Debounced to 1
				flags |= DBNC_VALUE_MASK
					| DBNC_CHANGED_MASK
					| DBNC_RISING_EDGE_MASK
					| DBNC_COUNT_MASK ;
			}
		}
	}
	
	* ptr_flags = flags ;
}

//*********************************************************************
// If the pin is a discrete input, debounce.
//*********************************************************************
void debounce_abcd ( void )
{
	// If the pin is pulled up, then treat it as a discrete input
	if ( IN_A_PORT & IN_A_MASK )
	{
		debounce_pin ( & in_A_dbnc_flags, IN_A_PIN & IN_A_MASK ) ;
	}
	if ( IN_B_PORT & IN_B_MASK )
	{
		debounce_pin ( & in_B_dbnc_flags, IN_B_PIN & IN_B_MASK ) ;
	}
	if ( IN_C_PORT & IN_C_MASK )
	{
		debounce_pin ( & in_C_dbnc_flags, IN_C_PIN & IN_C_MASK ) ;
	}
	if ( IN_D_PORT & IN_D_MASK )
	{
		debounce_pin ( & in_D_dbnc_flags, IN_D_PIN & IN_D_MASK ) ;
	}
}

//*********************************************************************
// If the pin is an analog input, read and filter.
//*********************************************************************
void read_v_abcd ( void )
{
	uint8_t next_channel_is_digital ;
	uint16_t * p_adc_filt ;
	
	// If a conversion is in progress, return
	if ( ADCSRA & ADSC_MASK )
	{
		return ;
	}
	
	// Get a pointer to the result channel filtered value
	switch ( ADMUX )
	{
		case 0:
			next_channel_is_digital =  IN_B_PORT & IN_B_MASK ;
			p_adc_filt = & va ;
			break ;
		case 1:
			next_channel_is_digital =  IN_C_PORT & IN_C_MASK ;
			p_adc_filt = & vb ;
			break ;
		case 2:
			next_channel_is_digital =  IN_D_PORT & IN_D_MASK ;
			p_adc_filt = & vc ;
			break ;
		default:
			next_channel_is_digital =  IN_A_PORT & IN_A_MASK ;
			p_adc_filt = & vd ;
			break ;
	}
	
	// If there has been a conversion completed, mix the raw value
	// with the filtered value:
	//   filt_val = filt_val * 0.75 * raw_val * 0.25
	if ( ADCSRA & ADIF_MASK )
	{
		* p_adc_filt = ( ( * p_adc_filt * 3 ) + ADCW ) >> 2 ;
		ADCSRA |= ADIF_MASK ;	// Clear the conversion complete flag
	}
	
	// Advance the AD MUX to the next channel
	ADMUX = ( ADMUX + 1 ) & 0x03 ;
	
	// If the next channel is configured for analog input, start
	// a conversion
	if ( ! next_channel_is_digital )
	{
		ADCSRA |= ADSC_MASK ;
	}
}

//*********************************************************************
// Store the ABCD value passed
//*********************************************************************
void store_abcd_value ( uint8_t query_result )
{
	goto_wait_cmd ( ) ;

	qresult_char ( query_result ? '1' : '0' ) ;
}

//*********************************************************************
// Parse functions for ABCD
//*********************************************************************
void parse_avalue ( void )
{
	IN_A_PORT |= IN_A_MASK ;	// Enable pullup
	store_abcd_value ( in_A_dbnc_flags & DBNC_VALUE_MASK ) ;
}
void parse_bvalue ( void )
{
	IN_B_PORT |= IN_B_MASK ;	// Enable pullup
	store_abcd_value ( in_B_dbnc_flags & DBNC_VALUE_MASK ) ;
}
void parse_cvalue ( void )
{
	IN_C_PORT |= IN_C_MASK ;	// Enable pullup
	store_abcd_value ( in_C_dbnc_flags & DBNC_VALUE_MASK ) ;
}
void parse_dvalue ( void )
{
	IN_D_PORT |= IN_D_MASK ;	// Enable pullup
	store_abcd_value ( in_D_dbnc_flags & DBNC_VALUE_MASK ) ;
}
void parse_alatch ( void )
{
	IN_A_PORT |= IN_A_MASK ;	// Enable pullup
	store_abcd_value (
		( ( in_A_dbnc_flags ^ DBNC_VALUE_MASK )
		& ( DBNC_RISING_EDGE_MASK | DBNC_VALUE_MASK ) ) == 0 ) ;
	in_A_dbnc_flags &= ~DBNC_RISING_EDGE_MASK ;
}
void parse_blatch ( void )
{
	IN_B_PORT |= IN_B_MASK ;	// Enable pullup
	store_abcd_value (
		( ( in_B_dbnc_flags ^ DBNC_VALUE_MASK )
		& ( DBNC_RISING_EDGE_MASK | DBNC_VALUE_MASK ) ) == 0 ) ;
	in_B_dbnc_flags &= ~DBNC_RISING_EDGE_MASK ;
}
void parse_clatch ( void )
{
	IN_C_PORT |= IN_C_MASK ;	// Enable pullup
	store_abcd_value (
		( ( in_C_dbnc_flags ^ DBNC_VALUE_MASK )
		& ( DBNC_RISING_EDGE_MASK | DBNC_VALUE_MASK ) ) == 0 ) ;
	in_C_dbnc_flags &= ~DBNC_RISING_EDGE_MASK ;
}
void parse_dlatch ( void )
{
	IN_D_PORT |= IN_D_MASK ;	// Enable pullup
	store_abcd_value (
		( ( in_D_dbnc_flags ^ DBNC_VALUE_MASK )
		& ( DBNC_RISING_EDGE_MASK | DBNC_VALUE_MASK ) ) == 0 ) ;
	in_D_dbnc_flags &= ~DBNC_RISING_EDGE_MASK ;
}
void parse_va ( void )
{
	IN_A_PORT &= ~IN_A_MASK ;	// Disable pullup
	qresult_char ( va >> 2 ) ;	// 8 bits
	goto_wait_cmd ( ) ;
}
void parse_vb ( void )
{
	IN_B_PORT &= ~IN_B_MASK ;	// Disable pullup
	qresult_char ( vb >> 2 ) ;	// 8 bits
	goto_wait_cmd ( ) ;
}
void parse_vc ( void )
{
	IN_C_PORT &= ~IN_C_MASK ;	// Disable pullup
	qresult_char ( vc >> 2 ) ;	// 8 bits
	goto_wait_cmd ( ) ;
}
void parse_vd ( void )
{
	IN_D_PORT &= ~IN_D_MASK ;	// Disable pullup
	qresult_char ( vd >> 2 ) ;	// 8 bits
	goto_wait_cmd ( ) ;
}
