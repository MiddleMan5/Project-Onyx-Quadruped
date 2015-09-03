//*********************************************************************
// Routines for a general purpose sequencer for the SSC-32.
// Copyright 2006, Mike Dvorsky.
//
// Supported command strings:
// "PL p <cr>"	-- stops a player
// "PL p SQ sss SM mmm IX xxx PA ttt <cr>"	-- starts a player
// "PL p SQ sss <cr>"	-- changes the sequence for a player
// "PL p SM mmm <cr>"	-- changes speed multiplier for a player
// "PL p PA ttt <cr>"	-- changes pause time for a player
// "QPL p"	-- returns information about a player
//
// PL p -- specifies player number p, from 0 to 3
// SQ sss -- specifies sequence number sss, from 0 to 127
// SM mmm -- specifies speed multiplier mmm, from -200 to 200
//			 (optional, default=100)
// PA ttt -- specifies the amount of time ttt to pause between steps,
//			 from 0 to 65535 milliseconds
//			 (optional, default=0)
// IX xxx -- specifies starting step index, from 0 to 255
//			 (optional, default=0)
// QPL p -- returns 4 bytes of information about player p:
// 		Byte 0 = the current sequence, or 255 if no sequence is playing
// 		Byte 1 = the index moving from in the sequence (0 if no sequence)
// 		Byte 2 = the index moving to in the sequence (0 if no sequence)
// 		Byte 3 = time remaining in move, 100ms/bit (0 if no sequence)
//
// p, sss, mmm, ttt, and xxx are all ASCII decimal numbers.  The returned
// data for a QPL command is 4 bytes in binary.  The IX command is
// only valid when starting a player; it is ignored when changing
// the sequence or speed multiplier for a player.
//
// The memory map of the sequences in external EEPROM is as follows:
// 0-1:    Address of sequence 0
// 2-3:    Address of sequence 1
// ...
// 254-255:  Address of sequence 127
// 256-32767:  Sequence data
//
// Sequence:
//   Header (3 bytes)
//     Sequence number, 0-127
//     Number of servos (0-31)
//     Number of setps = M
//   List of servos and max speeds (3*N bytes)
//     1st servo number
//     1st servo speedH
//     1st servo speedL
//     2nd servo number
//     2nd servo speedH
//     2nd servo speedL
//     ...
//     Nth servo number
//     Nth servo speedH
//     Nth servo speedL
//   Step M to 1 time (2 bytes)
//   Step 1 (2 bytes per servo)
//   Step 1 to 2 time
//   Step 2
//   Step 2 to 3 time
//   ...
//   Step M-1 to M time
//   Step M
//   Step M to 1 time (repeated)
//
// The bytes of each step are encoded as follows:
//   1st servo pulse width high byte
//   1st servo pulse width low byte
//   2nd servo pulse width high byte
//   2nd servo pulse width low byte
//   ...
//   Nth servo pulse width high byte
//   Nth servo pulse width low byte
//
// Servo max speeds are in us/sec; servo pulse widths are in us; move
// times are in ms.
//*********************************************************************

#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "compiler.h"

#include "numtypes.h"
#include "appdefs.h"
#include "globals.h"
#include "gpseq.h"

//*********************************************************************
// Global variables.
//*********************************************************************

// NOTE:  No global variables (or static) allowed in this file.  See 
// the note in globals.c for an explanation.

#if GENERAL_SEQUENCER_ENABLE

//*********************************************************************
// Perform periodic processing for gpseq.
//*********************************************************************
void process_gpseq ( void )
{
	if ( check_player_num < NUM_SEQ_PLAYERS )
	{
    	// Update the player indexed by global variable check_player_num
    	update_player ( ) ;
    	++ check_player_num ;
 	}
}


//*********************************************************************
// Return the previous step number for the current player.
//*********************************************************************
uint8_t gpseq_prev_step_num ( seq_player_control_t * ptr_player )
{
    uint8_t prev_step_num ;
    
    prev_step_num = ptr_player -> step_num ;
	if ( ptr_player -> speed < 0 )
	{
	    ++ prev_step_num ;
	    if ( prev_step_num >= ptr_player -> num_steps )
	    {
	        prev_step_num = 0 ;
	    }
	}
	else
	{
	    if ( prev_step_num == 0 )
	    {
	        prev_step_num = ptr_player -> num_steps ;
	    }
	    -- prev_step_num ;
	}
	
	return prev_step_num ;
}

//*********************************************************************
// Get the servo numbers and speeds for the passed player into the
// command array.
//*********************************************************************
void gpseq_get_servo_num_speed ( seq_player_control_t * ptr_player )
{
    servo_command_def_t * ptr_cmd ;
    uint8_t count ;

    // Get address of servo command structure.  GPSEQ commands grow
    // from the last element downwards.    
    ptr_cmd = & servo_cmd_wks [ 31 ] ;
    
    // Start reading servo numbers and speeds at the beginning of the
    // command area in EEPROM (offset in the player).  Read 3 bytes
    // for each servo (servo number = 1 byte, speed = 2 bytes).
	eex_start_read (
	    ptr_player -> seq_offset,
	    ptr_player -> num_servos * 3 ) ;
	for ( count = ptr_player -> num_servos ;
	    count > 0 ;
	    --count, --ptr_cmd )
	{
		ptr_cmd -> servo_num = eex_read_get_data ( ) ;
		ptr_cmd -> speed = (uint16_t)eex_read_get_data ( ) << 8 ;
		ptr_cmd -> speed |= eex_read_get_data ( ) ;
	}
}

//*********************************************************************
// Get the destination positions for the passed player into the
// command array.
//*********************************************************************
void gpseq_get_dest_positions ( seq_player_control_t * ptr_player )
{
    servo_command_def_t * ptr_cmd ;
    uint8_t count ;
    uint16_t temp_u16 ;

    // Get address of servo command structure.  GPSEQ commands grow
    // from the last element downwards.    
    ptr_cmd = & servo_cmd_wks [ 31 ] ;
    
	// Advance EEPROM offset to start of servo pulse widths
	// for this step.  Skip over the servo numbers and speed
	// by adding num_servos*3, then skip to the desired step
	// by adding 2 + 2*step_num + 2*num_servos*step_num.  With
	// some algebra, this equates to skipping
	//   num_servos * (3 + 2*step_num) + 2*step_num + 2
	// bytes.
	// Read 2 bytes per servo.
	temp_u16 = 2 * (uint16_t)ptr_player -> step_num ;
	temp_u16 += 2 +
	    ( ( 3 + temp_u16 ) * ptr_player -> num_servos ) ;
	eex_start_read (
	    ptr_player -> seq_offset + temp_u16,
	    2 * ptr_player -> num_servos ) ;
	// Get the pulse widths from EEPROM and store in the
	// command array
	for ( count = ptr_player -> num_servos ;
	    count > 0 ;
	    --count, --ptr_cmd )
	{
		temp_u16 = (uint16_t)eex_read_get_data ( ) << 8 ;
		temp_u16 |= eex_read_get_data ( ) ;
		if ( temp_u16 < 500 )
		{
			temp_u16 = 500 ;
		}
		else if ( temp_u16 > 2500 )
		{
			temp_u16 = 2500 ;
		}
		// Convert to 14.7456MHz clock cycles and store
		temp_u16 = ( temp_u16 * (uint32_t)( 65536L * 14.7456 ) + 32768 ) >> 16 ;
		ptr_cmd -> target_width = temp_u16 ;
	}
}

//*********************************************************************
// Calculate the move time for the player passed.  This takes into
// account the following:
// 1.  Configured time for the step being started/continued;
// 2.  Current servo positions, used to adjust the time if the servos
//     are not at the "from" position in the sequence;
// 3.  Speed multiplier.
// It does not take into account the maximum speed settings for the
// servos--that will be handled later.
//
// It is assumed that the servo numbers, speeds, and destination
// pulse width are already in the command array.  Algorithm:
// 0.  If the speed is 0, return 65535 (flag to pause sequence);
// 1.  Read the configured sequence_time for the step;
// 2.  Read the source position and current position for each servo;
// 3.  Accumulate total move distance based on the source and
//     destination positions in the sequence;
// 4.  Accumulate remaining move distance based on the current and
//     destination positions in the sequence;
// 5.  Calculate move_time as
//     move_time = 
//        sequence_time * 100 * remaining_move
//        / (abs(speed) * total_move)
// 6.  Limit the move_time to 30000
//*********************************************************************
uint16_t gpseq_calculate_move_time ( seq_player_control_t * ptr_player )
{
    servo_command_def_t * ptr_cmd ;
    uint8_t count ;
    uint8_t servo_invalid ;
    uint16_t temp_u16 ;
    uint32_t move_time ;
    uint32_t total_move = 0 ;
    uint32_t remaining_move = 0 ;
    
    // If speed is 0, return 65535, indicating that the player should
    // be paused.
    if ( ptr_player -> speed == 0 )
    {
        return 65535 ;
    }

    // Get address of servo command structure.  GPSEQ commands grow
    // from the last element downwards.    
    ptr_cmd = & servo_cmd_wks [ 31 ] ;
    
	// Advance EEPROM offset to start of servo pulse widths
	// and move time for this step.  Skip over the servo
	// numbers and speed by adding num_servos*3, then skip
	// to the desired step by adding
	// 2*step_num + 2*num_servos*step_num.  With
	// some algebra, this equates to skipping
	//   num_servos * (3 + 2*step_num) + 2*step_num
	// bytes.
	// Read 2 bytes per servo.
	temp_u16 = 2 * (uint16_t)gpseq_prev_step_num ( ptr_player ) ;
	temp_u16 += ( ( 3 + temp_u16 ) * ptr_player -> num_servos ) ;
	// Adjust starting offset for pos/neg speed.  (If speed is
	// negative, use the time before the servo positions; if
	// speed is positive, use the time after the servo
	// positions.)
	if ( ptr_player -> speed >= 0 )
	{
	    temp_u16 += 2 ;
	}
	eex_start_read (
	    ptr_player -> seq_offset + temp_u16,
	    2 * ptr_player -> num_servos + 2 ) ;
    // Get the move time for negative speed.
    if ( ptr_player -> speed < 0 )
    {
        move_time = (uint16_t)eex_read_get_data ( ) << 8 ;
        move_time |= eex_read_get_data ( ) ;
    }
    
	// Get the pulse widths from EEPROM and perform the calculations.
	for ( count = ptr_player -> num_servos ;
	    count > 0 ;
	    --count, --ptr_cmd )
	{
		// Get current pulse width
		temp_u16 = pulse_array [ ptr_cmd -> servo_num ] . current_width ;
		// Handle special case of pulse width 0 or 65535 by assuming
		// current pulse width equals target width.
		servo_invalid = FALSE ;
		if ( ( temp_u16 == 0 ) || ( temp_u16 == 65535 ) )
		{
		    servo_invalid = TRUE ;
		}
		// Determine absolute value of difference
		if ( temp_u16 > ptr_cmd -> target_width )
		{
		    temp_u16 -= ptr_cmd -> target_width ;
		}
		else
		{
		    temp_u16 = ptr_cmd -> target_width - temp_u16 ;
		}
		// Accumulate to remaining_move if current position valid.
		if ( servo_invalid == 0 )
		{
    		remaining_move += temp_u16 ;
        }
	    // Read previous position from EEPROM, clip to range
		temp_u16 = (uint16_t)eex_read_get_data ( ) << 8 ;
		temp_u16 |= eex_read_get_data ( ) ;
		if ( temp_u16 < 500 )
		{
			temp_u16 = 500 ;
		}
		else if ( temp_u16 > 2500 )
		{
			temp_u16 = 2500 ;
		}
		// Convert to 14.7456MHz clock cycles
		temp_u16 = ( temp_u16 * (uint32_t)( 65536L * 14.7456 ) + 32768 ) >> 16 ;
		// Determine absolute value of difference
		if ( temp_u16 > ptr_cmd -> target_width )
		{
		    temp_u16 -= ptr_cmd -> target_width ;
		}
		else
		{
		    temp_u16 = ptr_cmd -> target_width - temp_u16 ;
		}
		// Accumulate to total_move if current position valid
		if ( servo_invalid == 0 )
		{
    		total_move += temp_u16 ;
        }
	}
	// For positive speed, get time
	if ( ptr_player -> speed >= 0 )
	{
        move_time = (uint16_t)eex_read_get_data ( ) << 8 ;
        move_time |= eex_read_get_data ( ) ;
    }

    // Now that all of the data is gathered, calculate the
    // prorated time.
    // Special case:  starting a sequence and the servos are
    // already at their destination position.  Make sure
    // the move is immediate by setting total_move to 1.
    if ( ( ptr_player -> state == SEQ_PL_ST_STARTING )
        && ( remaining_move <= 32 ) )
    {
        total_move = 1 ;
    }
    // Special case:  if the total_time is 0, that
    // means that there was no change of position in this step.
    // In this case, set total_move and remaining_move to 1.
    if ( total_move == 0 )
    {
        total_move = remaining_move = 1 ;
    }
    // Get absolute value of speed
    if ( ptr_player -> speed < 0 )
    {
        temp_u16 = - ptr_player -> speed ;
    }
    else
    {
        temp_u16 = ptr_player -> speed ;
    }
    
    // Calculate denominator = abs(speed) * total_move
    total_move *= temp_u16 ;
    // Calculate numerator = 100 * remaining_move
    remaining_move *= 100 ;
    // Divide down numerator and denominator until
    // numerator < 65536.  The purpose for this is to
    // ensure that there will not be an overflow when
    // multiplying by move_time.
    while ( remaining_move > 65536 )
    {
        remaining_move >>= 1 ;
        total_move >>= 1 ;
    }
    // Check for divide-by-zero
    if ( total_move == 0 )
    {
        total_move = 1 ;
    }
    // Now finish calculation.
    move_time = move_time * remaining_move / total_move ;
    // Clip to 60000
    if ( move_time > 60000 )
    {
        move_time = 60000 ;
    }
    
    return move_time ;
}

//*********************************************************************
// Update the player indexed by global variable check_player_num.
//
// STARTING:
// Move servos to the starting index at the fastest speed that does
// not exceed the max speed (i.e. move time = 0).
//*********************************************************************
void update_player ( void )
{
//	uint32_t temp_u32 ;
//	uint16_t temp_u16 ;
	uint16_t gpseq_time ;
    seq_player_control_t * ptr_player ;

	// Copy selected player
	ptr_player =  & seq_player [ check_player_num ] ;

    // If STOPPED, leave immediately
	if ( ptr_player -> state == SEQ_PL_ST_STOPPED )
	{
	    return ;
	}
	
	// If RESTING and speed is 0, leave; if speed is
	// not 0, fall through to PLAYING state.
	if ( ptr_player -> state == SEQ_PL_ST_RESTING )
	{
	    if ( ptr_player -> speed == 0 )
	    {
    	    return ;
    	}
	}

    // Decrement ms_remaining, limiting to 0
    if ( ptr_player -> ms_remaining > 20 )
    {
        ptr_player -> ms_remaining -= 20 ;
    }
    else
    {
        ptr_player -> ms_remaining =0 ;
    }

	
	switch ( ptr_player -> state )
	{
		case SEQ_PL_ST_STOPPING :
		{
        	uint8_t count ;
			// For the STOPPING state, scan through the servos used
			// in this sequence, stopping each in turn.  Then set
			// the state to STOPPED.
   			eex_start_read ( ptr_player -> seq_offset, ptr_player -> num_servos*3 ) ;
			for ( count = ptr_player -> num_servos ;
					count > 0 ;
					-- count )
			{
				stop_servo ( eex_read_get_data ( ) ) ;
				eex_read_get_data ( ) ; // 2 more calls to skip over speed
				eex_read_get_data ( ) ;
			}
			ptr_player -> state = SEQ_PL_ST_STOPPED ;
	        return ;
	    }
	  
        case SEQ_PL_ST_PAUSING :
        {
            // For the PAUSING state, test decremented counter.  If not 0,
            // leave.  Else fall through to the PLAYING state.
            if ( ptr_player -> ms_remaining > 0 )
            {
                return ;
            }
            ptr_player -> flags |= SEQ_PL_FLAGS_MASK_CHANGE ;
        }
        
		case SEQ_PL_ST_STARTING :
		    // The STARTING state is identical to the PLAYING state,
		    // except that the move to the initial position is
		    // immediate.  This is handled by special case code in
		    // the PLAYING state, so for now just fall through to
		    // the PLAYING state.
		case SEQ_PL_ST_RESTING :
		    // The RESTING state is identical to PLAYING, except
		    // that it skips PAUSING.
		case SEQ_PL_ST_PLAYING :
		{
		    // For the PLAYING state, test the changed flag and
		    // test ms_remaining.  If not changed and not
		    // timed out, then leave.
		    if ( ( ( ptr_player -> flags & SEQ_PL_FLAGS_MASK_CHANGE ) == 0 )
		        && ( ptr_player -> ms_remaining > 0 ) )
		    {
                return ;
            }
            // Clear changed flag
            ptr_player -> flags &= ~ SEQ_PL_FLAGS_MASK_CHANGE ;
            // If timed out, current state is PLAYING, and there is
            // a pause delay, start pause delay and leave.
            if ( ( ptr_player -> ms_remaining == 0 )
                && ( ptr_player -> state == SEQ_PL_ST_PLAYING )
                && ( ptr_player -> pause_ms > 0 ) )
            {
                ptr_player -> ms_remaining = ptr_player -> pause_ms ;
                ptr_player -> state = SEQ_PL_ST_PAUSING ;
                return ;
            }
            // If the speed is 0, stop the servos and transition to
            // RESTING state.
            if ( ptr_player -> speed == 0 )
            {
            	uint8_t count ;
    			// Scan through the servos used
    			// in this sequence, stopping each in turn.  Then set
    			// the state to STOPPED.
    			eex_start_read ( ptr_player -> seq_offset, ptr_player -> num_servos*3 ) ;
    			for ( count = ptr_player -> num_servos ;
    					count > 0 ;
    					-- count )
    			{
    				stop_servo ( eex_read_get_data ( ) ) ;
    				eex_read_get_data ( ) ; // 2 more calls to skip over speed
    				eex_read_get_data ( ) ;
    			}
    			ptr_player -> state = SEQ_PL_ST_RESTING ;
    	        return ;
   	        }
                
            // At this point, either the current step is complete, or
            // a command has been received to change a parameter.  In
            // either case, recalculate.
            // If PLAYING, PAUSING, or RESTING and timeout just
            // occurred, increment the step number;
            if ( ( ( ptr_player -> state == SEQ_PL_ST_PLAYING )
                || ( ptr_player -> state == SEQ_PL_ST_PAUSING )
                || ( ptr_player -> state == SEQ_PL_ST_RESTING ) )
                &&  ( ptr_player -> ms_remaining == 0 ) )
            {
                // Test for ONCE option.  If the ONCE flag is set, this it
                // not the first step completed, and the step number just reached
                // is the final step number, set state to STOPPED and
                // leave.
                if ( ( ( ptr_player -> flags 
                    & ( SEQ_PL_FLAGS_MASK_ONCE | SEQ_PL_FLAGS_MASK_STEP_COMPLETED ) )
                    == ( SEQ_PL_FLAGS_MASK_ONCE | SEQ_PL_FLAGS_MASK_STEP_COMPLETED ) )
                    && ( ptr_player -> step_num == ptr_player -> stop_step_num ) )
                {
                    ptr_player -> state = SEQ_PL_ST_STOPPED ;
                    return ;
                }
                ptr_player -> flags |= SEQ_PL_FLAGS_MASK_STEP_COMPLETED ;
                // Now increment/decrement the step number
                if ( ptr_player -> speed >= 0 )
                {
                    ++ ptr_player -> step_num ;
                    if ( ptr_player -> step_num == ptr_player -> num_steps )
                    {
                        ptr_player -> step_num = 0 ;
                    }
                }
                else
                {
                    if ( ptr_player -> step_num == 0 )
                    {
                        ptr_player -> step_num = ptr_player -> num_steps ;
                    }
                    -- ptr_player -> step_num ;
                }
            }
            // Read the servo numbers, speeds and destination positions into
            // the command array.
            gpseq_get_servo_num_speed ( ptr_player ) ;
            gpseq_get_dest_positions ( ptr_player ) ;
            // Compute the move time based on max speeds, destination
            // positions (both previously read), source positions,
            // current positions, and configured move time.  Special
            // case, if the state is STARTING, set the time to 0 for
            // an immediate move to the initial position.
//            if ( ptr_player -> state == SEQ_PL_ST_STARTING )
//            {
//                gpseq_time = 0 ;
//            }
//            else
//            {
                gpseq_time = gpseq_calculate_move_time ( ptr_player ) ;
//            }
            // Convert the move information in the command array into
            // servo pulse widths and deltas in the pulse array.
    		convert_servo_cmd_to_pulse_arr (
    		    ptr_player -> num_servos,
    		    32 - ptr_player -> num_servos,
    		    gpseq_time ) ;
    		// Set the ms_remaining from the global ms in the command
    		// just processed.
    		ptr_player -> ms_remaining = ms_in_command ;
    		
    		// At this point the state could be STARTING, PLAYING,
    		// or PAUSING.  Set it to PLAYING for the next loop.
    		ptr_player -> state = SEQ_PL_ST_PLAYING ;
		}
    }
}

//*********************************************************************
// Do function for gpseq, called when the terminating carriage return
// is received.
// If the sequence number is different from the currently playing
// sequence, then copy the command structure to the player. 
// Otherwise, this is just a speed change, so just set the new speed.
//*********************************************************************
void do_gpseq ( void )
{
	seq_player_control_t * ptr_player ;

	ptr_player = & seq_player [ cmd_player_num ] ;
        
    // Shortcut, handle command to stop.  If the state is STOPPED or
    // STARTING, set state to STOPPED and leave.  Otherwise set state
    // to STOPPING and leave.
    if ( cmd_wks . pl_cmd . flags == 0 )
    {
        if ( ptr_player -> state == SEQ_PL_ST_STOPPED
            || ptr_player -> state == SEQ_PL_ST_STARTING )
        {
            ptr_player -> state = SEQ_PL_ST_STOPPED ;
        }
        else
        {
            ptr_player -> state = SEQ_PL_ST_STOPPING ;
        }
        return ;
    }
    
    // If the command included a speed value as the only parameter but
    // it is the same as the current value, ignore the command.
    if ( cmd_wks . pl_cmd . flags == SEQ_PL_FLAGS_MASK_SPEED_CHANGE 
        && cmd_wks . pl_cmd . speed == ptr_player -> speed )
    {
        return ;
    }

    // If the command included a pause value as the only parameter, set
    // the value and leave.
    if ( cmd_wks . pl_cmd . flags == SEQ_PL_FLAGS_MASK_PAUSE_CHANGE )
    {
        ptr_player -> pause_ms = cmd_wks . pl_cmd . pause_ms ;
        return ;
    }
    
    // Handle case of STOPPED or STOPPING and request to start.  If
    // no valid sequence number, leave.  Otherwise set defaults where
    // needed.
    if ( ptr_player -> state == SEQ_PL_ST_STOPPED
        || ptr_player -> state == SEQ_PL_ST_STOPPING )
    {
        if ( cmd_wks . pl_cmd . flags & SEQ_PL_FLAGS_MASK_SEQ_CHANGE )
        {
            // Set state to STARTING
            ptr_player -> state = SEQ_PL_ST_STARTING ;
            // Set defaults as needed
            // Default index is 0
            ptr_player -> step_num = 0 ;
            // Default speed is 100
            ptr_player -> speed = 100 ;
            // Default pause is 0
            ptr_player -> pause_ms = 0 ;
        }
        else
        {
            // Starting without a valid sequence number, leave
            return;
        }
    }
    
    // At this point the state is STARTING, PLAYING, or PAUSING.  Copy
    // whichever parameters were in the command and set changed flag.
    if ( cmd_wks . pl_cmd . flags & SEQ_PL_FLAGS_MASK_SEQ_CHANGE )
    {
        ptr_player -> seq_num = cmd_wks . pl_cmd . seq_num ;
        ptr_player -> num_servos = cmd_wks . pl_cmd . num_servos ;
        ptr_player -> num_steps = cmd_wks . pl_cmd . num_steps ;
        ptr_player -> seq_offset = cmd_wks . pl_cmd . seq_offset ;
    }
    if ( cmd_wks . pl_cmd . flags & SEQ_PL_FLAGS_MASK_STEPNUM_CHANGE )
    {
        ptr_player -> step_num = cmd_wks . pl_cmd . step_num ;
    }
    if ( cmd_wks . pl_cmd . flags & SEQ_PL_FLAGS_MASK_SPEED_CHANGE )
    {
        // If the state is not STARTING and the sign of the speed
        // changed, then change the step number to the previous
        // step number.
        if ( ( ptr_player -> state != SEQ_PL_ST_STARTING )
            && ( ptr_player -> state != SEQ_PL_ST_PAUSING )
            && ( ptr_player -> ms_remaining > 20 )
            && ( ( ptr_player -> speed >= 0 ) != ( cmd_wks . pl_cmd . speed >= 0 ) ) )
        {
            ptr_player -> step_num = gpseq_prev_step_num ( ptr_player ) ;
        }
        ptr_player -> speed = cmd_wks . pl_cmd . speed ;
    }
    if ( cmd_wks . pl_cmd . flags & SEQ_PL_FLAGS_MASK_PAUSE_CHANGE )
    {
        ptr_player -> pause_ms = cmd_wks . pl_cmd . pause_ms ;
    }

    ptr_player -> flags = SEQ_PL_FLAGS_MASK_CHANGE ;

    // If ONCE was specified, set the flag and store the stopping index
    if ( cmd_wks . pl_cmd . flags & SEQ_PL_FLAGS_MASK_ONCE_CHANGE )
    {
        ptr_player -> flags |= SEQ_PL_FLAGS_MASK_ONCE ;
        ptr_player -> stop_step_num = ptr_player -> step_num ;
    }
}

//*********************************************************************
// Do function for GOTO, called when the terminating carriage return
// is received.
// This function is invoked by a command line
//  "SQ s IX i T t"
// where T is optional.  The sequence should not be playing when a
// GOTO command is started.
//
// This function starts the servos moving towards the sequence "s"
// step "i" with a move time of "t", honoring the maximum speeds of
// the servos from the sequence.  If a time is not specified, the 
// move will be as fast as possible.
//*********************************************************************
void do_goto ( void )
{
    servo_command_def_t * ptr_cmd ;
    uint16_t temp_u16 ;
    uint8_t count ;

    // Get the servo numbers and speeds from the header into the
    // command array.
    {
        // Get address of servo command structure.  GPSEQ commands grow
        // from the last element downwards.    
        ptr_cmd = & servo_cmd_wks [ 0 ] ;
        
        // Start reading servo numbers and speeds at the beginning of the
        // command area in EEPROM (offset in the player).  Read 3 bytes
        // for each servo (servo number = 1 byte, speed = 2 bytes).
    	eex_start_read (
    	    cmd_wks . pl_cmd . seq_offset,
    	    cmd_wks . pl_cmd . num_servos * 3 ) ;
    	for ( count = cmd_wks . pl_cmd . num_servos ;
    	    count > 0 ;
    	    --count, ++ptr_cmd )
    	{
    		ptr_cmd -> servo_num = eex_read_get_data ( ) ;
    		ptr_cmd -> speed = (uint16_t)eex_read_get_data ( ) << 8 ;
    		ptr_cmd -> speed |= eex_read_get_data ( ) ;
    	}
	}
    
    // Get the destination positions from the header into the
    // command array.
    {
        // Get address of servo command structure.  GPSEQ commands grow
        // from the last element downwards.    
        ptr_cmd = & servo_cmd_wks [ 0 ] ;
        
    	// Advance EEPROM offset to start of servo pulse widths
    	// for this step.  Skip over the servo numbers and speed
    	// by adding num_servos*3, then skip to the desired step
    	// by adding 2 + 2*step_num + 2*num_servos*step_num.  With
    	// some algebra, this equates to skipping
    	//   num_servos * (3 + 2*step_num) + 2*step_num + 2
    	// bytes.
    	// Read 2 bytes per servo.
    	temp_u16 = 2 * (uint16_t)cmd_wks . pl_cmd . step_num ;
    	temp_u16 += 2 +
    	    ( ( 3 + temp_u16 ) * cmd_wks . pl_cmd . num_servos ) ;
    	eex_start_read (
    	    cmd_wks . pl_cmd . seq_offset + temp_u16,
    	    2 * cmd_wks . pl_cmd . num_servos ) ;
    	// Get the pulse widths from EEPROM and store in the
    	// command array
    	for ( count = cmd_wks . pl_cmd . num_servos ;
    	    count > 0 ;
    	    --count, ++ptr_cmd )
    	{
    		temp_u16 = (uint16_t)eex_read_get_data ( ) << 8 ;
    		temp_u16 |= eex_read_get_data ( ) ;
    		if ( temp_u16 < 500 )
    		{
    			temp_u16 = 500 ;
    		}
    		else if ( temp_u16 > 2500 )
    		{
    			temp_u16 = 2500 ;
    		}
    		// Convert to 14.7456MHz clock cycles and store
    		temp_u16 = ( temp_u16 * (uint32_t)( 65536L * 14.7456 ) + 32768 ) >> 16 ;
    		ptr_cmd -> target_width = temp_u16 ;
    	}
    }
    
    // Convert the move information in the command array into
    // servo pulse widths and deltas in the pulse array.
    convert_servo_cmd_to_pulse_arr (
        cmd_wks . pl_cmd . num_servos,
        0,
        rx_time ) ;
}

//*********************************************************************
// Store function for the PL command, called when data for the command
// is received.
// If the player number is valid, store the player number and set
// command type.
//*********************************************************************
void store_pl ( void )
{
	// If player number is valid, save value and set cmd_type.
	if ( ( cmd_player_num = cmd_arg_int ) < NUM_SEQ_PLAYERS )
	{
		cmd_type = CMDTYPE_PLAY_SEQ ;
	}
	
	// Default (sentinel) values
	cmd_wks . pl_cmd . flags = 0 ;
}

//*********************************************************************
// Store function for the SQ command, called when data for the command
// is received.
// If the command type and sequence number are valid, store the
// sequence number, default speed, and default starting index.  Then
// read the starting sequence address from EEPROM at address 2*seq_num.
// Read sequence number, flags, and number of steps from EEPROM.  If
// the sequence number in EEPROM does not match the argument, then
// the sequence is invalid, so set the sequence number to 255, which
// turns off the player.
// Memory map of the sequence header:
// 0: seq_num (0-127)
// 1: number of servos (1-32)
// 2: number of steps in sequence (0-255)
//*********************************************************************
void store_sq ( void )
{
	uint8_t seq_num ;
	
	seq_num = cmd_arg_int ;

	// Exit if the sequence number is invalid.	
	if ( seq_num > 127 )
	{
		return ;
	}
	// If this is not part of a PL command, then it must be a GOTO
	if ( cmd_type != CMDTYPE_PLAY_SEQ )
	{
		cmd_type = CMDTYPE_GOTO ;
		cmd_wks . pl_cmd . flags = 0 ;
		cmd_wks . pl_cmd . step_num = 0 ;
		rx_time = 0 ;
	}
	
	// Set the sequence number
	cmd_wks . pl_cmd . seq_num = seq_num ;
	cmd_wks . pl_cmd . flags |= SEQ_PL_FLAGS_MASK_SEQ_CHANGE ;
	
	// Get pointer to sequence header from EEPROM
	eex_start_read ( (uint16_t)seq_num * 2, 2 ) ;
	cmd_wks . pl_cmd . seq_offset =
		(uint16_t)eex_read_get_data ( ) << 8 ;	// High byte
	cmd_wks . pl_cmd . seq_offset |= eex_read_get_data ( ) ;	// Low byte

	// Follow pointer to get sequence number from EEPROM, compare
	//    with this sequence number, kill command if no match
	eex_start_read ( cmd_wks . pl_cmd . seq_offset, 3 ) ;
	seq_num = eex_read_get_data ( ) ;
	if ( seq_num != cmd_wks . pl_cmd . seq_num 
		|| cmd_wks . pl_cmd . seq_offset < 16 )
	{
		cmd_type = CMDTYPE_NONE ;	// Stop processing of this command
	}

	// Follow pointer to load #servos
	cmd_wks . pl_cmd . num_servos = eex_read_get_data ( ) ;

	// Follow pointer to load number of steps from EEPROM
	cmd_wks . pl_cmd . num_steps = eex_read_get_data ( ) ;
		
	// Advance the sequence offset past the header to point to the
	// start of the servos.
	cmd_wks . pl_cmd . seq_offset += 3 ;
}

//*********************************************************************
// Store function for the SM command, called when data for the command
// is received.
// If the command type and speed multiplier are valid, store the
// speed multiplier with the correct sign.  If the speed multiplier
// is invalid (>100) then limit to 100.
//*********************************************************************
void store_sm ( void )
{
	if ( cmd_type != CMDTYPE_PLAY_SEQ )
	{
		return ;
	}

    // Set flag indicating speed value was specified
	cmd_wks . pl_cmd . flags |= SEQ_PL_FLAGS_MASK_SPEED_CHANGE ;

	// Store the unsigned value, limited to 200
	if ( cmd_arg_int > 100 )
	{
		cmd_wks . pl_cmd . speed = 200 ;
	}
	else
	{
		cmd_wks . pl_cmd . speed = cmd_arg_int ;
	}
	
	// Check the sign flag and invert if necessary
	if ( cmd_arg_positive == FALSE )
	{
		cmd_wks . pl_cmd . speed = - cmd_wks . pl_cmd . speed ;
	}
}

//*********************************************************************
// Store function for the IX command, called when data for the command
// is received.
// If the command type and index are valid, store the index.  If the
// index is invalid (i.e. greater than the number of steps), then
// leave the value unchanged from the default of 0.
//*********************************************************************
void store_ix ( void )
{
	if ( ( cmd_type != CMDTYPE_PLAY_SEQ ) && ( cmd_type != CMDTYPE_GOTO ) )
	{
		return ;
	}

	// Store the unsigned value if valid
	if ( cmd_arg_int < cmd_wks . pl_cmd . num_steps )
	{
		cmd_wks . pl_cmd . step_num = cmd_arg_int ;
        // Set flag indicating speed value was specified
	    cmd_wks . pl_cmd . flags |= SEQ_PL_FLAGS_MASK_STEPNUM_CHANGE ;
	}
}

//*********************************************************************
// Store function for the PA command, called when data for the command
// is received.
// If the command type is valid, store the pause amount.
//*********************************************************************
void store_pa ( void )
{
	if ( cmd_type != CMDTYPE_PLAY_SEQ )
	{
		return ;
	}

	// Store the unsigned value
	cmd_wks . pl_cmd . pause_ms = cmd_arg_int ;
	
    // Set flag indicating speed value was specified
    cmd_wks . pl_cmd . flags |= SEQ_PL_FLAGS_MASK_PAUSE_CHANGE ;
}

//*********************************************************************
// Store function for the QPL command, called when data for the command
// is received.  The data is the player number.
// If the player number is valid, the QPL command transmits 4 bytes:
// Byte 0 = the current sequence, or 255 if no sequence is playing
// Byte 1 = the index moving from in the sequence (0 if no sequence)
// Byte 2 = the index moving to in the sequence (0 if no sequence)
// Byte 3 = time remaining in move, 100ms/bit (0 if no sequence)
//*********************************************************************
void store_qpl ( void )
{
	uint8_t player_num ;
	uint8_t ms_div_100 ;
	seq_player_control_t * ptr_player ;
	
	player_num = cmd_arg_int ;
	if ( player_num > 3 )
	{
		return ;
	}
	
    // Get copy of the player
	ptr_player = & seq_player [ player_num ] ;

    //	Generate response
	if ( ptr_player -> state == SEQ_PL_ST_STOPPED )
	{
		qresult_char ( 255 ) ;
		qresult_char ( 0 ) ;
		qresult_char ( 0 ) ;
		qresult_char ( 0 ) ;
	}
	else
	{
		qresult_char ( ptr_player -> seq_num ) ;
    	qresult_char ( ptr_player -> step_num ) ;
    	qresult_char ( gpseq_prev_step_num ( ptr_player ) ) ;
    	ms_div_100 = 255 ;
    	if ( ptr_player -> ms_remaining < ( 100 * 255 ) )
    	{
	    	ms_div_100 = ptr_player -> ms_remaining / 100 ;
    	}
    	if ( ptr_player -> state == SEQ_PL_ST_PAUSING )
    	{
    		// Return timer remaining of 0 if paused
	    	ms_div_100 = 0 ;
    	}
    	qresult_char ( ms_div_100 ) ;
	}
}

//*********************************************************************
// Parse functions for gpseq, called when a command is recognized.
//*********************************************************************
void parse_pl ( void )
{
	ptr_store_arg_func = store_pl ;
	goto_parse_int16_sign ( ) ;
}

void parse_sq ( void )
{
	ptr_store_arg_func = store_sq ;
	goto_parse_int16_sign ( ) ;
}

void parse_sm ( void )
{
	ptr_store_arg_func = store_sm ;
	goto_parse_int16_sign ( ) ;
}

void parse_pa ( void )
{
	ptr_store_arg_func = store_pa ;
	goto_parse_int16_sign ( ) ;
}

void parse_ix ( void )
{
	ptr_store_arg_func = store_ix ;
	goto_parse_int16_sign ( ) ;
}

void parse_qpl ( void )
{
	ptr_store_arg_func = store_qpl ;
	goto_parse_int16_sign ( ) ;
}

void parse_once ( void )
{
	if ( cmd_type != CMDTYPE_PLAY_SEQ )
	{
		return ;
	}

    // Set flag indicating ONCE was specified
    cmd_wks . pl_cmd . flags |= SEQ_PL_FLAGS_MASK_ONCE_CHANGE ;

	goto_wait_cmd ( ) ;
}

#endif //GENERAL_SEQUENCER_ENABLE
