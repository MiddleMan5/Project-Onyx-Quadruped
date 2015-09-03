//*********************************************************************
// Globals and functions for serial command processing.
// Copyright 2005-2006, Mike Dvorsky.
//*********************************************************************

#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "compiler.h"

#include "numtypes.h"
#include "appdefs.h"
#include "globals.h"
#include "cmdproc.h"
#include "hexapod.h"
#include "serial.h"
#include "gpseq.h"
#include "regrw.h"


//*********************************************************************
// Defines
//*********************************************************************

//*********************************************************************
// Global variables.
//*********************************************************************

/* mod_wait: delay execution of group move until a previous group-move
 * has been finished
 */
static boolean mod_wait;

/* mod_binary: until '\r' is received numbers are expected to be in
 * 16bit binary big-endian format.
 */
static boolean mod_binary;
static uint8_t binbytenum;

static void goto_parse_modifier(void);
static void process_parse_modifier(void);

// NOTE:  No global variables (or static) allowed in this file.  See 
// the note in globals.c for an explanation.

//*********************************************************************
//*********************************************************************
// Top-level state sequencer to process incoming serial commands.
//*********************************************************************
//*********************************************************************
void process_commands ( void )
{
  while ( rx_get_char ( & received_char ) )
  {
    rx_char_valid = TRUE ;
		
    while ( rx_char_valid )
    {
      // Go to appropriate state processing
      switch ( cmd_state )
      {
      case ST_WAIT_CMD:
	process_wait_cmd ( ) ;
	break ;
#if BINARY_COMMAND_ENABLE
      case ST_EMUL_SNUM:
	process_emul_snum ( ) ;
	break ;
      case ST_EMUL_PW:
	process_emul_pw ( ) ;
	break ;
#endif
      case ST_PARSE_MODIFIER:
	process_parse_modifier();
	break;
      case ST_PARSE_NAME:
	process_parse_name ( ) ;
	break ;
      case ST_PARSE_INT8_SIGN:
	process_parse_int8_sign ( ) ;
	break ;
      case ST_PARSE_INT8_INT:
	process_parse_int8_int ( ) ;
	break ;
      case ST_PARSE_INT16_SIGN:
	process_parse_int16_sign ( ) ;
	break ;
      case ST_PARSE_INT16_INT:
	process_parse_int16_int ( ) ;
	break ;
#if FRACTION_ARG_ENABLE
      case ST_PARSE_INT16_FRAC:
	process_parse_int16_frac ( ) ;
	break ;
#endif
#if STARTUP_STRING_ENABLE
      case ST_SSCAT:
	storech_sscat ( ) ;
	break ;
#endif
      default:
	cmd_type = CMDTYPE_NONE ;
	goto_wait_cmd ( ) ;
      }
    }
  }
}


//*********************************************************************
//*********************************************************************
// Functions to handle the different states in the state sequencer.
//*********************************************************************
//*********************************************************************
                                                                       
// Conditionally enable emulation commands (binary commands)
#if BINARY_COMMAND_ENABLE
//*********************************************************************
// Handle state ST_EMUL_SNUM.  Receive the servo number in SSC
// emulation mode.
//*********************************************************************
void process_emul_snum ( void )
{
	rx_ch_num = received_char ;
	cmd_state = ST_EMUL_PW ;
	rx_char_valid = 0 ;
}


//*********************************************************************
// Handle state ST_EMUL_PW.  Receive the servo speed in SSC
// emulation mode.  If the servo number was valid, store the speed.
//*********************************************************************
void process_emul_pw ( void )
{
	uint16_t pw ;

	goto_wait_cmd ( ) ;
	rx_char_valid = 0 ;

	if ( rx_ch_num > 31 )
	{
		return ;
	}
			
	if ( received_char <= 250 )
	{
		// Extended range mapping is
		//     0 => 500us => 500 * 14.7456
		//   250 => 2500us => 2500 * 14.7456
		// and the formula is
		//   pw = ( rxc * 8 + 500 ) * 14.7456
		//   pw = ( rxc * 8 + 500 ) * 14.7456 * 65536 / 65536
		//   pw = ( rxc * 8 * 14.7456 * 65536 + 500 * 14.7456 * 65536 ) / 65536
		pw = ( received_char * (uint32_t)( 65536 * 8 * 14.7456 )
			+ (uint32_t)( 65536 * 500 * 14.7456 + 32768 ) )
			>> 16 ;

		// Store in the pulse array for the selected channel
		pulse_array [ rx_ch_num ] . target_width = pw ;
		pulse_array [ rx_ch_num ] . current_width = pw ;
	}
	else
	{
		// Turn off the pulse
		pulse_array [ rx_ch_num ] . target_width = 0 ;
	}
}
#endif

//*********************************************************************
// Handle state ST_PARSE_NAME.  Receive the incoming characters one
// at a time, and match against the command strings in the parse
// table.  The search ends when a string is received that cannot
// be matched, or when a non-legal character is received and the
// string received so far exactly matches a command.  When there is a
// match, then the function for the command will be called.
//*********************************************************************
void goto_parse_name ( void )
{
	cmd_state = ST_PARSE_NAME ;
	cmd_name_index = 0 ;	
}
void process_parse_name ( void )
{
#if defined(__CODEVISIONAVR__)
	cmd_parse_table_entry_t flash * ptr_parse_table_entry ;
#elif defined(__GNUC__)
	cmd_parse_table_entry_t /*flash*/ * ptr_parse_table_entry ;
#endif
	uint8_t step_size ;
	int8_t strcmp_retcode ;
	int8_t prev_strcmp_retcode ;
	uint8_t up_char;

	// Convert to upper case
	up_char = toupper ( received_char ) ;
	
	/* Any character from '!' to '}' is legal in a name, except for
	 * 0-9, '+', '-', '.', '~'.
	 */
	if ( up_char >= '!' && up_char < '~'
		&& !( up_char >= '0' && up_char <= '9' )
		&& up_char != '+'
		&& up_char != '-'
		&& up_char != '.' )
	{
		// Remove character from RX queue
		rx_char_valid = 0 ;
		
		// Save character
		if ( cmd_name_index < MAX_CMD_NAME_LEN )
		{
			rx_cmd_name [ cmd_name_index ++ ] = up_char ;
		}
		else
		{
			// Length exceeds max, store invalid character to
			// cause a mismatch.
			rx_cmd_name [ 0 ] = 1 ;
		}
	}
	
	// A non-legal character has been received, possibly signaling
	// the end of the command name.
	else
	{
		// If this was the first character received, then don't
		// bother to search for a match.
		if ( cmd_name_index == 0 )
		{
			rx_char_valid = 0 ;
			goto_wait_cmd ( ) ;
			return ;
		}

		rx_cmd_name [ cmd_name_index ] = 0 ;	// String terminator

		// Search for the command name, using binary search.  This assumes the command
		// table is sorted.
		step_size = NUM_CMD_PARSE_TABLE_ENTRIES / 2 ;
		ptr_parse_table_entry =	& cmd_parse_table [ NUM_CMD_PARSE_TABLE_ENTRIES / 2 ] ;
		prev_strcmp_retcode = 0 ;
		while ( 1 )
		{
#if defined(__CODEVISIONAVR__)
			strcmp_retcode = strcmpf ( rx_cmd_name, ptr_parse_table_entry -> ptr_cmdstr ) ;
#elif defined(__GNUC__)
			strcmp_retcode =
			    strcmp_P(rx_cmd_name,
				     (uint8_t *)pgm_word(
					 ptr_parse_table_entry->ptr_cmdstr));
#endif
			if ( strcmp_retcode == 0 )
			{
				// Found the string, so call the function, which will determine
				// the next state.
				void ( * fp ) ( void ) ;
				fp = (void (*)(void))pgm_word(
				    ptr_parse_table_entry->ptr_function);
				( * fp ) ( ) ;
				break ;
			}
			else
			{
				if ( strcmp_retcode < 0 )
				{
					strcmp_retcode = -1 ;
				}
				else
				{
					strcmp_retcode = 1 ;
				}

				if ( step_size == 1 )
				{
					if ( strcmp_retcode != prev_strcmp_retcode )
					{
						// Failed to find string, quit.  If the strcmp return code
						// changes between two searches, that says the the string
						// must be between the two searches.  But if the step size
						// is 1, there is nothing between, so the search has failed.
						// This is the terminating condition for a failed search.
						// Sentinels as the first and last table entries
						// guarantee the search will end.
						goto_wait_cmd ( ) ;
						break ;
					}
				}
				else
				{
					step_size >>= 1 ;
				}

				if ( strcmp_retcode < 0 )
				{
					// Overshot, need to back up pointer
					ptr_parse_table_entry -= step_size ;
				}
				else
				{
					// Undershot, need to advance pointer
					ptr_parse_table_entry += step_size ;
				}
			}
			prev_strcmp_retcode = strcmp_retcode ;
		}
	}
}

//*********************************************************************
// Handle state ST_PARSE_INT8_SIGN.
//*********************************************************************
void goto_parse_int8_sign ( void )
{
	goto_parse_int16_sign();
	cmd_state = ST_PARSE_INT8_SIGN ; /* move to the correct state */
}

void process_parse_int8_sign ( void )
{
	process_parse_int16_sign();
	goto_parse_int8_int();
}

//*********************************************************************
// Handle state ST_PARSE_INT8_INT.
//*********************************************************************
void goto_parse_int8_int ( void )
{
	if (cmd_state == ST_PARSE_INT16_INT) {
		cmd_state = ST_PARSE_INT8_INT ;
	}
}

void process_parse_int8_int(void)
{
	binbytenum = 1; /* fake 2nd byte (little byte) receive */
	process_parse_int16_int();
}

//*********************************************************************
// Handle state ST_PARSE_INT16_SIGN.
//*********************************************************************
void goto_parse_int16_sign ( void )
{
	cmd_state = ST_PARSE_INT16_SIGN ;
	cmd_arg_positive = TRUE ;
	cmd_arg_int = 0 ;
//	cmd_arg_frac = 0 ;
	valid_digit = FALSE ;
	binbytenum = 0; /* for binary mode */
}

void process_parse_int16_sign ( void )
{
	if (mod_binary) {
		goto_parse_int16_int();
	} else if ( received_char == '+' ) {
		rx_char_valid = 0 ;
		goto_parse_int16_int ( ) ;
	} else if ( received_char == '-' ){
		cmd_arg_positive = FALSE ;
		rx_char_valid = 0 ;
		goto_parse_int16_int ( ) ;
	} else if ( received_char == ' ' ) {
		// Discard leading spaces
		rx_char_valid = 0 ;
	} else {
		goto_parse_int16_int ( ) ;
	}
}

//*********************************************************************
// Handle state ST_PARSE_INT16_INT.
//*********************************************************************
void goto_parse_int16_int ( void )
{
	cmd_state = ST_PARSE_INT16_INT ;
}

void process_parse_int16_int ( void )
{
  uint8_t new_digit ;
  boolean do_store = FALSE;
    
  if (mod_binary) {
    /* network byte-order, i.e. big-endian */
    cmd_arg_int <<= 8;
    cmd_arg_int |= received_char;
    if (binbytenum++ > 0) {
      valid_digit = TRUE;
      do_store = TRUE;
#if 0
      if (cmd_arg_int == 0) {
	tx_char('Z');
      }
      if (cmd_arg_int == 20) {
	tx_char('T');
      }
      if (cmd_arg_int == 1500) {
	tx_char('M');
      }
      qresult_int16(cmd_arg_int);
      do_query();
      tx_char('\n');
      tx_char('\r');
#endif
    }
    rx_char_valid = 0;
  } else if ( isdigit ( received_char ) ) {
    // If decimal digit received, incorporate into number
    rx_char_valid = 0 ;
		
    valid_digit = TRUE ;
		
    if ( cmd_arg_int > 6553 ) {
      // If multiply by 10 will cause overflow, then saturate
      cmd_arg_int = 65535 ;
    } else {
      new_digit = received_char - '0' ;
      cmd_arg_int *= 10 ;
      if ( new_digit > ( 65535 - cmd_arg_int ) ) {
	// If adding the new digit will cause ovf, saturate
	cmd_arg_int = 65535 ;
      } else {
	// Add new digit
	cmd_arg_int += new_digit ;
      }
    }
  }
#if 0
  else if ( received_char == '.' ) {
    // If decimal point received, get fraction part
    rx_char_valid = 0 ;
    goto_parse_int16_frac ( ) ;
  }
#endif
  else {
    do_store = TRUE;
  }

  if (do_store) {
    // Any character besides 0-9 or '.' signals the end of the
    // number.  Go to the wait_cmd state and call the function
    // pointer to store the received number if a valid number
    // was received.
    if ( valid_digit ) {
      ( * ptr_store_arg_func ) ( ) ;
    }
    goto_wait_cmd ( ) ;
  }
}

#if FRACTION_ARG_ENABLE
//*********************************************************************
// Handle state ST_PARSE_INT16_FRAC.
//*********************************************************************
void goto_parse_int16_frac ( void )
{
	cmd_state = ST_PARSE_INT16_FRAC ;
	num_frac_digits = 0 ;
Uncomment the init of cmd_arg_frac
}

void process_parse_int16_frac ( void )
{
	uint8_t new_digit ;
    
    // If decimal digit received, incorporate into number
	if ( isdigit ( received_char ) )
	{
		rx_char_valid = 0 ;
		
		if ( num_frac_digits < 4 )
		{
			new_digit = received_char - '0' ;
			cmd_arg_frac = cmd_arg_frac * 10 + new_digit ;
			++ num_frac_digits ;
		}
	}
	else
	{
		// Any character besides 0-9 signals the end of the
		// number.  Go to the wait_cmd state and call the function
		// pointer to store the received number if a valid number
		// was received.
		goto_wait_cmd ( ) ;
		
		// Convert the decimal digits received to a binary fraction
		// with scaling 2^16.
		cmd_arg_frac = ( cmd_arg_frac * pgm_byte(frac_mul[num_frac_digits]) ) >> 16 ;

		( * ptr_store_arg_func ) ( ) ;
	}
}
#endif

//*********************************************************************
// Handle state ST_WAIT_CMD.
//*********************************************************************
void goto_wait_cmd ( void )
{
	cmd_state = ST_WAIT_CMD ;
}

void process_wait_cmd ( void )
{
	// Mini-SSC emulation is treated as a special case
#if BINARY_COMMAND_ENABLE
	if ( received_char == 0xFF )
	{
		// 0xFF is the first byte of an emulation command
		rx_char_valid = 0 ;
		cmd_state = ST_EMUL_SNUM ;
	}
	else
#endif
	if ( received_char == ASCII_CR )
	{
		// For carriage return, execute the command
		rx_char_valid = 0 ;
		execute_command ( ) ;
	}
	else if ( received_char == ASCII_ESC )
	{
		/* For ESCape, kill the current command if any. This
		 * kills also any modifiers.
		*/
		rx_char_valid = FALSE ;
		mod_binary = mod_wait = FALSE;
		cmd_type = CMDTYPE_NONE ;
	}
	else if ( ( received_char == ' ' )
		|| ( received_char == ASCII_LF ) )
	{
		// Discard leading spaces and line feeds
		rx_char_valid = 0 ;
	}
	else
	{
		// For any character but 0xFF, ESC, or CR, start parsing the name
		goto_parse_name ( ) ;
	}
}

//*********************************************************************
//*********************************************************************
// Various utility functions.
//*********************************************************************
//*********************************************************************

//*********************************************************************
// Execute the command based on the command type.
//*********************************************************************
void execute_command ( void )
{
	switch ( cmd_type )
	{
		case CMDTYPE_PULSE :
			if (mod_wait) {
				/* Delay the execution until any
				 * previous command has finished
				 */
				move_pending = TRUE;
			} else {
				convert_servo_cmd_to_pulse_arr ( servo_cmd_index + 1, 0, rx_time ) ;
				ms_remaining_in_command = ms_in_command ;
			}
			break ;
		case CMDTYPE_GOBOOT :
			do_goboot ( ) ;
			break ;
		case CMDTYPE_QUERY :
			do_query ( ) ;
			break ;
#if HEXAPOD_SEQUENCER_ENABLE
		case CMDTYPE_XCMD :
			do_xcmd ( ) ;
			break ;
#endif
#if BANKVAL_ENABLE
		case CMDTYPE_BANKUPDATE :
			do_bankupdate ( ) ;
			break ;
#endif
		case CMDTYPE_OFFSET :
			do_offset ( ) ;
			break ;
#if STARTUP_STRING_ENABLE
		case CMDTYPE_SS :
			do_ss ( ) ;
			break ;
		case CMDTYPE_SSDELETE :
			do_ssdelete ( ) ;
			break ;
		case CMDTYPE_SSCAT :
			do_sscat ( ) ;
			break ;
#endif
#if EEPROM_ENABLE
		case CMDTYPE_EEW :
			do_eew ( ) ;
			break ;
#endif
#if GENERAL_SEQUENCER_ENABLE
		case CMDTYPE_PLAY_SEQ :
			do_gpseq ( ) ;
			break ;
		case CMDTYPE_GOTO :
			do_goto ( ) ;
			break ;
#endif
#if REGRW_ENABLE
		case CMDTYPE_REGRW :
			do_regrw ( ) ;
			break ;
		case CMDTYPE_RDFLT :
			do_rdflt ( ) ;
			break ;
#endif
		case CMDTYPE_SERVOSTOP :
			do_servostop ( ) ;
			break ;
		case CMDTYPE_BAUD :
			do_baud();
			break;
	}
	cmd_type = CMDTYPE_NONE ;

	/* Cancel any pending modifier */
	mod_wait = mod_binary = FALSE;
}

//*********************************************************************
// Parse through command array, adjusting the pulse array.
//*********************************************************************
void convert_servo_cmd_to_pulse_arr (
	uint8_t num_servos,
	uint8_t start_index,
	uint16_t cmd_time )
{
	int8_t count ;
	uint16_t speed ;
	uint32_t time ;
	uint16_t width ;
	servo_command_def_t * ptr_cmd ;
	pulse_def_t * ptr_pulse ;

	// For each element of the command array, set the speed to be the
	// minimum of the value in the command array and the pre-programmed
	// maximum speed for the channel.  Then, using the current width,
	// target width, and commanded speed, calculate the time for the
	// move at the commanded speed.  If this calculated time is greater
	// than the value in global cmd_time, then store it in cmd_time.
	for (
		count = num_servos, ptr_cmd = & servo_cmd_wks [ start_index ] ;
		count != 0 ;
		--count, ++ptr_cmd )
	{
		// Get a pointer to the indexed pulse
		ptr_pulse = & pulse_array [ ptr_cmd -> servo_num ] ;

		// Get width from command
		width = ptr_cmd -> target_width ;

		// Set the target width in the pulse array to the commanded
		// target width.  Discard the fractional part of the current
		// width.
		ptr_pulse -> target_width = width ;

        // If current width is 0, set it to target width
        if ( ptr_pulse -> current_width == 0 )
        {
        	ptr_pulse -> current_width = width ;
        }
		
		// Set speed to the minimum of the commanded speed and the
		// configured maximum speed for the channel.  (For now, just
		// use the commanded speed.  Channel configuration is not 
		// supported yet.)
		speed = ptr_cmd -> speed ;
		
		// Calculate the time (in ms) required to move from the current
		// width to the commanded target width at the commanded speed.
		// The formula is
		//   t = |targ - cur| * (1000/14.7456) / speed
		//     = |targ - cur| * (256*1000/14.7456) / speed / 256
		//     = |targ - cur| * 17361 / speed / 256
		// The target width needs to be adjusted by adding the pulse
		// offset (if any).  This is necessary to make the speed
		// calculation come out correctly.  The current_width already
		// takes the pulse offset into account.  Since pulse_offset is
		// in microseconds and width is in clock ticks, it is necessary
		// to multiply pulse_offset by 14.7456 to convert to ticks.
		// The approximation used is 14.7456 = 59/4 = 59 >> 2
		width = width
			+ ( ( (int16_t)pulse_offset_array [ ptr_cmd -> servo_num ] * 59 ) >> 2 )
			- ptr_pulse -> current_width ;

		if ( (int16_t)width < 0 )
		{
			width = -width ;
		}
		// Save the computed delta for later
		ptr_cmd -> speed = width ;
		 
		// If speed is 0, set time to max
		if ( speed == 0 )
		{
			time = 65535 ;
		}
		else
		{
			time = ( (uint32_t)width * 17361L / speed ) >> 8 ;
			if ( time > 65535 )
			{
				time = 65535 ;
			}
		}
		
		// Use max of time and cmd_time
		if ( time > cmd_time )	
	{
			cmd_time = time ;
		}
	}
	
	// Set global ticks_in_command to the total number of 20ms ticks for
	// this command.
	if ( ( ms_in_command = cmd_time ) == 0 )
	{
		ms_in_command = 1 ;
	}
	
	// Now re-compute the speed for each channel based on the total
	// time for the move and the distance to move.
	for (
		count = num_servos, ptr_cmd = & servo_cmd_wks [ start_index ] ;
		count != 0 ;
		--count, ++ptr_cmd )
	{
		// Get servo number and pointer to the indexed pulse
//		pulse_index = ptr_cmd -> servo_num ;
//		ptr_pulse = & pulse_array [ pulse_index ] ;
		ptr_pulse = & pulse_array [ ptr_cmd -> servo_num ] ;

		// Get the saved width delta
		width = ptr_cmd -> speed ;
		
		// Calculate the speed in clocks/20ms, which is the max_delta.
		// The formula is
		//   max_d = |targ - cur| * 20 / t
		//         = |targ - cur| * 20 * 256 / t / 256
		//         = |targ - cur| * 5120 / t / 256
		time = width * 5120L / cmd_time ;
		if ( time > 0x00FFFFFFL )
		{
			time = 0x00FFFFFFL ;
		}
		ptr_pulse -> max_delta_frac = time & 0xFF ;
		ptr_pulse -> max_delta = time >> 8 ;
	}
}

//*********************************************************************
// Transfer to boot
//*********************************************************************
void do_goboot ( void )
{
	// Jump to the last address in FLASH, which is word address
	// 8191 for an ATMega168 with 16K bytes FLASH.  This address
	// is the entry point for the boot loader.
#if defined(__CODEVISIONAVR__)
# asm
	cli			;// Must have interrupts disabled in boot
;	clr  R2		;// Boot loader expects R2=0
	jmp 8191	;// Entry point for boot loader
# endasm
#elif defined(__GNUC__)
	asm __volatile__("cli\n\t"
			 "clr\tr2\n\t"
			 "jmp\tcall_boot");
#endif
}

//*********************************************************************
// Send the result of a query command.
//*********************************************************************
void do_query ( void )
{
	uint8_t index ;
	
	for ( index = 0 ; index < cmd_wks . query_result . num_bytes ; ++ index )
	{
	    while ( txq_nbytes >= TXQ_NBYTES );
		tx_char ( cmd_wks . query_result . result_string [ index ] ) ;
	}
}

#if BANKVAL_ENABLE
//*********************************************************************
// Update the pulse array based on bank update commands
//*********************************************************************
void do_bankupdate ( void )
{
	uint8_t count ;
	uint8_t bank_num ;
	uint8_t byteval ;
	pulse_def_t * ptr_pulse ;
	bank_update_t * ptr_bank_update ;

	ptr_bank_update = & cmd_wks . bank_update_command [ 0 ] ;
	for ( bank_num = 0 ; bank_num < 4 ; ++ bank_num )
	{
		if ( ptr_bank_update -> modified )
		{
			ptr_pulse = & pulse_array [ bank_num << 3 ] ;
			byteval = ptr_bank_update -> bank_value ;
			for ( count = 8 ; count > 0 ; -- count )
			{
				if ( byteval & 0x01 )
				{
					ptr_pulse -> target_width = 65535 ;
				}
				else
				{
					ptr_pulse -> target_width = 0 ;
				}
				byteval >>= 1 ;
				++ ptr_pulse ;
			}
		}
		++ ptr_bank_update ;
	}
}
#endif

//*********************************************************************
// Update the offset array based on offset commands
//*********************************************************************
void do_offset ( void )
{
    uint8_t idx ;
/*
    Loop through all of the pulse offsets.  For each one that has
    changed, set the move speed delta to the
    maximum speed.  Copy the new pulse offset value.
*/
    for ( idx = 0 ; idx < 32 ; ++ idx )
    {
        if ( pulse_offset_array [ idx ]
            != cmd_wks . pulse_offset_command [ idx ] )
        {
            pulse_array [ idx ] . max_delta = 65535 ;
            pulse_offset_array [ idx ]
                = cmd_wks . pulse_offset_command [ idx ] ;
        }
    }
}

//*********************************************************************
// Command parsing functions.  These functions are called after an
// individual command name is recognized.
//*********************************************************************
void parse_offset ( void )
{
	ptr_store_arg_func = store_offset ;
	goto_parse_int8_sign ( ) ;
}

#if BANKVAL_ENABLE
void parse_bankvalue ( void )
{
	ptr_store_arg_func = store_bankvalue ;
	goto_parse_int8_sign ( ) ;
}
#endif

void parse_servonum ( void )
{
	ptr_store_arg_func = store_servonum ;
	goto_parse_int8_sign ( ) ;
}

void parse_goboot ( void )
{
	cmd_type = CMDTYPE_GOBOOT ;
	goto_wait_cmd ( ) ;
}

void parse_servohigh ( void )
{
	cmd_arg_int = 65535 ;
	store_servo_pw ( ) ;
	goto_wait_cmd ( ) ;
}

void parse_servolow ( void )
{
	cmd_arg_int = 0 ;
	store_servo_pw ( ) ;
	goto_wait_cmd ( ) ;
}

void parse_servopw ( void )
{
	ptr_store_arg_func = store_servo_pw ;
	goto_parse_int16_sign ( ) ;
}

void parse_querystatus ( void )
{
	if ( ms_remaining_in_command == 0 )
	{
		qresult_char ( '.' ) ;
	}
	else
	{
		qresult_char ( '+' ) ;
	}
	goto_wait_cmd ( ) ;
}

void parse_querypw ( void )
{
	ptr_store_arg_func = store_querypw ;
	goto_parse_int8_sign ( ) ;
}

void parse_servospeed ( void )
{
	ptr_store_arg_func = store_servo_speed ;
	goto_parse_int16_sign ( ) ;
}

void parse_servomovetime ( void )
{
	ptr_store_arg_func = store_move_time ;
	goto_parse_int16_sign ( ) ;
}

#if MODIFIER_ENABLE
static void goto_parse_modifier(void)
{
  cmd_state = ST_PARSE_MODIFIER;
}

static void process_parse_modifier(void)
{
  switch (received_char) {
  case '+':
  case '~':
    rx_char_valid = 0;
    switch (received_char) {
    case '+': mod_wait = TRUE; /* tx_char('W'); */ break;
    case '~': mod_binary = TRUE; /* tx_char('B'); */ break;
    }
    break;
  default:
    goto_wait_cmd();
    break;
  }
}

void parse_modifier(void)
{
  goto_parse_modifier();
}

#endif

void parse_ver ( void )
{
	qresult_string ( & swver [ 0 ] ) ;
	goto_wait_cmd ( ) ;
}

void store_baud(void)
{
	cmd_type = CMDTYPE_BAUD;
	goto_wait_cmd ( ) ;
}

void parse_baud(void)
{
	ptr_store_arg_func = store_baud;
	goto_parse_int16_sign();
}

void do_baud(void)
{
	set_baud (cmd_arg_positive ? cmd_arg_int : -cmd_arg_int);
}

//*********************************************************************
// Store the pulse width offset.
//*********************************************************************
void store_offset ( void )
{
	int8_t offset ;

	// Do nothing if invalid bank number	
	if ( rx_ch_num > 31 )
	{
		return ;
	}

	// For the first offset command of a command group, init the
	// command array to the current values.
	if ( cmd_type != CMDTYPE_OFFSET )
	{
		 memcpy ( & cmd_wks, pulse_offset_array, 32 ) ;
	}
	cmd_type = CMDTYPE_OFFSET ;

	if ( cmd_arg_int > 127 )
	{
		offset = 127 ;
	}
	else
	{
		offset = cmd_arg_int ;
	}
	
	if ( ! cmd_arg_positive )
	{
		offset = -offset ;
	}
	cmd_wks . pulse_offset_command [ rx_ch_num ] = offset ;
}

#if BANKVAL_ENABLE
//*********************************************************************
// Store the value for the bank number in rx_ch_num, if in the range
// 0-3.
//*********************************************************************
void store_bankvalue ( void )
{
	bank_update_t * ptr_cmd ;

	// Do nothing if invalid bank number	
	if ( rx_ch_num > 3 )
	{
		return ;
	}

	// For the first bankvalue command of a command group, clear the
	// command array.
	if ( cmd_type != CMDTYPE_BANKUPDATE )
	{
		memset ( & cmd_wks, 0, sizeof ( cmd_wks ) ) ;
	}
	cmd_type = CMDTYPE_BANKUPDATE ;

	ptr_cmd = & cmd_wks . bank_update_command [ rx_ch_num ] ;
	ptr_cmd -> modified = TRUE ;
	ptr_cmd -> bank_value = cmd_arg_int ;
}
#endif

//*********************************************************************
// Store the pulse width for the servo channel passed.  The pulse
// width stored is in units of 10us, and is stored as a binary byte.
//*********************************************************************
void store_querypw ( void )
{
	uint16_t pw ;

	if ( cmd_arg_int < 32 )
	{
		// Calculate pulse width in 10us increments from pulse width
		// in cycles (from pulse_array):
		//   pw = cycles / 14.7456 / 10
		//      = cycles * ( 65536 / 147.456 ) / 65536
		//      = ( cycles * 444 ) >> 16
		pw = ( pulse_array [ cmd_arg_int ] . current_width 
			* 444L + 32768 ) >> 16 ;
		if ( pw > 255 )
		{
			pw = 255 ;
		}
		qresult_char ( pw ) ;
	}
}

//*********************************************************************
// Store the servo channel number, if valid.  Otherwise store 255.
//*********************************************************************
void store_servonum ( void )
{
	if ( cmd_arg_int < 32 )
	{
		rx_ch_num = cmd_arg_int ;
	}
	else
	{
		rx_ch_num = 255 ;
	}
}

//*********************************************************************
// Store the move time.
//*********************************************************************
void store_move_time ( void )
{
	rx_time = cmd_arg_int ;
}

//*********************************************************************
// Store the servo pulse width, adjusted to a valid range.
//*********************************************************************
void store_servo_pw ( void )
{
	uint16_t pw ;
	servo_command_def_t * ptr_cmd ;
	
	// Do nothing if invalid servo number
	if ( rx_ch_num > 31 )
	{
		return ;
	}

	// For the first pulse command of a command group, clear the
	// command array.
	if ( cmd_type != CMDTYPE_PULSE )
	{
		rx_time = 0 ;
		servo_cmd_index = -1 ;
	}
	
	cmd_type = CMDTYPE_PULSE ;
	pw = cmd_arg_int ;
	
	if ( ( pw != 0 ) && ( pw != 65535 ) )
	{
		if ( pw < MIN_PULSE_WIDTH_US )
		{
			pw = MIN_PULSE_WIDTH_US ;
		}
		else if ( pw > MAX_PULSE_WIDTH_US )
		{
			pw = MAX_PULSE_WIDTH_US ;
		}

		// Convert to 14.7456MHz clock cycles
		pw = ( pw * (uint32_t)( 65536L * 14.7456 ) + 32768 ) >> 16 ;
	
	}
    
	// Advance to next servo command index, limit to range 0-31
	servo_cmd_index = ( servo_cmd_index + 1 ) & 31 ;
	
	ptr_cmd = & servo_cmd_wks [ servo_cmd_index ] ;
	ptr_cmd -> servo_num = rx_ch_num ;
	ptr_cmd -> speed = 65535 ;	// Default to max speed
	ptr_cmd -> target_width = pw ;
}

//*********************************************************************
// Store the servo speed.
//*********************************************************************
void store_servo_speed ( void )
{
	// Do nothing if invalid servo number
	if ( rx_ch_num > 31 )
	{
		return ;
	}
	
	servo_cmd_wks [ servo_cmd_index ] . speed = cmd_arg_int ;
}

//*********************************************************************
// Add a character to the result string for later transmission.
//*********************************************************************
void qresult_char ( uint8_t ch )
{
	uint8_t num_bytes ;

	num_bytes = cmd_wks . query_result . num_bytes ;

	if ( cmd_type != CMDTYPE_QUERY )
	{
		cmd_type = CMDTYPE_QUERY ;
		num_bytes = 0 ;
	}
	
	if ( num_bytes < MAX_QUERY_RESULT_BYTES )
	{
		cmd_wks . query_result . result_string [ num_bytes ++ ] = ch ;
	}
	
	cmd_wks . query_result . num_bytes = num_bytes ;
}

//*********************************************************************
// Add a string from flash to the result string for later transmission.
//*********************************************************************
void qresult_string ( flash_uint8_t * ch_str )
{
	uint8_t ch_index;
	uint8_t ch ;

	for ( ch_index = 0 ; ; ++ ch_index )
	{
		ch = pgm_byte(ch_str[ch_index]);
		if ( ch == 0 )
		{
			break ;
		}
		qresult_char ( ch ) ;
	}
}

//*********************************************************************
// Queue an unsigned, 16-bit, ASCII decimal number for later
// transmission.
//*********************************************************************
void qresult_uint16 ( uint16_t n )
{
    uint8_t first_digit = FALSE ;   // Set TRUE on first non-zero
    uint16_t pow10 = 10000 ;
    uint8_t digit ;
    
    while ( pow10 > 0 )
    {
        digit = n / pow10 ;
        if ( digit > 0 || first_digit || pow10 == 1 )
        {
            first_digit = TRUE ;
            qresult_char ( '0' + digit ) ;
        }
        n -= digit * pow10 ;
        pow10 /= 10 ;
    }
    qresult_char ( 0x0D ) ;
}

//*********************************************************************
// Queue a signed, 16-bit, ASCII decimal number for later
// transmission.
//*********************************************************************
void qresult_int16 ( int16_t n )
{
    if ( n < 0 )
    {
        qresult_char ( '-' ) ;
        n = -n ;
    }
    qresult_uint16 ( n ) ;
}

//*********************************************************************
// Functions for the SS (display startup string), SS+ (append to
// startup string), and SS- (delete from startup string) commands.
//*********************************************************************
#if STARTUP_STRING_ENABLE
void parse_ss ( void )
{
	goto_wait_cmd ( ) ;
	cmd_type = CMDTYPE_SS ;
}
void do_ss ( void )
{
    uint8_t idx ;
    uint8_t nbytes ;
    
    nbytes = eep_byte(startup_string_nbytes);
    if ( nbytes > STARTUP_STRING_MAX_BYTES )
    {
        nbytes = 0 ;
    }
    
    tx_char ( '"' ) ;
    for ( idx = 0 ; idx < nbytes ; ++ idx )
    {
        while ( txq_nbytes >= ( TXQ_NBYTES - 2 ) ) ;  // Spin till txq not full
        tx_char ( eep_byte(startup_string[idx]) ) ;
    }
    tx_char ( '"' ) ;
    tx_char ( ASCII_CR ) ;
}

void parse_sscat ( void )
{
	cmd_type = CMDTYPE_SSCAT ;
	cmd_state = ST_SSCAT ;
	cmd_wks . ss_idx = eep_byte(startup_string_nbytes);
	// If next char is blank, delete it.
	if ( received_char == ' ' )
	{
	    rx_char_valid = 0 ;
	}
}
void storech_sscat ( void )
{
    if ( ( received_char == ASCII_CR )
        || ( received_char == ASCII_ESC ) )
    {
        goto_wait_cmd ( ) ;
        return ;
    }
    
    if ( cmd_wks . ss_idx < STARTUP_STRING_MAX_BYTES )
    {
        eep_wbyte(startup_string[cmd_wks.ss_idx++], received_char);
    }
    
	rx_char_valid = 0 ;
}
void do_sscat ( void )
{
    eep_wbyte(startup_string_nbytes, cmd_wks.ss_idx);
}

void parse_ssdelete ( void )
{
	ptr_store_arg_func = store_ssdelete ;
	goto_parse_int16_sign ( ) ;
}
void store_ssdelete ( void )
{
	cmd_type = CMDTYPE_SSDELETE ;
	cmd_wks . ss_idx = eep_byte(startup_string_nbytes);
	if ( cmd_wks . ss_idx > cmd_arg_int )
	{
	    cmd_wks . ss_idx -= cmd_arg_int ;
	}
	else
	{
	    cmd_wks . ss_idx = 0 ;
	}
}
void do_ssdelete ( void )
{
    eep_wbyte(startup_string_nbytes, cmd_wks.ss_idx);
}
#endif

//*********************************************************************
// Functions for the STOP command.
//*********************************************************************
void parse_servostop ( void )
{
	ptr_store_arg_func = store_servostop ;
	goto_parse_int16_sign ( ) ;
}
void store_servostop ( void )
{
    if ( cmd_arg_int >= 32 )
    {
        return ;
    }
    
	cmd_type = CMDTYPE_SERVOSTOP ;
	cmd_wks . stop_idx = cmd_arg_int ;
}
void do_servostop ( void )
{
    stop_servo ( cmd_wks . stop_idx ) ;
}

//*********************************************************************
// Stop the servo whose number is passed.
//*********************************************************************
void stop_servo ( uint8_t servonum )
{
	pulse_def_t * ptr_pulse ;
	
	ptr_pulse = & pulse_array [ servonum ] ;
	ptr_pulse -> target_width = ptr_pulse -> current_width ;
}

