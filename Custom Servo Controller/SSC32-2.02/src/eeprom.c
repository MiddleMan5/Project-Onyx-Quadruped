//*********************************************************************
// Routines to manage EEPROM reads and writes for the SSC-32.
// Copyright 2006, Mike Dvorsky.
//
// EEPROM command format:
// "EEW addr , data , data , data <cr>"   (internal EEPROM)
// "EEW -addr , data , data , data <cr>"  (external EEPROM)
//    where the ", data " can be repeated up to 32 data bytes total
//    
// "EER addr ; nbytes"
//    where "nbytes" can be up to 32 bytes
//
// Address, data, and nbytes for the commands are all ASCII decimal
// numbers.  The returned data for a read command is binary.
//*********************************************************************

#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "compiler.h"

#include "numtypes.h"
#include "appdefs.h"
#include "globals.h"
#include "eeprom.h"

//*********************************************************************
// Global variables.
//*********************************************************************

// NOTE:  No global variables (or static) allowed in this file.  See 
// the note in globals.c for an explanation.

#if EEPROM_ENABLE

//*********************************************************************
// Get a byte of data from internal EEPROM.  The argument is the
// address of the byte, 0-511.  If the argument is out of range, it
// rolls over.
//*********************************************************************
#if INTERNAL_EEPROM_ENABLE
uint8_t eei_get_byte(uint16_t addr)
{
	// No need to wait for previous write to complete since that delay
	// is handled in the write function.
	
	// Set up address register
	EEAR = addr & 511 ;
	// Start EEPROM read
	EECR |= _BV(EERE);
	// Return data from read
	return EEDR ;
}
uint16_t eei_get_word( uint16_t addr )
{
    uint16_t value;
    
    value = eei_get_byte(addr);
    value |= eei_get_byte(addr+1) << 8;
    
    return value;
}
uint8_t eei_put_byte ( uint16_t addr, uint8_t value)
{
    // Set up address and data registers
    EEAR = addr ;
    EEDR = value ;
    // Start EEPROM write
    EECR |= _BV(EEMPE);
    EECR |= _BV(EEPE) ;
    // Wait for write to complete
    while (EECR & _BV(EEPE)) ;
}
uint16_t eei_put_word(uint16_t addr, uint16_t value)
{
    eei_put_byte(addr, value & 0xff);
    eei_put_byte(addr+1, (value >> 8) & 0xff);
    return value;
}
#endif

//*********************************************************************
// Write the buffered data to EEPROM.
//*********************************************************************
void do_eew ( void )
{
	uint8_t index ;

#if ( ( INTERNAL_EEPROM_ENABLE ) && ( EXTERNAL_EEPROM_ENABLE ) )
	if ( eerw_internal )
	{
#endif
#if ( INTERNAL_EEPROM_ENABLE )
		// Write to internal EEPROM
		for ( index = 0 ; index < eew_index ; ++ index )
		{
			// Set up address and data registers
			EEAR = eerw_addr ;
			EEDR = cmd_wks . eew_buffer [ index ] ;
			eerw_addr = ( eerw_addr + 1 ) & 511 ;
			// Start EEPROM write
			EECR |= _BV(EEMPE);
			EECR |= _BV(EEPE) ;
			// Wait for write to complete
			while (EECR & _BV(EEPE)) ;
		}
#endif
#if ( ( INTERNAL_EEPROM_ENABLE ) && ( EXTERNAL_EEPROM_ENABLE ) )
	}
	else
	{
#endif
#if ( EXTERNAL_EEPROM_ENABLE )
		// Write to external EEPROM
		eex_write_addr ( eerw_addr ) ;
		for ( index = 0 ; index < eew_index ; ++index, ++eerw_addr )
		{
			// On rollover to a new 64-byte page, allow page write to complete
			// then send the start address for the new page.
			if ( ( ( eerw_addr & 63 ) == 0 ) && index != 0 )
			{
				twi_stop ( ) ;
				delay_ms ( 5 ) ;	// Wait for write to complete
				eex_write_addr ( eerw_addr ) ;
			}
			twi_write ( cmd_wks . eew_buffer [ index ] ) ;
		}
		twi_stop () ;
		delay_ms ( 5 ) ;	//Wait for write to complete
#endif
#if ( ( INTERNAL_EEPROM_ENABLE ) && ( EXTERNAL_EEPROM_ENABLE ) )
	}
#endif
}

//*********************************************************************
// Store functions for EEPROM, called when the data for a command is
// received.
//*********************************************************************
void store_eew ( void )
{
	cmd_type = CMDTYPE_EEW ;
	eerw_addr = cmd_arg_int ;
	eerw_internal = cmd_arg_positive ;
	eew_index = 0 ;
}

void store_eer ( void )
{
	cmd_type = CMDTYPE_EER ;
	eerw_addr = cmd_arg_int ;
	eerw_internal = cmd_arg_positive ;
}

void store_ee_data ( void )
{
	if ( ( cmd_type == CMDTYPE_EEW ) && ( eew_index < 32 ) )
	{
    	cmd_wks . eew_buffer [ eew_index++ ] = cmd_arg_int ;
	}
}

void store_ee_nbytes ( void )
{
	uint8_t nbytes ;
	uint8_t data ;
	
	if ( cmd_type != CMDTYPE_EER )
	{
		return ;
	}
	
	for ( nbytes = cmd_arg_int ; nbytes > 0 ; -- nbytes )
	{
#if ( ( INTERNAL_EEPROM_ENABLE ) && ( EXTERNAL_EEPROM_ENABLE ) )
		if ( eerw_internal )
		{
#endif
#if ( INTERNAL_EEPROM_ENABLE )
			data = eei_get_byte ( eerw_addr ) ;
#endif
#if ( ( INTERNAL_EEPROM_ENABLE ) && ( EXTERNAL_EEPROM_ENABLE ) )
		}
		else
		{
#endif
#if ( EXTERNAL_EEPROM_ENABLE )
			eex_start_read ( eerw_addr, 1 ) ;
			data = eex_read_get_data ( ) ;
#endif
#if ( ( INTERNAL_EEPROM_ENABLE ) && ( EXTERNAL_EEPROM_ENABLE ) )
		}
#endif
		++ eerw_addr ;
		qresult_char ( data ) ;
	}
}

//*********************************************************************
// Parse functions for EEPROM, called when a command is recognized.
//*********************************************************************
void parse_eew ( void )
{
	ptr_store_arg_func = store_eew ;
	goto_parse_int16_sign ( ) ;
}

void parse_eer ( void )
{
	ptr_store_arg_func = store_eer ;
	goto_parse_int16_sign ( ) ;
}

void parse_ee_data ( void )
{
	ptr_store_arg_func = store_ee_data ;
	goto_parse_int8_sign ( ) ;
}

void parse_ee_nbytes ( void )
{
	ptr_store_arg_func = store_ee_nbytes ;
	goto_parse_int16_sign ( ) ;
}

#if ( EXTERNAL_EEPROM_ENABLE )
//*********************************************************************
// Two-wire interface (I2C) functions, used to access the external
// EEPROM.
//*********************************************************************

// Wait for the TWINT bit in TWCR to be '1'.  Time out if it 
// takes more than 255 loops.
void wait_twint ( void )
{
    uint8_t counter = 0xFF;
    
    while ( ( counter > 0 ) && ( ( TWCR & _BV(TWINT) ) == 0 ) )
    {
        -- counter;
    }
}

// Start a two-wire transaction.
void twi_start ( void )
{
    TWCR = _BV(TWINT) | _BV(TWSTA) | _BV(TWEN);
    wait_twint( );
}

// Write a byte of data on the two-wire bus.
void twi_write ( uint8_t data )
{
	TWDR = data ;
	TWCR = _BV(TWINT) | _BV(TWEN) ;
    wait_twint( );
}

void twi_stop ( void )
{
    TWCR = _BV(TWINT) | _BV(TWSTO) | _BV(TWEN) ;
}

//*********************************************************************
// External EEPROM functions.
//*********************************************************************
void eex_write_addr ( uint16_t addr )
{
	twi_start () ;
	twi_write ( EEPROM_BUS_ADDRESS | I2C_WRITE ) ;
	twi_write ( addr >> 8 ) ;
	twi_write ( addr ) ;
}

//*********************************************************************
// Functions for reading EEPROM.  In order to read N bytes starting
// at address A, first call eex_start_read, passing it the address
// and number of bytes.  Then call eex_read_get_data N times.  If
// eex_read_get_data is called more than N times, or if there is
// no EEPROM chip present, the returned value will be 0xFF.  If
// eex_read_get_data is called fewer than N times, then the final
// i2c STOP condition will not be sent, leaving the read operation
// incomplete--this has the potential to affect subsequent accesses
// to the EEPROM.
//
// Example:
//   eex_start_read ( A, N ) ;
//   for ( i = 0 ; i < N ; ++i )
//   {
//       buffer [ i ] = eex_read_get_data ( ) ;
//   }
// 
//*********************************************************************
void eex_start_read ( uint16_t addr, uint8_t nbytes )
{
#if EEX_IS_EEI
eex_read_nbytes = addr ;
#else
	// Send the starting address for the read
	eex_write_addr ( addr ) ;

	// Send the slave address with direction = READ
	twi_start () ;
	twi_write ( EEPROM_BUS_ADDRESS | I2C_READ ) ;

	// SIf the slave address was not ACK'ed, then set the number
	// of bytes for this read to 0.  If the slave address was
	// ACK'ed, then start the data read and set the global for
	// number of bytes to the passed value.
    if ( ( TWSR & 0xF8 ) == EEX_SLA_R_NACK )
    {
    	eex_read_nbytes = 0 ;
    }
 	else
    {
    	if ( nbytes == 1 )
    	{
    		// Use NACK if only 1 byte
	    	TWCR = TWI_READ_NACK ;
	    }
	    else
    	{
    		// Use ACK for >1 byte
	    	TWCR = TWI_READ_ACK ;
	    }
		// Store the number of bytes to be read in a global
		eex_read_nbytes = nbytes ;
    }
#endif
}

uint8_t eex_read_get_data ( void )
{
#if EEX_IS_EEI
return eei_get_byte ( eex_read_nbytes++ ) ;
#else
	uint8_t data ;
	
	// Return immediately if the number of bytes remaining to read
	// is 0.  This indicates either that the slave address was
	// not ACK'ed at the start of the read, or that the caller is
	// trying to read more bytes than originally indicated.
	if ( eex_read_nbytes == 0 )
	{
		return 0xFF ;	// If no chip, looks like erased
	}
	
	// Wait for TWI read to complete
    wait_twint( );

	// Just read the data.  Assume the read was ACK'ed, since
	// the slave address was.
   	data = TWDR ;
   	
   	// Decrement the number of bytes remaining.  If this is
   	// the last byte, then read with NACK and send the stop.
   	switch ( --eex_read_nbytes )
   	{
   		case 0:
		    twi_stop();
		    break ;
   		case 1:
	   		TWCR = TWI_READ_NACK ;	// Init the final read with NACK
		    break ;
		default:
   			TWCR = TWI_READ_ACK ;	// Init the next read with ACK
    }
    
    return data ;
#endif
}
//*******************************************************

#endif //EXTERNAL_EEPROM_ENABLE

#endif //EEPROM_ENABLE
