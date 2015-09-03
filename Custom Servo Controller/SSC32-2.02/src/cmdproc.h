//*********************************************************************
// Command processing macros, externs, and prototypes.
// Copyright 2005-2006, Mike Dvorsky.
//*********************************************************************

#ifndef _cmdproc_h_
#define _cmdproc_h_

#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "compiler.h"

#include "numtypes.h"
#include "appdefs.h"
#include "globals.h"

//*********************************************************************
// Typedefs
//*********************************************************************

//*********************************************************************
// Externs
//*********************************************************************

//*********************************************************************
// Function prototypes
//*********************************************************************
void execute_command ( void ) ;

void process_commands ( void ) ;
void goto_parse_name ( void ) ;
void process_parse_name ( void ) ;
void goto_parse_int8_sign ( void ) ;
void process_parse_int8_sign ( void ) ;
void goto_parse_int8_int ( void ) ;
void process_parse_int8_int ( void ) ;
void goto_parse_int16_sign ( void ) ;
void process_parse_int16_sign ( void ) ;
void goto_parse_int16_int ( void ) ;
void process_parse_int16_int ( void ) ;
#if BINARY_COMMAND_ENABLE
void process_emul_snum ( void ) ;
void process_emul_pw ( void ) ;
#endif
void goto_wait_cmd ( void ) ;
void process_wait_cmd ( void ) ;
#if FRACTION_ARG_ENABLE
void goto_parse_int16_frac ( void ) ;
void process_parse_int16_frac ( void ) ;
#endif


void convert_servo_cmd_to_pulse_arr (
	uint8_t num_servos,
	uint8_t start_index,
	uint16_t cmd_time ) ;
//void convert_servo_cmd_to_pulse_arr ( void ) ;
void do_goboot ( void ) ;
void do_query ( void ) ;
#if BANKVAL_ENABLE
void do_bankupdate ( void ) ;
void parse_bankvalue ( void );
#endif
void do_offset ( void ) ;

void parse_servonum ( void ) ;
void parse_goboot ( void ) ;
void parse_servohigh ( void ) ;
void parse_servolow ( void ) ;
void parse_servopw ( void ) ;
void parse_querystatus ( void ) ;
void parse_querypw ( void ) ;
void parse_servospeed ( void ) ;
void parse_servomovetime ( void ) ;
void parse_ver ( void ) ;
void parse_offset ( void ) ;

void store_servonum ( void ) ;
void store_servo_pw ( void ) ;
void store_servo_speed ( void ) ;
void store_move_time ( void ) ;
void store_querypw ( void ) ;
#if BANKVAL_ENABLE
void store_bankvalue ( void ) ;
#endif
void store_offset ( void ) ;

//void reset_command_array ( void ) ;
void qresult_char ( uint8_t ch ) ;
void qresult_string ( flash_uint8_t * ch_str ) ;
void qresult_uint16 ( uint16_t n ) ;
void qresult_int16 ( int16_t n ) ;


void stop_servo ( uint8_t servonum ) ;

#if STARTUP_STRING_ENABLE
void parse_ss ( void ) ;
void do_ss ( void ) ;
void parse_sscat ( void ) ;
void storech_sscat ( void ) ;
void do_sscat ( void ) ;
void parse_ssdelete ( void ) ;
void store_ssdelete ( void ) ;
void do_ssdelete ( void ) ;
#endif

void parse_servostop ( void ) ;
void store_servostop ( void ) ;
void do_servostop ( void ) ;

#if MODIFIER_ENABLE
void parse_modifier(void);
#endif

void store_baud(void);
void parse_baud(void);
void do_baud(void);

#endif // _cmdproc_h_
