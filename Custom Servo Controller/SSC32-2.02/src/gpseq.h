//*********************************************************************
// General purpose sequencer macros, externs, and prototypes.
// Copyright 2006, Mike Dvorsky.
//*********************************************************************

#ifndef _gpseq_h_
#define _gpseq_h_


#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "compiler.h"

#include "numtypes.h"
#include "appdefs.h"
#include "globals.h"

#if GENERAL_SEQUENCER_ENABLE                 
void process_gpseq ( void ) ;
void update_player ( void ) ;
void parse_pl ( void ) ;
void parse_sq ( void ) ;
void parse_sm ( void ) ;
void parse_pa ( void ) ;
void parse_ix ( void ) ;
void parse_qpl ( void ) ;
void parse_once ( void ) ;
void store_pl ( void ) ;
void store_sq ( void ) ;
void store_sm ( void ) ;
void store_pa ( void ) ;
void store_ix ( void ) ;
void store_qpl ( void ) ;
void do_gpseq ( void ) ;
void do_goto ( void );
#endif //GENERAL_SEQUENCER_ENABLE

#endif // _gpseq_h_
