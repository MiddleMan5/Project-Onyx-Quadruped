//*********************************************************************
// Register read/write macros, externs, and prototypes.
// Copyright 2006, Mike Dvorsky.
//*********************************************************************

#ifndef _regrw_h_
#define _regrw_h_


#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "compiler.h"

#include "numtypes.h"
#include "appdefs.h"
#include "globals.h"

#if REGRW_ENABLE                 
void parse_r ( void ) ;
void parse_eq ( void ) ;
void parse_rdflt ( void );
void store_r ( void ) ;
void store_eq ( void ) ;
void do_regrw ( void ) ;
void do_rdflt ( void );
void reg_rdhdlr_enable ( uint8_t reg_index ) ;
void reg_rdhdlr_tdl ( uint8_t reg_index ) ;
void reg_rdhdlr_tpa ( uint8_t reg_index ) ;
void reg_rdhdlr_unitaddr ( uint8_t reg_index ) ;
void reg_rdhdlr_baudrate ( uint8_t reg_index ) ;
void reg_rdhdlr_poffs ( uint8_t reg_index ) ;
void reg_rdhdlr_startpos ( uint8_t reg_index ) ;
void reg_rdhdlr_minpw ( uint8_t reg_index ) ;
void reg_rdhdlr_maxpw ( uint8_t reg_index ) ;
void reg_rdhdlr_maxspeed ( uint8_t reg_index ) ;
void reg_wrhdlr_enable ( uint8_t reg_index ) ;
void reg_wrhdlr_tdl ( uint8_t reg_index ) ;
void reg_wrhdlr_tpa ( uint8_t reg_index ) ;
void reg_wrhdlr_unitaddr ( uint8_t reg_index ) ;
void reg_wrhdlr_baudrate ( uint8_t reg_index ) ;
void reg_wrhdlr_poffs ( uint8_t reg_index ) ;
void reg_wrhdlr_startpos ( uint8_t reg_index ) ;
void reg_wrhdlr_minpw ( uint8_t reg_index ) ;
void reg_wrhdlr_maxpw ( uint8_t reg_index ) ;
void reg_wrhdlr_maxspeed ( uint8_t reg_index ) ;
#endif //REGRW_ENABLE

#endif // _regrw_h_
