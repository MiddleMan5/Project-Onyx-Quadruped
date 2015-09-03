//*********************************************************************
// Hexapod sequencer macros, externs, and prototypes.
// Copyright 2005, Mike Dvorsky.
//*********************************************************************

#ifndef _hexapod_h_
#define _hexapod_h_

// No declaractions if not enabled
#if HEXAPOD_SEQUENCER_ENABLE

#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include <mega168.h>
#include <spi.h>
#include <delay.h>

#include "numtypes.h"
#include "appdefs.h"
#include "globals.h"

#define HEX_RR_VERT_CHNUM	0
#define HEX_RR_HORIZ_CHNUM	1
#define HEX_RC_VERT_CHNUM	2
#define HEX_RC_HORIZ_CHNUM	3
#define HEX_RF_VERT_CHNUM	4
#define HEX_RF_HORIZ_CHNUM	5
#define HEX_LR_VERT_CHNUM	16
#define HEX_LR_HORIZ_CHNUM	17
#define HEX_LC_VERT_CHNUM	18
#define HEX_LC_HORIZ_CHNUM	19
#define HEX_LF_VERT_CHNUM	20
#define HEX_LF_HORIZ_CHNUM	21

void hex_sequencer ( void ) ;
void hex_update_leg (
	uint8_t state ,
	hex_pulse_def_set_t * pulse_def_set ,
	pulse_def_t * vert_pulse_ptr ) ;
void do_xcmd ( void ) ;
void parse_xstop ( void ) ;
void parse_xspeed ( void ) ;
void parse_ldisp ( void ) ;
void parse_rdisp ( void ) ;
void parse_lhigh ( void ) ;
void parse_lmid ( void ) ;
void parse_llow ( void ) ;
void parse_rhigh ( void ) ;
void parse_rmid ( void ) ;
void parse_rlow ( void ) ;
void parse_vspeed ( void ) ;
void parse_lrear ( void ) ;
void parse_lfront ( void ) ;
void parse_rrear ( void ) ;
void parse_rfront ( void ) ;
void parse_htime ( void ) ;
void parse_queryhex ( void ) ;

void store_ldisp ( void ) ;
void store_rdisp ( void ) ;
void store_xspeed ( void ) ;
void store_xarg ( void ) ;

#endif // HEXAPOD_SEQUENCER_ENABLE
#endif // _hexapod_h_
