//*********************************************************************
// EEPROM macros, externs, and prototypes.
// Copyright 2006, Mike Dvorsky.
//*********************************************************************

#ifndef _eeprom_h_
#define _eeprom_h_


#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "compiler.h"

#include "numtypes.h"
#include "appdefs.h"
#include "globals.h"

#if EEPROM_ENABLE                 
void parse_eew ( void ) ;
void parse_eer ( void ) ;
void parse_ee_data ( void ) ;
void parse_ee_nbytes ( void ) ;
void store_eew ( void ) ;
void store_eer ( void ) ;
void store_ee_data ( void ) ;
void store_ee_nbytes ( void ) ;
void do_eew ( void ) ;

#if ( INTERNAL_EEPROM_ENABLE )
uint8_t eei_get_byte ( uint16_t addr ) ;
uint16_t eei_get_word( uint16_t addr );
uint8_t eei_put_byte ( uint16_t addr, uint8_t value);
uint16_t eei_put_word(uint16_t addr, uint16_t value);
#endif

#if ( EXTERNAL_EEPROM_ENABLE )
void twi_start ( void ) ;
void twi_stop ( void ) ;
void twi_write ( uint8_t data ) ;
void eex_write_addr ( uint16_t addr ) ;
uint8_t eex_read_get_data ( void ) ;
void eex_start_read ( uint16_t addr, uint8_t nbytes ) ;
//uint8_t eex_get_byte ( uint16_t addr ) ;
#endif //EXTERNAL_EEPROM_ENABLE

#endif //EEPROM_ENABLE

#endif // _eeprom_h_
