//*********************************************************************
// Routines to manage the hexapod sequencer.
// Copyright 2005, Mike Dvorsky.
//*********************************************************************

#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "compiler.h"

#include "numtypes.h"
#include "appdefs.h"
#include "globals.h"
#include "hexapod.h"

// If hexapod sequencer not enabled, generate no code
#if HEXAPOD_SEQUENCER_ENABLE


//*********************************************************************
// Global variables.
//*********************************************************************

// NOTE:  No global variables (or static) allowed in this file.  See 
// the note in globals.c for an explanation.


//*********************************************************************
// Return the absolute value of the difference of the two
// arguments.
//*********************************************************************
uint16_t abs_diff_u16 ( uint16_t a, uint16_t b )
{
	if ( a > b )
	{
		return a - b ;
	}
	else
	{
		return b - a ;
	}
}

//*********************************************************************
// Compute the move speed in cycles per 20ms tick.
// Inputs are the
// total move distance in us, and the displacement
// percentage.  The global xspeed and htime are also used.  The
// calculation is:
//   s = 2^8 * |delta| * 14.7456 * 0.02 * disp/100 * xspeed/100 * 1000/htime
//     = |delta| * disp * xspeed / htime * (14.7456 * 0.02 * 1000 / 100 / 100)
//     = |delta| * disp * xspeed / htime * 0.0295
//     = |delta| * disp * xspeed / htime * 0.0295 * 65536 / 65536
//     = |delta| * disp * xspeed / htime * 1933 / 655536
//*********************************************************************
uint16_t calc_speed ( uint16_t delta , uint16_t disp )
{
	uint32_t u32_temp ;
	
	u32_temp = (uint32_t)delta * disp * xconfig . xspeed ;
	
	if ( u32_temp >= 0xFFFFFFFF/1933 )
	{
		u32_temp = u32_temp / xconfig . htime * 1933L ;
	}
	else
	{
		u32_temp = u32_temp * 1933L / xconfig . htime ;
	}

	// Round and unscale
	return ( u32_temp + 0x00008000 ) >> 16 ;
}

//*********************************************************************
// Execute the hexapod sequencer.  The basic sequence consists of 8
// states, with reference to tripod A:
// 0 - vlow, moving from hfront to hcenter
// 1 - vlow, moving from hcenter to hrear
// 2 - vlow, hrear
// 3 - moving from vlow to vmid, hrear
// 4 - moving from vmid to vhigh, moving from hrear to hcenter
// 5 - moving from vhigh to vmid, moving from hcenter to hfront
// 6 - moving from vmid to vlow, hfront
// 7 - vlow, hfront
//*********************************************************************
void hex_sequencer ( void )
{
	uint16_t u16_temp ;
	uint16_t u16_temp2 ;
	uint16_t u16_temp3 ;
	uint32_t u32_temp ;
	
	// If not started and speed != 0, set speed to 0 and fall through
	// to stop servos.  Subsequent times, the speed will be 0; in this
	// case return immediately.
	if ( ! xconfig . started )
	{
		if ( xconfig . xspeed == 0 )
		{
			return ;
		}
		xconfig.xspeed = 0 ;
	}

	//*************************
	// Compute and store the L/R horizontal speed in units of us per
	// 20ms tick.  The speed is determined from the front/rear configured
	// pulse widths (pwf/pwr), the displacement percentage (d), horiz
	// speed multipler (xs), and the horiz move time (ht).
	if ( xconfig . htime == 0 )
	{
		// Ensure htime is non-zero
		xconfig . htime = 1 ;
	}
	// Left speed
	hex_left_pulse_def . hspeed =
		calc_speed ( abs_diff_u16 ( xconfig . lfront, xconfig . lrear ), xconfig . ldisp ) ;
	// Right speed
	hex_right_pulse_def . hspeed =
		calc_speed ( abs_diff_u16 ( xconfig . rfront, xconfig . rrear ), xconfig . rdisp ) ;

	//*************************
	// Compute and store the vertical speed based on the configured
	// speed (vs).  The vertical speed in cycles per 20ms loop is
	// calculated as:
	//   s = vs * 14.7456 * 0.02 * 65536 / 65536
	hex_left_pulse_def . vspeed_low_mid =
	hex_right_pulse_def . vspeed_low_mid =
		( (uint32_t)( 14.7456 * 0.02 * 65536 ) * xconfig . vspeed ) >> 16 ;
	// Calculate the speed for vertical moves combined with horizontal.  Use a
	// displacement of 200 because the total time is half the configured horizontal
	// time so it must travel at twice the speed.
	hex_left_pulse_def . vspeed_high_mid =
		calc_speed ( abs_diff_u16 ( xconfig . lhigh, xconfig . lmid ), 200 ) ;
	hex_right_pulse_def . vspeed_high_mid =
		calc_speed ( abs_diff_u16 ( xconfig . rhigh, xconfig . rmid ), 200 ) ;
	
	//*************************
	// Determine move time in ms.  In states 0, 1, 4, and 5, the move time is
	// determined from horizontal move time and speed; in states 2, 3, 6, and 7,
	// the move time is computed from the vertical move speed and ( vmid - vmin ).
	// 
	// Leave the move time in u16_temp.
	if ( hex_state & 0x02 )	// States 2,3,6,7
	{
		//   t = abs(vmid - vmin) * 1000 / vs
		u16_temp = abs_diff_u16 ( xconfig . lmid, xconfig . llow ) ;
		u16_temp2 = abs_diff_u16 ( xconfig . rmid, xconfig . rlow ) ;
		// Vertical move time is based on the larger of the L/R movements
		if ( u16_temp2 > u16_temp )
		{
			u16_temp = u16_temp2 ;
		}
		u32_temp = 1000L * u16_temp / xconfig . vspeed ;
		if ( u32_temp > 65535 )
		{
			u16_temp = 65535 ;
		}
		else
		{
			u16_temp = u32_temp ;
		}
	}
	else // States 0,1,4,5
	{
		// Horizontal move time is
		//   t = htime / ( 2 * xspeed / 100 )
		//     = ( 256 * 100 / 2 ) * htime / xspeed / 256
		//     = ( 12800 * htime / xspeed ) >> 8
		// If xspeed is 0, ignore divide-by-zero since the value will be discarded
		u16_temp = ( 12800L * xconfig . htime / xconfig . xspeed ) >> 8 ;
	}

	//*************************
	// Update move timer.  First compute move time delta, then subtract
	// from move time.  The delta (per 20ms loop) is determined from the
	// initial move timer value (65535) and the computed move time:
	//   delta = 20 * 65535 / move_time
	// Initially, hex_move_time is in u16_temp
	// Intermediate move_time_delta is stored in u16_t
	//
	// If hex speed is 0, then stop timer by setting delta=0
	if ( xconfig . xspeed == 0 )
	{
		u16_temp = 0 ;
	}
	// Test hex_move_time, if <20ms then delta=max
	else if ( u16_temp < 20 )
	{
		u16_temp = 65535 ;
	}
	else
	{
		u16_temp = ( 20L * 65535L ) / u16_temp ;
	}
	// Compare timer with delta; if subtraction will cause timer to be <0,
	// then timer has expired.
	if ( hex_move_timer > u16_temp )
	{
		hex_move_timer -= u16_temp ;
	}
	else
	{
		// If move timer expired, advance state and reset timer
		hex_state = ( hex_state + 1 ) & 0x07 ;
		hex_move_timer = 65535 ;
	}
	
	//*************************
	// Compute L/R front/rear pulse widths in us.  Pulse width is computed from
	// front/rear configured pulse width and displacement.
	//   center = ( front + rear ) / 2
	//   temp = center * ( 100 - disp ) / 100
	//        = ( 65536 / 100 ) * center * ( 100 - disp ) >> 16
	//   f' = front * disp / 100 + center * ( 100 - disp ) / 100
	//      = ( 65536 / 100 ) * front * disp >> 16 + temp
	//      = 655 * front * disp >> 16 + temp
	//   r' = rear * disp / 100 + center * ( 100 - disp ) / 100
	//      = ( 65536 / 100 ) * rear * disp >> 16 + temp
	//      = 655 * rear * disp >> 16 + temp
	// Left side
	u16_temp = xconfig . lrear ;
	u16_temp2 = xconfig . lfront ;
	u16_temp3 = ( u16_temp + u16_temp2 ) / 2 ;
	hex_left_pulse_def . pw_center = u16_temp3 ;
	u16_temp3 = ( 655L * u16_temp3 * ( 100 - xconfig . ldisp ) ) >> 16 ;
	u32_temp = 655L * xconfig . ldisp ;
	u16_temp2 = (uint16_t) ( ( u32_temp * u16_temp2 ) >> 16 ) + u16_temp3 ;
	u16_temp = (uint16_t) ( ( u32_temp * u16_temp ) >> 16 ) + u16_temp3 ;
	if ( !xconfig . ldisp_neg )
	{
		// Swap u16_temp and u16_temp2
		u16_temp ^= u16_temp2 ;
		u16_temp2 ^= u16_temp ;
		u16_temp ^= u16_temp2 ;
	}
	hex_left_pulse_def . pw_rear = u16_temp ;
	hex_left_pulse_def . pw_front = u16_temp2 ;
	// Right side
	u16_temp = xconfig . rrear ;
	u16_temp2 = xconfig . rfront ;
	u16_temp3 = ( u16_temp + u16_temp2 ) / 2 ;
	hex_right_pulse_def . pw_center = u16_temp3 ;
	u16_temp3 = ( 655L * u16_temp3 * ( 100 - xconfig . rdisp ) ) >> 16 ;
	u32_temp = 655L * xconfig . rdisp ;
	u16_temp2 = (uint16_t) ( ( u32_temp * u16_temp2 ) >> 16 ) + u16_temp3 ;
	u16_temp = (uint16_t) ( ( u32_temp * u16_temp ) >> 16 ) + u16_temp3 ;
	if ( !xconfig . rdisp_neg )
	{
		// Swap u16_temp and u16_temp2
		u16_temp ^= u16_temp2 ;
		u16_temp2 ^= u16_temp ;
		u16_temp ^= u16_temp2 ;
	}
	hex_right_pulse_def . pw_rear = u16_temp ;
	hex_right_pulse_def . pw_front = u16_temp2 ;
	
	// Store L/R vertical pulse widths
	hex_left_pulse_def . pw_low = xconfig . llow ;
	hex_left_pulse_def . pw_mid = xconfig . lmid ;
	hex_left_pulse_def . pw_high = xconfig . lhigh ;
	hex_right_pulse_def . pw_low = xconfig . rlow ;
	hex_right_pulse_def . pw_mid = xconfig . rmid ;
	hex_right_pulse_def . pw_high = xconfig . rhigh ;

	// Update front left leg
	hex_update_leg (
		hex_state,					// Tripod A gets direct state
		& hex_left_pulse_def,		// Pulse widths for left side
		& pulse_array [ HEX_LF_VERT_CHNUM ] ) ; // Left front vertical channel pointer
	
	// Update center left leg
	hex_update_leg (
		( hex_state + 4 ) & 0x07,	// Tripod B gets offset state
		& hex_left_pulse_def,		// Pulse widths for left side
		& pulse_array [ HEX_LC_VERT_CHNUM ] ) ; // Left center vertical channel pointer
	
	// Update rear left leg
	hex_update_leg (
		hex_state,					// Tripod A gets direct state
		& hex_left_pulse_def,		// Pulse widths for left side
		& pulse_array [ HEX_LR_VERT_CHNUM ] ) ; // Left rear vertical channel pointer
	
	// Update front right leg
	hex_update_leg (
		( hex_state + 4 ) & 0x07,	// Tripod B gets offset state
		& hex_right_pulse_def,		// Pulse widths for right side
		& pulse_array [ HEX_RF_VERT_CHNUM ] ) ; // Right front vertical channel pointer
	
	// Update center right leg
	hex_update_leg (
		hex_state,					// Tripod A gets direct state
		& hex_right_pulse_def,		// Pulse widths for right side
		& pulse_array [ HEX_RC_VERT_CHNUM ] ) ; // Right center vertical channel pointer
	
	// Update rear right leg
	hex_update_leg (
		( hex_state + 4 ) & 0x07,	// Tripod B gets offset state
		& hex_right_pulse_def,		// Pulse widths for right side
		& pulse_array [ HEX_RR_VERT_CHNUM ] ) ; // Right rear vertical channel pointer
}

//*********************************************************************
// Update the servo channel information for a leg of the hexapod.
// IMPORTANT:  Assumes that the horizontal pulse structure immeditately
// follows the vertical pulse structure in pulse_array[].
//*********************************************************************
void hex_update_leg (
	uint8_t state ,
	hex_pulse_def_set_t * pulse_def_set ,
	pulse_def_t * pulse_ptr )
{
	uint16_t vert_pw ;
	uint16_t horiz_pw ;
	
	// Default values for vertcial speed.  pulse_ptr is passed a pointer
	// to the vertical pulse structure.
	pulse_ptr -> max_delta = pulse_def_set -> vspeed_low_mid ;
	pulse_ptr -> max_delta_frac = 0 ;
	
	// Default values for pulse width
	vert_pw = pulse_def_set -> pw_low ; 
	horiz_pw = pulse_def_set -> pw_front ;

	switch ( state )
	{
		case 0 :		// vlow, moving from hfront to hcenter
// in case 0, set the target to rear; this makes case 0 identical to cases 1 & 2
//			horiz_pw = pulse_def_set -> pw_center ;
//			break ;
		case 1 :		// vlow, moving from hcenter to hrear
// case 1 is identical to case 2
//			horiz_pw = pulse_def_set -> pw_rear ;
//			break ;
		case 2 :		// vlow, hrear
			horiz_pw = pulse_def_set -> pw_rear ;
			break ;
		case 3 :		// moving from vlow to vmid, hrear
			vert_pw = pulse_def_set -> pw_mid ; 
			horiz_pw = pulse_def_set -> pw_rear ;
			break ;
		case 4 :		// moving from vmid to vhigh, moving from hrear to hcenter
			vert_pw = pulse_def_set -> pw_high ; 
// in case 4, set the target to front; this makes it the default target
//			horiz_pw = pulse_def_set -> pw_center ;
			// pulse_ptr is a pointer to the vertical pulse structure
			pulse_ptr -> max_delta = pulse_def_set -> vspeed_high_mid ;
			break ;
		case 5 :		// moving from vhigh to vmid, moving from hcenter to hfront
			vert_pw = pulse_def_set -> pw_mid ; 
			// pulse_ptr is a pointer to the vertical pulse structure
			pulse_ptr -> max_delta = pulse_def_set -> vspeed_high_mid ;
			break ;
// case 6 and case 7 are the default values
//		case 6 :		// moving from vmid to vlow, hfront
//			break ;
//		case 7 :		// vlow, hfront
//			break ;    
	}

	// Store vertical pulse width.  Convert from us to cycles.
	// pulse_ptr points to the vertical pulse structure.
	pulse_ptr -> target_width = ( (uint32_t)( 14.7456 * 65536 ) * vert_pw ) >> 16 ;
	
	// Advance pulse_ptr to the horizontal pulse structre, and store the
	// horizontal pulse width.  Convert from us to cycles.
	++ pulse_ptr ;
	pulse_ptr -> target_width = ( (uint32_t)( 14.7456 * 65536 ) * horiz_pw ) >> 16 ;
	pulse_ptr -> max_delta = pulse_def_set -> hspeed ;
	pulse_ptr -> max_delta_frac = 0 ;
}

//*********************************************************************
// Process a hex config command.  Copy the data from the command
// workspace into the command structure.
//*********************************************************************
void do_xcmd ( void )
{
	xconfig = cmd_wks . xconfig ;
}

//*********************************************************************
// Functions that get called when a hex sequencer config command
// is recognized.
//*********************************************************************
void parse_queryhex ( void )
{
	uint8_t move_frac ;	// 0-9 indicating 0 - 90% of move complete
	
	move_frac = ( ( hex_move_timer >> 8 ) * 10 ) >> 8 ;
	qresult_char ( ( hex_state << 4 ) + 9 - move_frac ) ;
	goto_wait_cmd ( ) ;
}
void parse_xstop ( void )
{
	uint16_t dummy ;
	
	xargptr = & dummy ;
	store_xarg ( ) ;
	cmd_wks . xconfig . started = FALSE ;
}
void parse_xspeed ( void )
{
	xargptr = & cmd_wks . xconfig . xspeed ;
	ptr_store_arg_func = store_xspeed ;
	goto_parse_int16_sign ( ) ;
}
void parse_ldisp ( void )
{
	xargptr = & cmd_wks . xconfig . ldisp ;
	ptr_store_arg_func = store_ldisp ;
	goto_parse_int16_sign ( ) ;
}
void parse_rdisp ( void )
{
	xargptr = & cmd_wks . xconfig . rdisp ;
	ptr_store_arg_func = store_rdisp ;
	goto_parse_int16_sign ( ) ;
}
void parse_llow ( void )
{
	xargptr = & cmd_wks . xconfig . llow ;
	ptr_store_arg_func = store_xarg ;
	goto_parse_int16_sign ( ) ;
}
void parse_lmid ( void )
{
	xargptr = & cmd_wks . xconfig . lmid ;
	ptr_store_arg_func = store_xarg ;
	goto_parse_int16_sign ( ) ;
}
void parse_lhigh ( void )
{
	xargptr = & cmd_wks . xconfig . lhigh ;
	ptr_store_arg_func = store_xarg ;
	goto_parse_int16_sign ( ) ;
}
void parse_rlow ( void )
{
	xargptr = & cmd_wks . xconfig . rlow ;
	ptr_store_arg_func = store_xarg ;
	goto_parse_int16_sign ( ) ;
}
void parse_rmid ( void )
{
	xargptr = & cmd_wks . xconfig . rmid ;
	ptr_store_arg_func = store_xarg ;
	goto_parse_int16_sign ( ) ;
}
void parse_rhigh ( void )
{
	xargptr = & cmd_wks . xconfig . rhigh ;
	ptr_store_arg_func = store_xarg ;
	goto_parse_int16_sign ( ) ;
}
void parse_vspeed ( void )
{
	xargptr = & cmd_wks . xconfig . vspeed ;
	ptr_store_arg_func = store_xarg ;
	goto_parse_int16_sign ( ) ;
}
void parse_lrear ( void )
{
	xargptr = & cmd_wks . xconfig . lrear ;
	ptr_store_arg_func = store_xarg ;
	goto_parse_int16_sign ( ) ;
}
void parse_lfront ( void )
{
	xargptr = & cmd_wks . xconfig . lfront ;
	ptr_store_arg_func = store_xarg ;
	goto_parse_int16_sign ( ) ;
}
void parse_rrear ( void )
{
	xargptr = & cmd_wks . xconfig . rrear ;
	ptr_store_arg_func = store_xarg ;
	goto_parse_int16_sign ( ) ;
}
void parse_rfront ( void )
{
	xargptr = & cmd_wks . xconfig . rfront ;
	ptr_store_arg_func = store_xarg ;
	goto_parse_int16_sign ( ) ;
}
void parse_htime ( void )
{
	xargptr = & cmd_wks . xconfig . htime ;
	ptr_store_arg_func = store_xarg ;
	goto_parse_int16_sign ( ) ;
}


//*********************************************************************
// Store the hex speed percentage
//*********************************************************************
void store_xspeed ( void )
{
	store_xarg ( ) ;
	
	cmd_wks . xconfig . started = TRUE ;
}

//*********************************************************************
// Store the hex left horizontal displacement parameter with sign
//*********************************************************************
void store_ldisp ( void )
{
	store_xarg ( ) ;
	
	cmd_wks . xconfig . ldisp_neg = ! cmd_arg_positive ;
}

//*********************************************************************
// Store the hex right horizontal displacement parameter with sign
//*********************************************************************
void store_rdisp ( void )
{
	store_xarg ( ) ;
	
	cmd_wks . xconfig . rdisp_neg = ! cmd_arg_positive ;
}

//*********************************************************************
// Store the hex walking parameter
//*********************************************************************
void store_xarg ( void )
{
	goto_wait_cmd ( ) ;

	if ( cmd_type != CMDTYPE_XCMD )
	{
		cmd_type = CMDTYPE_XCMD ;
		cmd_wks . xconfig = xconfig ;
	}

	* xargptr = cmd_arg_int ;
}

#endif
