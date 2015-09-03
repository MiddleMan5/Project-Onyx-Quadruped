//*********************************************************************
// Pulse and edge processing macros, externs, and prototypes.
// Copyright 2005, Mike Dvorsky.
//*********************************************************************

#ifndef _pulsedge_h_
#define _pulsedge_h_


#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "compiler.h"

#include "numtypes.h"
#include "appdefs.h"
#include "globals.h"

void copy_pulses_to_sorted_edges (
	uint8_t starting_pulse_index,
	uint8_t starting_edge_index ) ;

void set_edge_data_and_function_ptr (
	uint8_t starting_edge_index, 
	uint8_t function_ptr_normal_index ) ;

void pulse_and_edge_init ( void ) ;

void update_pulse_array ( void ) ;

uint16_t get_edge_data_pointer ( void ) ;

#endif // _pulsedge_h_
