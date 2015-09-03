//*********************************************************************
// Routines for reading and writing registers in EEPROM.
// Copyright 2007, Mike Dvorsky.
//
// Supported command strings:
// "R r <cr>"	-- displays register 'r' contents in ASCII
// "R r = d <cr>"	-- stores data 'd' in register 'r'
//*********************************************************************

#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "compiler.h"

#include "numtypes.h"
#include "appdefs.h"
#include "globals.h"
#include "regrw.h"

//*********************************************************************
// Global variables.
//*********************************************************************

// NOTE:  No global variables (or static) allowed in this file.  See 
// the note in globals.c for an explanation.

#if REGRW_ENABLE

//*********************************************************************
// Find the entry in the register group array for the register passed.
// Return a pointer to the register group entry if found, else NULL.
//*********************************************************************
flash register_defs_t * regrw_find_reg_group ( uint8_t reg_num )
{
#if defined(__CODEVISIONAVR__)
    flash register_defs_t * ptr_reg_def ;
#else
    register_defs_t * ptr_reg_def ;
#endif

    // Search for the regsiter.  Registers 0-31 are individual
    // (global) registers; others are assumed to be in groups of
    // 32 channel-specific registers.
    if ( reg_num >= 32 )
    {
        reg_num &= 0xE0 ;     // Get group starting register
    }

    // Search the table for the register
    for ( ptr_reg_def = & register_groups [ 0 ] ;
	  (void *)pgm_word(ptr_reg_def->ptr_read_handler) != NULL ;
        ++ ptr_reg_def )
    {
        if ( pgm_word(ptr_reg_def->start_reg_num) == reg_num )
        {
            return ptr_reg_def ;
        }
    }

    // Not found, return NULL.
    return NULL ;
}

//*********************************************************************
// Do function for regrw, called when the terminating carriage return
// is received.
//*********************************************************************
void do_regrw ( void )
{
    uint8_t reg_num ;
#if defined(__CODEVISIONAVR__)
    flash register_defs_t * ptr_reg_def ;
#elif defined(__GNUC__)
    register_defs_t * ptr_reg_def ;
#endif
    void (*fp)(uint8_t arg);

    reg_num = cmd_wks . regrw_cmd . reg_offset ;

    // Find the register information.    
    ptr_reg_def = regrw_find_reg_group ( reg_num ) ;
    if ( ptr_reg_def == NULL )
    {
        return ;
    }
    
    // Call the appropriate read or write handler
    // Get the offset of the register in its group
    if ( reg_num < 32 )
    {
        // Registers 0-31 are all offset 0
        reg_num = 0 ;
    }
    reg_num &= 0x1F;

    // Call the appropriate read or write handler
    if ( cmd_wks . regrw_cmd . write )
    {
	fp = (void (*)(uint8_t))pgm_word(ptr_reg_def->ptr_write_handler);
	fp( reg_num ) ;
    }
    else
    {
        fp = (void (*)(uint8_t))pgm_word(ptr_reg_def->ptr_read_handler);
	fp( reg_num ) ;
        do_query ( ) ;
    }
}

//*********************************************************************
// Do function for regdflt, called when the terminating carriage return
// is received.
// Scans through the array of registers, storing the default value
// of each.
//*********************************************************************
void do_rdflt ( void )
{
#if defined(__CODEVISIONAVR__)
    flash register_defs_t * ptr_reg_def ;
#elif defined(__GNUC__)
    register_defs_t * ptr_reg_def ;
#endif
    uint8_t reg_num ;
    uint8_t reg_offset ;
    void (*fp)(uint8_t);
    
    reg_num = 0 ;
    do {
        // Get a pointer to the register group, if any.
        ptr_reg_def = regrw_find_reg_group ( reg_num ) ;
        if ( ptr_reg_def == NULL )
        {
            continue ;
        }
        
        // Get the offset of the register in the group.  The offset
        // is 0 for registers 0-31; otherwise it is reg_num % 32 (i.e.
        // the least significant 5 bits of reg_num).
        if ( reg_num < 32 )
        {
            reg_offset = 0 ;
        }
        else
        {
            reg_offset = reg_num & 0x1F ;
        }

        // Call the write handler.
        cmd_arg_int = pgm_word(ptr_reg_def->default_value) ;
        cmd_arg_positive = TRUE ;   // Assume all default values positive
	fp = (void (*)(uint8_t))pgm_word(ptr_reg_def->ptr_write_handler);
	fp( reg_offset ) ;
     } while ( ++reg_num != 0 ) ;

     // Transmit completion message:  "OK\n"
     tx_char ( 'O' ) ;
     tx_char ( 'K' ) ;
     tx_char ( 0x0D ) ;
}

//*********************************************************************
// Store function for the R command, called when data for the command
// is received.
//*********************************************************************
void store_r ( void )
{
	// If register offset is valid, save value and set cmd_type.
	if ( cmd_arg_int <= 255 )
	{
		cmd_type = CMDTYPE_REGRW ;
		cmd_wks . regrw_cmd . reg_offset = (uint8_t) cmd_arg_int ;
		cmd_wks . regrw_cmd . write = FALSE ;
	}
}

//*********************************************************************
// Store function for the EQ command, called when data for the command
// is received.
//*********************************************************************
void store_eq ( void )
{
	if ( cmd_type != CMDTYPE_REGRW )
	{
		return ;
	}

    cmd_wks . regrw_cmd . reg_value = cmd_arg_int ;
    cmd_wks . regrw_cmd . reg_value_positive = cmd_arg_positive ;
    cmd_wks . regrw_cmd . write = TRUE ;
}

//*********************************************************************
// Parse functions for regrw, called when a command is recognized.
//*********************************************************************
void parse_r ( void )
{
	ptr_store_arg_func = store_r ;
	goto_parse_int16_sign ( ) ;
}

void parse_eq ( void )
{
	ptr_store_arg_func = store_eq ;
	goto_parse_int16_sign ( ) ;
}

void parse_rdflt ( void )
{
	cmd_type = CMDTYPE_RDFLT ;
	goto_wait_cmd ( ) ;
}

//*********************************************************************
// Handler functions for register read/write
//*********************************************************************
void reg_rdhdlr_enable ( uint8_t reg_index )
{
    qresult_uint16 ( eep_word(reg_enable) ) ;
}

void reg_wrhdlr_enable ( uint8_t reg_index )
{
    if ( eep_word(reg_enable) != cmd_arg_int )
    {
        eep_wword(reg_enable, cmd_arg_int);
    }
}

void reg_rdhdlr_tdl ( uint8_t reg_index )
{
    qresult_uint16(eep_word(reg_tdl)) ;
}

void reg_wrhdlr_tdl ( uint8_t reg_index )
{
    if ( eep_word(reg_tdl) != cmd_arg_int )
    {
        eep_wword(reg_tdl, cmd_arg_int);
    }
    serial_set_tdl ( cmd_arg_int ) ;
}

void reg_rdhdlr_tpa ( uint8_t reg_index )
{
    qresult_uint16 ( eep_word(reg_tpa) ) ;
}

void reg_wrhdlr_tpa ( uint8_t reg_index )
{
    if ( eep_word(reg_tpa) != cmd_arg_int )
    {
        eep_wword(reg_tpa, cmd_arg_int);
    }
    serial_set_tpa ( cmd_arg_int ) ;
}

void reg_rdhdlr_unitaddr ( uint8_t reg_index )
{
    qresult_uint16 ( eep_byte(reg_unitaddr) ) ;
}

void reg_wrhdlr_unitaddr ( uint8_t reg_index )
{
    if ( eep_byte(reg_unitaddr) != cmd_arg_int )
    {
        eep_wbyte(reg_unitaddr, cmd_arg_int);
    }
}

void reg_rdhdlr_baudrate ( uint8_t reg_index )
{
    qresult_uint16 ( eep_byte(reg_baudrate) ) ;
}

void reg_wrhdlr_baudrate ( uint8_t reg_index )
{
    if ( eep_byte(reg_baudrate) != cmd_arg_int )
    {
        eep_wbyte(reg_baudrate, cmd_arg_int);
    }
}

void reg_rdhdlr_poffs ( uint8_t reg_index )
{
    qresult_int16 ( (int8_t) eep_byte(reg_poffs[reg_index]) ) ;
}

void reg_wrhdlr_poffs ( uint8_t reg_index )
{
    int8_t po ;
    
    if ( cmd_arg_int > 100 )
    {
        po = 100 ;
    }
    else
    {
        po = cmd_arg_int ;
    }
    
    if ( !cmd_arg_positive )
    {
        po = -po ;
    }
    
    if ( eep_byte(reg_poffs[reg_index]) != po )
    {
        eep_wbyte(reg_poffs[reg_index], po);
    }
}

void reg_rdhdlr_startpos ( uint8_t reg_index )
{
    qresult_uint16 ( eep_word(reg_startpos [ reg_index ]) ) ;
}

void reg_wrhdlr_startpos ( uint8_t reg_index )
{
    if ( eep_word(reg_startpos [ reg_index ]) != cmd_arg_int )
    {
        eep_wword(reg_startpos[reg_index], cmd_arg_int) ;
    }
}

void reg_rdhdlr_minpw ( uint8_t reg_index )
{
    qresult_uint16 ( eep_word(reg_minpw [ reg_index ]) ) ;
}

void reg_wrhdlr_minpw ( uint8_t reg_index )
{
    if ( eep_word(reg_minpw [ reg_index ]) != cmd_arg_int )
    {
        eep_wword(reg_minpw[reg_index], cmd_arg_int);
    }
}

void reg_rdhdlr_maxpw ( uint8_t reg_index )
{
    qresult_uint16 ( eep_word(reg_maxpw[reg_index]) ) ;
}

void reg_wrhdlr_maxpw ( uint8_t reg_index )
{
    if ( eep_word(reg_maxpw[reg_index]) != cmd_arg_int )
    {
        eep_wword(reg_maxpw[reg_index], cmd_arg_int);
    }
}

void reg_rdhdlr_maxspeed ( uint8_t reg_index )
{
    qresult_uint16 ( eep_word(reg_maxspeed[reg_index]) ) ;
}

void reg_wrhdlr_maxspeed ( uint8_t reg_index )
{
    if ( eep_word(reg_maxspeed[reg_index]) != cmd_arg_int )
    {
        eep_wword(reg_maxspeed[reg_index], cmd_arg_int);
    }
}

#endif //GENERAL_SEQUENCER_ENABLE
