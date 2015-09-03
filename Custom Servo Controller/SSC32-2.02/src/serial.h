//*********************************************************************
// Serial TX/RX macros, externs, and prototypes.
// Copyright 2005-2006, Mike Dvorsky.
//*********************************************************************

#ifndef _serial_h_
#define _serial_h_


#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "compiler.h"

#include "numtypes.h"
#include "appdefs.h"
#include "globals.h"

boolean rx_get_char ( uint8_t * ch_ptr ) ;

void tx_char ( uint8_t ch ) ;

void set_baud (int16_t baud) ;

void serial_set_tdl ( uint16_t tdl ) ;
void serial_set_tpa ( uint16_t tdl ) ;


#endif // _serial_h_
