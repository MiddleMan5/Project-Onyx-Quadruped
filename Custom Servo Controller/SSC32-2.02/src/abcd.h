//*********************************************************************
// ABCD input pins macros, externs, and prototypes.
// Copyright 2005, Mike Dvorsky.
//*********************************************************************

#ifndef _abcd_h_
#define _abcd_h_


#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "compiler.h"

#include "numtypes.h"
#include "appdefs.h"
#include "globals.h"

void debounce_pin ( uint8_t * ptr_flags, uint8_t raw_value ) ;

void debounce_abcd ( void ) ;

void read_v_abcd ( void ) ;

void parse_avalue ( void ) ;
void parse_bvalue ( void ) ;
void parse_cvalue ( void ) ;
void parse_dvalue ( void ) ;
void parse_alatch ( void ) ;
void parse_blatch ( void ) ;
void parse_clatch ( void ) ;
void parse_dlatch ( void ) ;
void store_abcd_value ( uint8_t query_result ) ;

void parse_va ( void ) ;
void parse_vb ( void ) ;
void parse_vc ( void ) ;
void parse_vd ( void ) ;

#endif // _abcd_h_
