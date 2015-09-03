//*********************************************************************
// Global variable declarations for SSC-32
// Copyright 2005-2006, Mike Dvorsky.
//*********************************************************************

#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "appdefs.h"

#include "numtypes.h"
#include "appdefs.h"
#include "globals.h"
#include "gpseq.h"
#include "regrw.h"
#include "cmdproc.h"
#include "eeprom.h"
#include "abcd.h"

//*********************************************************************
// Global variable declarations
//*********************************************************************

/* cH: the following comment is no longer true?? Or better: no longer
 * relevant?
 */

// NOTE:  All RAM global variables are defined in this file due to the
// need to place the rx_queue and tx_queue at specific addresses.
// There should be no global or static variables allocated in other
// modules, because they might be allocated before the rx_queue and
// tx_queue.  This will cause rx_queue and tx_queue to be placed at
// the wrong addresses unless they are located at a specific address
// using '@', in which case the other variable will be placed on top
// of rx_queue.

// NOTE:  All register global variables are defined in the main file,
// because that appears to be the only place such definitions can be
// made and still work correctly.

// Receive queue.  Needs to be less than 256 bytes in length.
uint8_t rx_queue [ RXQ_NBYTES ] ;

// Transmit queue.  Needs to be less than 256 bytes in length.
uint8_t tx_queue [ TXQ_NBYTES ] ;

// RXQ remove pointer.  The add pointer is declared in the main file
// so it can be allocated to a register.
uint8_t * rxq_remove_ptr ;

// TXQ number of bytes.  The remove pointer is declared in the main
// file so it can be allocated to a register.  There is no add pointer;
// it is computed as needed.
volatile uint8_t txq_nbytes ;

// State for command state sequencer.  Declared here so it can
// be allocated to a register.
uint8_t cmd_state ;

// Flag indicating whether the received character is valid.  Allocated
// in the main file.
//boolean rx_char_valid ;

// Received character for processing.  Allocated in the main file.
//uint8_t received_char ;

// Variables used in parsing numeric input
boolean cmd_arg_positive ;
uint16_t cmd_arg_int ;
boolean valid_digit ;

// EEW and EER command address information
#if EEPROM_ENABLE
uint16_t eerw_addr ;		// The address for an EER or EEW command
boolean eerw_internal ;		// TRUE if the EER/EEW command is for internal EEPROM
uint8_t eew_index ;			// Index of EEW data bytes
#endif

// Fractional portion of argument, if enabled
#if FRACTION_ARG_ENABLE
uint16_t cmd_arg_frac ;
uint8_t num_frac_digits ;
#endif

// Pointer to function to be called after numeric input is successfully
// parsed.  This function will store the numeric value in the
// appropriate location.
void ( * ptr_store_arg_func ) ( void ) ;

// Type of command being parsed.
uint8_t cmd_type ;

// Received channel number.
uint16_t rx_ch_num ;

// Received command execution time in millieconds.
uint16_t rx_time ;

// Array for storage of command name characters as they are
// received, and index into the array.
uint8_t rx_cmd_name [ MAX_CMD_NAME_LEN + 1 ] ;
uint8_t cmd_name_index ;

// Array of bytes for holding incoming commands.  Interpretation of
// the bytes depends on the type of command.
command_workspace_t cmd_wks ;

// Array for holding incoming servo commands and sequencer steps.
servo_command_def_t servo_cmd_wks [ 32 ] ;

// Array of edge structures.
edge_def_t edge_array [ MAX_EDGES ] ;

// Array of pulse structures, one for each of the 32 channels.
pulse_def_t pulse_array [ 32 ] ;

// Array of pulse offsets, one for each of the 32 channels.
int8_t pulse_offset_array [ 32 ] ;

// Flag indicating that a byte was received.  Used to control the LED.
boolean rx_char_status ;

// Number of 20ms ticks required by the command just processed
uint16_t ms_in_command ;

/* a group-move is pending, waiting for the current move to complete */
boolean move_pending ;

// Number of 20ms ticks remaining in the current command
uint16_t ms_remaining_in_command ;

// Debounce flags for input pins.  Initialize to value '1'.
uint8_t in_A_dbnc_flags = DBNC_VALUE_MASK | DBNC_COUNT_MASK ;
uint8_t in_B_dbnc_flags = DBNC_VALUE_MASK | DBNC_COUNT_MASK ;
uint8_t in_C_dbnc_flags = DBNC_VALUE_MASK | DBNC_COUNT_MASK ;
uint8_t in_D_dbnc_flags = DBNC_VALUE_MASK | DBNC_COUNT_MASK ;

// Only allocate storage for hex sequencer variables if enabled
#if HEXAPOD_SEQUENCER_ENABLE
    // Arguments for the hex walking configuration command
    xconfig_t xconfig ;
    
    // Pointer to the argument for hex config commands
    uint16_t * xargptr ;
    
    // Countdown timer for hex moves
    uint16_t hex_move_timer ;
    
    // State for hex sequencer
    uint8_t hex_state = 7 ;
    
    // Computed horizontal positions for right & left sides
    hex_pulse_def_set_t hex_left_pulse_def ;
    hex_pulse_def_set_t hex_right_pulse_def ;
#endif // HEXAPOD_SEQUENCER_ENABLED

// Global values for analog input channels
uint16_t va ;
uint16_t vb ;
uint16_t vc ;
uint16_t vd ;

// Number of cycles for pre-transmit delay and pacing delay
uint16_t tx_delay_cycles = DEFAULT_TX_DELAY_CYCLES ;
uint16_t tx_pacing_delay_cycles = DEFAULT_TX_PACING_DELAY_CYCLES ;

// State number for debug command strings used in the main loop
#if DEBUGCMD_ENABLE
uint8_t debugcmd ;
#endif

// Index for servo commands being entered
int8_t servo_cmd_index ;

// Array of sequence players
#if GENERAL_SEQUENCER_ENABLE
seq_player_control_t seq_player [ NUM_SEQ_PLAYERS ] ;
uint8_t cmd_player_num ;	// Player number from command
uint8_t check_player_num ;	// Player number for periodic processing
#endif

// Global indicating the number of bytes for a sequential read from
// external EEPROM.
#if EXTERNAL_EEPROM_ENABLE
uint8_t eex_read_nbytes ;
#endif

//*********************************************************************
//** Flash variables stored after this point.
//*********************************************************************

// Create an array in flash containing pointers to the pulse functions in
// the OCR1A ISR.
#if defined(__CODEVISIONAVR__)
#asm
.cseg
_oc1a_isr_function_ptrs:
.dw bank1_pulse				;// [ 0 ]
.dw bank1_pulse_loop		;// [ 1 ]
.dw bank2_pulse				;// [ 2 ]
.dw bank2_pulse_loop		;// [ 3 ]
.dw bank3_pulse				;// [ 4 ]
.dw bank3_pulse_loop		;// [ 5 ]
.dw bank4_pulse				;// [ 6 ]
.dw bank4_pulse_loop		;// [ 7 ]
.dw no_pulse				;// [ 8 ]
.dseg
#endasm
#elif defined(__GNUC__)
flash uint16_t oc1a_isr_function_ptrs[9] = {
    (uint16_t)bank1_pulse,
    (uint16_t)bank1_pulse_loop,
    (uint16_t)bank2_pulse,
    (uint16_t)bank2_pulse_loop,
    (uint16_t)bank3_pulse,
    (uint16_t)bank3_pulse_loop,
    (uint16_t)bank4_pulse,
    (uint16_t)bank4_pulse_loop,
    (uint16_t)no_pulse
};
#endif

flash uint8_t pulse_data_array [ 32 ] =
{
	// Bank 1
	~0x40 ,	// Pulse output 0  is QB = bit 6
	~0x20 ,	// Pulse output 1  is QC = bit 5
	~0x10 ,	// Pulse output 2  is QD = bit 4
	~0x08 ,	// Pulse output 3  is QE = bit 3
	~0x04 ,	// Pulse output 4  is QF = bit 2
	~0x01 ,	// Pulse output 5  is QH = bit 0
	~0x02 ,	// Pulse output 6  is QG = bit 1
	0xFF & ~0x80 ,	// Pulse output 7  is QA = bit 7
	// Bank 2
	0xFF & ~0x80 ,	// Pulse output 8  is QA = bit 7
	~0x40 ,	// Pulse output 9  is QB = bit 6
	~0x20 ,	// Pulse output 10 is QC = bit 5
	~0x10 ,	// Pulse output 11 is QD = bit 4
	~0x08 ,	// Pulse output 12 is QE = bit 3
	~0x04 ,	// Pulse output 13 is QF = bit 2
	~0x02 ,	// Pulse output 14 is QG = bit 1
	~0x01 ,	// Pulse output 15 is QH = bit 0
	// Bank 3
	~0x01 ,	// Pulse output 16 is QH = bit 0
	~0x02 ,	// Pulse output 17 is QG = bit 1
	~0x04 ,	// Pulse output 18 is QF = bit 2
	~0x08 ,	// Pulse output 19 is QE = bit 3
	~0x10 ,	// Pulse output 20 is QD = bit 4
	~0x20 ,	// Pulse output 21 is QC = bit 5
	~0x40 ,	// Pulse output 22 is QB = bit 6
	0xFF & ~0x80 ,	// Pulse output 23 is QA = bit 7
	// Bank 4
	0xFF & ~0x80 ,	// Pulse output 24 is QA = bit 7
	~0x01 ,	// Pulse output 25 is QH = bit 0
	~0x02 ,	// Pulse output 26 is QG = bit 1
	~0x04 ,	// Pulse output 27 is QF = bit 2
	~0x08 ,	// Pulse output 28 is QE = bit 3
	~0x10 ,	// Pulse output 29 is QD = bit 4
	~0x20 ,	// Pulse output 30 is QC = bit 5
	~0x40	// Pulse output 31 is QB = bit 6
} ;

// Software version number string
//flash uint8_t swver [ ] = "SSC32X V1.03\x0D" ;
//flash uint8_t swver [ ] = "SSC32-1.03XE-Beta\x0D" ;
//flash uint8_t swver [ ] = "SSC32-500us-TEST\x0D" ;
//flash uint8_t swver [ ] = "SSC32-1.04XE\x0D    " ;
#ifndef SWVER
# define SWVER "V2.01GP_BETA1-GCC" ;
#endif
flash uint8_t swver [ ] = "SSC32-"SWVER"\x0D" ;

/* Defining the command table with gcc is a nuisance. Gnah.  The order
 * of the entries here does not matter, just define for each command
 * one _named_ string constant CMD_str in flash.
 */
#define DEF_STRING(cmd, str) static flash uint8_t cmd##_str[] = str
#define CMD_ENTRY(cmd) { cmd##_str, cmd }

DEF_STRING(NULL, "");			// SENTINEL
DEF_STRING(parse_servonum, "#");		// SERVOMOVE
#if EEPROM_ENABLE
DEF_STRING(parse_ee_data, ",");			// EEPROM
#endif
#if BANKVAL_ENABLE
DEF_STRING(parse_bankvalue, ":");       // BANKVAL
#endif
#if EEPROM_ENABLE
DEF_STRING(parse_ee_nbytes, ";");		// EEPROM
#endif
#if REGRW_ENABLE
DEF_STRING(parse_eq, "=");		    	// REGRW
#endif
DEF_STRING(parse_avalue, "A");			// ABCD
DEF_STRING(parse_alatch, "AL");			// ABCD
DEF_STRING(parse_bvalue, "B");			// ABCD
DEF_STRING(parse_blatch, "BL");			// ABCD
DEF_STRING(parse_cvalue, "C");			// ABCD
DEF_STRING(parse_clatch, "CL");			// ABCD
DEF_STRING(parse_dvalue, "D");			// ABCD
DEF_STRING(parse_dlatch, "DL");			// ABCD
#if EEPROM_ENABLE
DEF_STRING(parse_eer, "EER");			// EEPROM
DEF_STRING(parse_eew, "EEW");			// EEPROM
#endif
DEF_STRING(parse_goboot, "GOBOOT");		// GOBOOT
DEF_STRING(parse_servohigh, "H");		// SERVOMOVE
#if HEXAPOD_SEQUENCER_ENABLE
DEF_STRING(parse_htime, "HT");			// HEXSEQ
#endif
#if GENERAL_SEQUENCER_ENABLE
DEF_STRING(parse_ix, "IX");				// GPSEQ
#endif
DEF_STRING(parse_servolow, "L");		// SERVOMOVE
#if HEXAPOD_SEQUENCER_ENABLE
DEF_STRING(parse_lfront, "LF");			// HEXSEQ
DEF_STRING(parse_lhigh, "LH");	  		// HEXSEQ
DEF_STRING(parse_llow, "LL");	  		// HEXSEQ
DEF_STRING(parse_lmid, "LM");	  		// HEXSEQ
DEF_STRING(parse_lrear, "LR");	   		// HEXSEQ
#endif
#if MODIFIER_ENABLE
DEF_STRING(parse_modifier, "M"); // modifier M+ == wait, M~ binary
#endif
#if GENERAL_SEQUENCER_ENABLE
DEF_STRING(parse_once, "ONCE");			// GPSEQ
#endif
DEF_STRING(parse_servopw, "P");			// SERVOMOVE
#if GENERAL_SEQUENCER_ENABLE
DEF_STRING(parse_pa, "PA");				// GPSEQ
DEF_STRING(parse_pl, "PL");				// GPSEQ
#endif
DEF_STRING(parse_offset, "PO");			// PULSEOFFS
DEF_STRING(parse_querystatus, "Q");		// QUERYST
DEF_STRING(parse_querypw, "QP");		// QUERYPW
#if GENERAL_SEQUENCER_ENABLE
DEF_STRING(parse_qpl, "QPL");			// GPSEQ
#endif
#if REGRW_ENABLE
DEF_STRING(parse_r, "R");		    	// REGRW
DEF_STRING(parse_rdflt, "RDFLT");    	// REGRW
#endif
#if HEXAPOD_SEQUENCER_ENABLE
DEF_STRING(parse_rfront, "RF");	   		// HEXSEQ
DEF_STRING(parse_rhigh, "RH");	   		// HEXSEQ
DEF_STRING(parse_rlow, "RL");	   		// HEXSEQ
DEF_STRING(parse_rmid, "RM");	   		// HEXSEQ
DEF_STRING(parse_rrear, "RR");	   		// HEXSEQ
#endif
DEF_STRING(parse_servospeed, "S");		// SERVOMOVE
#if GENERAL_SEQUENCER_ENABLE
DEF_STRING(parse_sm, "SM");				// GPSEQ
DEF_STRING(parse_sq, "SQ");				// GPSEQ
#endif
#if STARTUP_STRING_ENABLE
DEF_STRING(parse_ss, "SS");				// SS
DEF_STRING(parse_sscat, "SSCAT");		// SS
DEF_STRING(parse_ssdelete, "SSDEL");    // SS
#endif
DEF_STRING(parse_servostop, "STOP");	// SERVOSTOP
DEF_STRING(parse_servomovetime, "T");	// SERVOMOVE
DEF_STRING(parse_va, "VA");				// VABCD
DEF_STRING(parse_vb, "VB");				// VABCD
DEF_STRING(parse_vc, "VC");				// VABCD
DEF_STRING(parse_vd, "VD");				// VABCD
DEF_STRING(parse_ver, "VER");			// VER
#if HEXAPOD_SEQUENCER_ENABLE
DEF_STRING(parse_vspeed, "VS");	  		// HEXSEQ
DEF_STRING(parse_ldisp, "XL");	  		// HEXSEQ
DEF_STRING(parse_queryhex, "XQ");		// HEXSEQ
DEF_STRING(parse_rdisp, "XR");	 		// HEXSEQ
DEF_STRING(parse_xspeed, "XS");	 		// HEXSEQ
DEF_STRING(parse_xstop, "XSTOP");		// HEXSEQ
#endif
DEF_STRING(parse_baud, "BAUD");                 // bit-rate command
DEF_STRING(TERM, "\xFF");		// SENTINEL

// Command parsing table, must be sorted alphabetically by name.
// The first and last entries are sentinels.  Except for
// the sentinel entries, the function pointer must not be NULL.
// 
//flash cmd_parse_table_entry_t cmd_parse_table [ NUM_CMD_PARSE_TABLE_ENTRIES ] =
flash cmd_parse_table_entry_t cmd_parse_table [ ] =
{
	// First entry is sentinel, must be empty string
	CMD_ENTRY(NULL),					// SENTINEL
	CMD_ENTRY(parse_servonum),		// SERVOMOVE
#if EEPROM_ENABLE
	CMD_ENTRY(parse_ee_data),			// EEPROM
#endif
#if BANKVAL_ENABLE
	CMD_ENTRY(parse_bankvalue),       // BANKVAL
#endif
#if EEPROM_ENABLE
	CMD_ENTRY(parse_ee_nbytes),		// EEPROM
#endif
#if REGRW_ENABLE
	CMD_ENTRY(parse_eq),		    	// REGRW
#endif
	CMD_ENTRY(parse_avalue),			// ABCD
	CMD_ENTRY(parse_alatch),			// ABCD
	CMD_ENTRY(parse_bvalue),			// ABCD
	CMD_ENTRY(parse_baud),                 // bit-rate command
	CMD_ENTRY(parse_blatch),			// ABCD
	CMD_ENTRY(parse_cvalue),			// ABCD
	CMD_ENTRY(parse_clatch),			// ABCD
	CMD_ENTRY(parse_dvalue),			// ABCD
	CMD_ENTRY(parse_dlatch),			// ABCD
#if EEPROM_ENABLE
	CMD_ENTRY(parse_eer),			// EEPROM
	CMD_ENTRY(parse_eew),			// EEPROM
#endif
	CMD_ENTRY(parse_goboot),		// GOBOOT
	CMD_ENTRY(parse_servohigh),		// SERVOMOVE
#if HEXAPOD_SEQUENCER_ENABLE
	CMD_ENTRY(parse_htime),			// HEXSEQ
#endif
#if GENERAL_SEQUENCER_ENABLE
	CMD_ENTRY(parse_ix),				// GPSEQ
#endif
	CMD_ENTRY(parse_servolow),		// SERVOMOVE
#if HEXAPOD_SEQUENCER_ENABLE
	CMD_ENTRY(parse_lfront),			// HEXSEQ
	CMD_ENTRY(parse_lhigh),	  		// HEXSEQ
	CMD_ENTRY(parse_llow),	  		// HEXSEQ
	CMD_ENTRY(parse_lmid),	  		// HEXSEQ
	CMD_ENTRY(parse_lrear),	   		// HEXSEQ
#endif
#if MODIFIER_ENABLE
	CMD_ENTRY(parse_modifier),  // binary, sync group moves
#endif
#if GENERAL_SEQUENCER_ENABLE
	CMD_ENTRY(parse_once),			// GPSEQ
#endif
	CMD_ENTRY(parse_servopw),			// SERVOMOVE
#if GENERAL_SEQUENCER_ENABLE
	CMD_ENTRY(parse_pa),				// GPSEQ
	CMD_ENTRY(parse_pl),				// GPSEQ
#endif
	CMD_ENTRY(parse_offset),			// PULSEOFFS
	CMD_ENTRY(parse_querystatus),		// QUERYST
	CMD_ENTRY(parse_querypw),		// QUERYPW
#if GENERAL_SEQUENCER_ENABLE
	CMD_ENTRY(parse_qpl),			// GPSEQ
#endif
#if REGRW_ENABLE
	CMD_ENTRY(parse_r),		    	// REGRW
	CMD_ENTRY(parse_rdflt),    	// REGRW
#endif
#if HEXAPOD_SEQUENCER_ENABLE
	CMD_ENTRY(parse_rfront),	   		// HEXSEQ
	CMD_ENTRY(parse_rhigh),	   		// HEXSEQ
	CMD_ENTRY(parse_rlow),	   		// HEXSEQ
	CMD_ENTRY(parse_rmid),	   		// HEXSEQ
	CMD_ENTRY(parse_rrear),	   		// HEXSEQ
#endif
	CMD_ENTRY(parse_servospeed),		// SERVOMOVE
#if GENERAL_SEQUENCER_ENABLE
	CMD_ENTRY(parse_sm),				// GPSEQ
	CMD_ENTRY(parse_sq),				// GPSEQ
#endif
#if STARTUP_STRING_ENABLE
	CMD_ENTRY(parse_ss),				// SS
	CMD_ENTRY(parse_sscat),		// SS
	CMD_ENTRY(parse_ssdelete),    // SS
#endif
	CMD_ENTRY(parse_servostop),	// SERVOSTOP
	CMD_ENTRY(parse_servomovetime),	// SERVOMOVE
	CMD_ENTRY(parse_va),				// VABCD
	CMD_ENTRY(parse_vb),				// VABCD
	CMD_ENTRY(parse_vc),				// VABCD
	CMD_ENTRY(parse_vd),				// VABCD
	CMD_ENTRY(parse_ver),			// VER
#if HEXAPOD_SEQUENCER_ENABLE
	CMD_ENTRY(parse_vspeed),	  		// HEXSEQ
	CMD_ENTRY(parse_ldisp),	  		// HEXSEQ
	CMD_ENTRY(parse_queryhex),		// HEXSEQ
	CMD_ENTRY(parse_rdisp),	 		// HEXSEQ
	CMD_ENTRY(parse_xspeed),	 		// HEXSEQ
	CMD_ENTRY(parse_xstop),		// HEXSEQ
#endif
	// Last entry is sentinel, must be 0xFF
	{ TERM_str, NULL }				// SENTINEL
} ;

#if REGRW_ENABLE
// Definitions of the registers.  All registers from 0 to 31 are assumed
// to be global.  All registers 32 and up are assumed to be channel-specific
// and part of a group of 32.
flash register_defs_t register_groups [ ] =
{
    {
        // Enable, R0, 2 bytes, eeoffs 510-511
        0,                      // start_reg_num
        0,                      // default_value
        reg_rdhdlr_enable,      // ptr_read_handler
        reg_wrhdlr_enable       // ptr_write_handler
    },
    {
        // Transmit delay, R1, 2 bytes, eeoffs 508-509
        1,                      // start_reg_num
        600,                    // default_value
        reg_rdhdlr_tdl,         // ptr_read_handler
        reg_wrhdlr_tdl          // ptr_write_handler
    },
    {
        // Transmit pacing, R2, 2 bytes, eeoffs 506-507
        2,                      // start_reg_num
        70,                     // default_value
        reg_rdhdlr_tpa,         // ptr_read_handler
        reg_wrhdlr_tpa          // ptr_write_handler
    },
    {
        // Unit address, R3, 1 byte, eeoffs 505
        3,                      // start_reg_num
        0,                      // default_value
        reg_rdhdlr_unitaddr,    // ptr_read_handler
        reg_wrhdlr_unitaddr     // ptr_write_handler
    },
    {
        // Baud rate, R4, 1 byte, eeoffs 504
        4,                      // start_reg_num
        0,                      // default_value = 2400 Baud
        reg_rdhdlr_baudrate,    // ptr_read_handler
        reg_wrhdlr_baudrate     // ptr_write_handler
    },
    {
        // Pulse offset, R32-R63, 1 byte, eeoffs 448-479
        32,                     // start_reg_num
        0,                      // default_value
        reg_rdhdlr_poffs,       // ptr_read_handler
        reg_wrhdlr_poffs        // ptr_write_handler
    },
    {
        // Start position, R64-R95, 2 bytes, eeoffs 384-447
        64,                     // start_reg_num
        1500,                   // default_value
        reg_rdhdlr_startpos,    // ptr_read_handler
        reg_wrhdlr_startpos     // ptr_write_handler
    },
    {
        // Min width, R96-R127, 2 bytes, eeoffs 320-383
        // Resolution = 10us
        96,                     // start_reg_num
        500,                     // default_value
        reg_rdhdlr_minpw,       // ptr_read_handler
        reg_wrhdlr_minpw        // ptr_write_handler
    },
    {
        // Max width, R128-R159, 2 bytes, eeoffs 256-319
        // Resolution = 10us
        128,                     // start_reg_num
        2500,                    // default_value
        reg_rdhdlr_maxpw,       // ptr_read_handler
        reg_wrhdlr_maxpw        // ptr_write_handler
    },
    {
        // Max speed, R160-R191, 2 bytes, eeoffs 192-255
        160,                     // start_reg_num
        65535,                  // default_value
        reg_rdhdlr_maxspeed,    // ptr_read_handler
        reg_wrhdlr_maxspeed     // ptr_write_handler
    },
    {
        // Sentinel, indicating end of list.  Handler must be NULL
        0,                      // start_reg_num
        0,                      // default_value
        NULL,                   // ptr_read_handler
        NULL                    // ptr_write_handler
    }
        
} ;
#endif	

#if 0
// Constants used to convert from decimal digits to a fraction.  If N
// digits are being converted in the form 0.1234, then they can be
// stored as an integer (1234) and multiplied by the appropriate constant
// from this table, using the table entry corresponding to the number of
// decimal digits.  The result will be a binary number with scale factor
// 2^16.
flash uint32_t frac_mul [ ] =
{
	0,
	65536L * 65536L / 10 ,
	65536L * 65536L / 100 ,
	65536L * 65536L / 1000 ,
	65536L * 65536L / 10000
} ;
#endif	

// EEPROM variables
#if REGRW_ENABLE
// Reserved, @0
eeprom uint8_t eeprom_reserved_0 = 0 ;
// Startup string length, @1
eeprom uint8_t startup_string_nbytes = 0 ;
// Startup string, @2-101
eeprom uint8_t startup_string [ STARTUP_STRING_MAX_BYTES ] =
{
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
} ;
// Reserved, @102-191
eeprom uint8_t eeprom_reserved1 [ 90 ] =
{
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0
} ;
// Maximum speed array, R160-R191, @192-255
eeprom uint16_t reg_maxspeed [ 32 ] = 
{
    65535,65535,65535,65535,65535,65535,65535,65535,
    65535,65535,65535,65535,65535,65535,65535,65535,
    65535,65535,65535,65535,65535,65535,65535,65535,
    65535,65535,65535,65535,65535,65535,65535,65535
} ;
// Maximum pulse width array, R128-R159, @256-319
eeprom uint16_t reg_maxpw [ 32 ] = 
{
    2500,2500,2500,2500,2500,2500,2500,2500,
    2500,2500,2500,2500,2500,2500,2500,2500,
    2500,2500,2500,2500,2500,2500,2500,2500,
    2500,2500,2500,2500,2500,2500,2500,2500
} ;
// Minimum pulse width array, R96-R127, @320-383
eeprom uint16_t reg_minpw [ 32 ] = 
{
    500,500,500,500,500,500,500,500,
    500,500,500,500,500,500,500,500,
    500,500,500,500,500,500,500,500,
    500,500,500,500,500,500,500,500
} ;
// Starting position array, R64-R95, @384-447
eeprom uint16_t reg_startpos [ 32 ] = 
{
    1500,1500,1500,1500,1500,1500,1500,1500,
    1500,1500,1500,1500,1500,1500,1500,1500,
    1500,1500,1500,1500,1500,1500,1500,1500,
    1500,1500,1500,1500,1500,1500,1500,1500
} ;
// Pulse offset array, R32-63, @448-479
eeprom uint8_t reg_poffs [ 32 ] = 
{
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
} ;
// Reserved, @480-503
eeprom uint8_t eeprom_reserved2 [ 24 ] =
{
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0
} ;
// Baud rate, R4, @504
eeprom uint8_t reg_baudrate = 0 ;
// Unit address, R3, @505
eeprom uint8_t reg_unitaddr = 0 ;
// TX pacing, R2, @506-507
eeprom uint16_t reg_tpa = 70 ;
// TX delay, R1, @508-509
eeprom uint16_t reg_tdl = 600 ;
// Enable, R0, @510-511
eeprom uint16_t reg_enable = 0 ;
#endif  //REGRW_ENABLE
