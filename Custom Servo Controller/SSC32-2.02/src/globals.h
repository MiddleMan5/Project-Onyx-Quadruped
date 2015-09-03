//*********************************************************************
// Global variable typedefs and externs, and globally used macros.
// Copyright 2005-2006, Mike Dvorsky.
//*********************************************************************

#ifndef _globals_h_
#define _globals_h_


#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "compiler.h"

#include "numtypes.h"
#include "appdefs.h"

//*********************************************************************
// Defines
//*********************************************************************
// Number of entries in the command parse table.
// NOTE:  It would be preferable to have the number of entries
// calculated using sizeof(array)/sizeof(entry), but CVAVR
// (version 1.24.1e) does not handle this correctly.

#if HEXAPOD_SEQUENCER_ENABLE
	#define NUM_CMD_PARSE_TABLE_HEXSEQ		17
#else
	#define NUM_CMD_PARSE_TABLE_HEXSEQ		0
#endif

#if STARTUP_STRING_ENABLE
	#define NUM_CMD_PARSE_TABLE_SS			3
#else
	#define NUM_CMD_PARSE_TABLE_SS			0
#endif

#if BANKVAL_ENABLE
	#define NUM_CMD_PARSE_TABLE_BANKVAL		1
#else
	#define NUM_CMD_PARSE_TABLE_BANKVAL		0
#endif

#if EEPROM_ENABLE
	#define NUM_CMD_PARSE_TABLE_EEPROM		4
#else
	#define NUM_CMD_PARSE_TABLE_EEPROM		0
#endif

#if GENERAL_SEQUENCER_ENABLE
	#define NUM_CMD_PARSE_TABLE_GPSEQ		7
#else
	#define NUM_CMD_PARSE_TABLE_GPSEQ		0
#endif

#if REGRW_ENABLE
	#define NUM_CMD_PARSE_TABLE_REGRW		3
#else
	#define NUM_CMD_PARSE_TABLE_REGRW		0
#endif

#if MODIFIER_ENABLE
# define NUM_CMD_PARSE_TABLE_MODIFIER 1
#else
# define NUM_CMD_PARSE_TABLE_MODIFIER 0
#endif

#define NUM_CMD_PARSE_TABLE_SENTINEL            2
#define NUM_CMD_PARSE_TABLE_SERVOMOVE           6
#define NUM_CMD_PARSE_TABLE_SERVOSTOP           1
#define NUM_CMD_PARSE_TABLE_ABCD                8
#define NUM_CMD_PARSE_TABLE_GOBOOT              1
#define NUM_CMD_PARSE_TABLE_PULSEOFFS           1
#define NUM_CMD_PARSE_TABLE_QUERYST             1
#define NUM_CMD_PARSE_TABLE_QUERYPW             1
#define NUM_CMD_PARSE_TABLE_VABCD               4
#define NUM_CMD_PARSE_TABLE_VER                 1
#define NUM_CMD_PARSE_TABLE_BAUD                1

#define NUM_CMD_PARSE_TABLE_ENTRIES		\
  ( 0						\
    + NUM_CMD_PARSE_TABLE_SENTINEL		\
    + NUM_CMD_PARSE_TABLE_HEXSEQ		\
    + NUM_CMD_PARSE_TABLE_GPSEQ			\
    + NUM_CMD_PARSE_TABLE_SERVOMOVE		\
    + NUM_CMD_PARSE_TABLE_SERVOSTOP		\
    + NUM_CMD_PARSE_TABLE_BANKVAL		\
    + NUM_CMD_PARSE_TABLE_ABCD			\
    + NUM_CMD_PARSE_TABLE_GOBOOT		\
    + NUM_CMD_PARSE_TABLE_PULSEOFFS		\
    + NUM_CMD_PARSE_TABLE_QUERYST		\
    + NUM_CMD_PARSE_TABLE_QUERYPW		\
    + NUM_CMD_PARSE_TABLE_VABCD			\
    + NUM_CMD_PARSE_TABLE_VER			\
    + NUM_CMD_PARSE_TABLE_SS			\
    + NUM_CMD_PARSE_TABLE_EEPROM		\
    + NUM_CMD_PARSE_TABLE_REGRW 		\
    + NUM_CMD_PARSE_TABLE_MODIFIER		\
    + NUM_CMD_PARSE_TABLE_BAUD			\
    + 0 )


// 4 groups of edges, one per bank.  Each group has one "edge" to turn
// all 8 pulses on plus 8 edges to turn them off individually.  Following
// the 4 groups, there will be an additional 4 "edges" to time the 10ms
// until the next iteration.  Total edges = 9 * 4 + 4 = 40.
#define MAX_EDGES 40

// There are a total of 32 pulse outputs
#define MAX_PULSES 32

// The minimum number of cycles between edges
#define MIN_EDGE_DELTA 22

// The minimum number of cycles between edges for doing a normal
// return from the ISR of the earlier edge.
#define MIN_EDGE_DELTA_FOR_ISR_RET 128

// The edge ISR occurs several cycles prior to the actual edge
// time.  This #define specifies the number of cycles.
// NOTE:  This constant is hard coded as a literal in the ISR itself, 
//        so if it is changed here, then the number must be changed
//        in the ISR.
#define NCYLES_ISR_PRE_EDGE 64

// The minimum and maximum pulse width, in microseconds
#if ( EXTENDED_RANGE )
#define MIN_PULSE_WIDTH_US 500
#define MAX_PULSE_WIDTH_US 2500
#else
#define MIN_PULSE_WIDTH_US 750
#define MAX_PULSE_WIDTH_US 2250
#endif

// The minimum and maximum pulse width, in clock cycles
#define MIN_PULSE_WIDTH_CYCLES US_TO_CYCLES(MIN_PULSE_WIDTH_US)
#define MAX_PULSE_WIDTH_CYCLES US_TO_CYCLES(MAX_PULSE_WIDTH_US)

// Bank to bank delta, used to initialize the bank starting times.  This
// must be greater than the max pulse width.  Conservatively, set it to
// be 100 microseconds greater.
#if ( EXTENDED_RANGE )
#define BANK_TO_BANK_DELTA_US 2600
#else
#define BANK_TO_BANK_DELTA_US 2500
#endif

// ASCII character defines
#define ASCII_TAB	9
#define ASCII_LF	10
#define ASCII_CR	13
#define ASCII_ESC	27

// Parameter types for servo move commands
#define PARMTYPE_WIDTH 0
#define PARMTYPE_SPEED 1

#define RX_CHAR_INIT		0
#define RX_CHAR_NO_CHAR		1
#define RX_CHAR_RECEIVED	2

// Maximum length of a command name
#define MAX_CMD_NAME_LEN 8

// Defines for debouncing
#define DBNC_VALUE_MASK 0x80
#define DBNC_CHANGED_MASK 0x40
#define DBNC_FALLING_EDGE_MASK 0x20
#define DBNC_RISING_EDGE_MASK 0x10
#define DBNC_COUNT_MASK 0x0F

// Maximum number of result bytes in a query
#define MAX_QUERY_RESULT_BYTES 32

// Bit definitions for the enable register R0
#define REG_ENABLE_STARTUP_STRING_ENABLE    0x0001
#define REG_ENABLE_TXDELAY_ENABLE           0x0002
#define REG_ENABLE_STARTING_POFFS_ENABLE    0x0004
#define REG_ENABLE_STARTING_POSN_ENABLE     0x0008
#define REG_ENABLE_MINPW_ENABLE             0x0010
#define REG_ENABLE_MAXPW_ENABLE             0x0020
#define REG_ENABLE_MAX_SPEED_ENABLE         0x0040
#define REG_ENABLE_UNIT_ADDR_ENABLE         0x0080
#define REG_ENABLE_BAUDRATE_ENABLE          0x0100
#define REG_ENABLE_GLOBAL_DISABLE           0x8000

//*********************************************************************
// Typedefs
//*********************************************************************

// Structure to define the output edges for servo pulses
typedef struct edge_def_s
{
	uint8_t  edge_data ;		// Bit values to output
	uint16_t edge_time ;	 	// Time (TCNT1) to output edge bits
	uint16_t function_ptr ;		// Pointer to address of strobe function
} edge_def_t ;

// Structure to define the servo pulses.  The current_width and
// max_delta are stored as 24-bit quantities, with an integer
// and a fractional part.  The integer part is in clock cycles;
// the fractional part is 1/256 clock cycles per bit.
typedef struct pulse_def_s
{
	uint16_t current_width ;	// Current pulse width
	uint8_t current_width_frac ;	// Fractional part
	uint16_t target_width ;	 	// Target pulse width
	uint16_t max_delta ;		// Maximum change in width each 20ms cycle
	uint8_t max_delta_frac ;	// Fractional part
} pulse_def_t ;

// Structure containing function pointers within the OCR1A ISR
typedef struct oc1a_isr_function_ptrs_s
{
	uint16_t bank1_pulse ;
	uint16_t bank1_pulse_loop ;
	uint16_t bank2_pulse ;
	uint16_t bank2_pulse_loop ;
	uint16_t bank3_pulse ;
	uint16_t bank3_pulse_loop ;
	uint16_t bank4_pulse ;
	uint16_t bank4_pulse_loop ;
	uint16_t no_pulse ;
} oc1a_isr_function_ptrs_t ;

// Structure for command parsing
typedef flash struct cmd_parse_table_entry_s
{
	flash_uint8_t * ptr_cmdstr ;
	void ( * ptr_function ) ( void ) ;
} cmd_parse_table_entry_t ;

// Structure to define servo commands
typedef struct servo_command_def_s
{
	uint8_t servo_num ;		// Number of servo being modified
	uint16_t target_width ;		// Target pulse width in clock cycles
	uint16_t speed ;		// Speed in microseconds / second
} servo_command_def_t ;

// Structure to define an ABCD query
typedef struct query_result_s
{
	uint8_t num_bytes ;
	uint8_t result_string [ MAX_QUERY_RESULT_BYTES ] ;
} query_result_t ;

// Structure to define bank outputs
typedef struct bank_update_s
{
	boolean modified ;
	uint8_t bank_value ;
} bank_update_t ;

// Structure to define a hex walker configuration
typedef struct xconfig_s
{
    uint16_t lhigh ;
    uint16_t lmid ;
    uint16_t llow ;
    uint16_t lrear ;
    uint16_t lfront ;
    uint16_t rhigh ;
    uint16_t rmid ;
    uint16_t rlow ;
    uint16_t rrear ;
    uint16_t rfront ;
    uint16_t vspeed ;
    uint16_t htime ;
    uint16_t xspeed ;
    uint16_t ldisp ;
    uint16_t rdisp ;
    boolean ldisp_neg ;
    boolean rdisp_neg ;
    boolean started ;
} xconfig_t ;

// Bit masks for the flags element in seq_player_control_t.
// Operation during normal update:
// Bit  7 indicates when the player has been changed
// Bit  6 indicates that the sequence should be played only once
// Bits 5:0 are unused
#define SEQ_PL_FLAGS_MASK_CHANGE		    0x80
#define SEQ_PL_FLAGS_MASK_ONCE  		    0x40
#define SEQ_PL_FLAGS_MASK_STEP_COMPLETED  	0x20

// Operation when a command is being parsed:
// Bit 7 indicates that the sequence been changed
// Bit 6 indicates that the index been changed
// Bit 5 indicates that the speed has been changed
// Bit 4 indicates that the pause delay has been changed
// Bit 3 indicates that the ONCE option was specified
// Bit 2 indicates that a move time has been specified
// Bits 1:0 are unused
#define SEQ_PL_FLAGS_MASK_SEQ_CHANGE        0x80
#define SEQ_PL_FLAGS_MASK_STEPNUM_CHANGE    0x40
#define SEQ_PL_FLAGS_MASK_SPEED_CHANGE      0x20
#define SEQ_PL_FLAGS_MASK_PAUSE_CHANGE      0x10
#define SEQ_PL_FLAGS_MASK_ONCE_CHANGE  		0x08
#define SEQ_PL_FLAGS_MASK_TIME_CHANGE       0x04

// Player states
// STOPPED:  the sequence executing
// PLAYING:  a sequence is executing and not paused
// STARTING: a play command has been received, but not started playing yet
// STOPPING: was playing, commanded to stop, has not stopped yet
// PAUSING:  a sequence is executing but is paused between steps
// RESTING:  a sequence is executing but the speed has been set to 0
#define SEQ_PL_ST_STOPPED 0
#define SEQ_PL_ST_PLAYING 1
#define SEQ_PL_ST_STARTING 2
#define SEQ_PL_ST_STOPPING 3
#define SEQ_PL_ST_PAUSING 4
#define SEQ_PL_ST_RESTING 5
#define SEQ_PL_ST_INVALID 255

// Struct to hold information about a sequence player.
// If seq_num <= 127, the player is playing
// NON-PORTABILITY ALERT:  The speed and ms_remaining must
// be the first and second items, respectively, in order
// to support an assembly language optimization in gpseq.c.
typedef struct seq_player_control_s
{
	int16_t speed ;					// Speed multiplier
	uint16_t ms_remaining ;			// Number of ms remaining in step
	uint8_t seq_num ;				// Sequence number, 0-127 
	uint8_t flags ;					// Misc flags
	uint8_t num_servos ;			// Number of servos
	uint8_t num_steps ;				// Number of steps in the sequence
	uint8_t state ;					// Stopped, starting, etc.
	uint16_t seq_offset ;			// Offset in EEPROM of the start of the servo list
	uint8_t step_num ;				// Current step number, 0-255
//	uint8_t prev_step_num ;			// Current step number, 0-255
//	uint16_t total_ms ;				// Total number of ms in step
//	int8_t new_speed ;				// New speed multiplier value
	uint16_t pause_ms ;				// Number of ms to pause between steps
	uint8_t stop_step_num ;			// The step number to stop at for one-time play
} seq_player_control_t ;

// Typedef for regrw commands
typedef struct regrw_cmd_s
{
    uint8_t reg_offset ;
    uint16_t reg_value ;
    boolean reg_value_positive ;
    boolean write ;
} regrw_cmd_t ;

// Union for misc command workspace
typedef union command_workspace_u
{
	xconfig_t xconfig ;
	uint8_t eew_buffer [ 32 ] ;
	query_result_t query_result ;
	bank_update_t bank_update_command [ 4 ] ;
	int8_t pulse_offset_command [ 32 ] ;
	uint16_t tpa_cmd ;
	uint16_t tdl_cmd ;
	uint16_t ss_cmd ;
	seq_player_control_t pl_cmd ;
	regrw_cmd_t regrw_cmd ;
	uint16_t regdflt_cmd ;
	uint8_t ss_idx ;
	uint8_t stop_idx ;
} command_workspace_t ;

// Command types
typedef enum cmdtype_e
{
	CMDTYPE_NONE,
	CMDTYPE_PULSE,
	CMDTYPE_GOBOOT,
	CMDTYPE_QUERY,
	CMDTYPE_XCMD,
	CMDTYPE_BANKUPDATE,
	CMDTYPE_OFFSET,
	CMDTYPE_TPA,
	CMDTYPE_TDL,
	CMDTYPE_SS,
	CMDTYPE_EEW,
	CMDTYPE_EER,
	CMDTYPE_PLAY_SEQ,
	CMDTYPE_GOTO,
	CMDTYPE_REGRW,
	CMDTYPE_RDFLT,
	CMDTYPE_SSCAT,
	CMDTYPE_SSDELETE,
	CMDTYPE_SERVOSTOP,
	CMDTYPE_BAUD,
	// CMDTYPE_FINAL should be the last command type defined
	CMDTYPE_FINAL
} cmdtype_t ;

typedef enum cmdstate_e
{
    ST_WAIT_CMD,
    ST_EMUL_SNUM,
    ST_EMUL_PW,
    ST_PARSE_NAME,
    ST_PARSE_INT8_SIGN,
    ST_PARSE_INT8_INT,
    ST_PARSE_INT16_SIGN,
    ST_PARSE_INT16_INT,
    ST_PARSE_INT16_FRAC,
    ST_PARSE_INT16_HEX,
    ST_PARSE_MODIFIER,
    ST_SSCAT,
    // ST_FINAL should be the last state defined
    ST_FINAL
} cmdstate_t ;

// Struct to hold a set of horizontal pulse definitions
typedef struct hex_pulse_def_set_s
{
	uint16_t pw_front ;
	uint16_t pw_center ;
	uint16_t pw_rear ;
	uint16_t pw_low ;
	uint16_t pw_high ;
	uint16_t pw_mid ;
	uint16_t hspeed ;			// cycles / 20ms
	uint16_t vspeed_low_mid ;	// cycles / 20ms
	uint16_t vspeed_high_mid ;	// cycles / 20ms
} hex_pulse_def_set_t ;

// Typedef for EEPROM register definition structure    
typedef flash struct register_defs_s
{
    uint16_t start_reg_num ;    // Has extra bits for flags as needed
    uint16_t default_value ;
    void ( * ptr_read_handler ) ( uint8_t reg_index ) ;
    void ( * ptr_write_handler ) ( uint8_t reg_index ) ;
} register_defs_t ;
    
//*********************************************************************
// Global variable declarations
//*********************************************************************

#if defined(__CODEVISIONAVR__)
// Register global variables need not (and must not) be extern.  If
// they are extern, it can mess up the assignment to registers.
# define GLOBAL_REGVARS_DEFINED 0
#elif defined(__GNUC__)
# define GLOBAL_REGVARS_DEFINED 1
// Edge data pointer used by OC1A ISR, in R3:R2
REGISTER(uint16_t, edge_data_ptr, 2);

// Temporary storage for use by the ISR, R6, R7, R8, R9, and R14.
REGISTER(uint8_t, isr_tmp1, 6);
REGISTER(uint8_t, isr_tmp2, 7);
REGISTER(uint8_t, isr_tmp3, 8);
REGISTER(uint8_t, isr_tmp4, 9);
REGISTER(uint8_t, isr_tmp5, 14);

// RX queue add pointer, R10 & R11
//register uint16_t rxq_add_ptr @10 ;
REGISTER(uint8_t *, rxq_add_ptr, 10);

// TX queue remove pointer, R12 & R13
//register uint16_t txq_remove_ptr @12 ;
REGISTER(uint8_t *, txq_remove_ptr, 12);
   
// Flag indicating whether the received character is valid.
REGISTER(boolean, rx_char_valid, 4);

// Flag indicating whether the received character is valid
REGISTER(uint8_t, received_char, 5);
#define received_char (*(uint8_t *)0x05)

#endif

//*****
// NOTE:  The following 2 arrays need to be less then 256 bytes.
//*****
extern uint8_t rx_queue [ ] ;
extern uint8_t tx_queue [ ] ;

// RXQ remove pointer.  The add pointer is declared in the main file
// so it can be allocated to a register.
extern uint8_t * rxq_remove_ptr ;

// TXQ number of bytes.  The remove pointer is declared in the main
// file so it can be allocated to a register.  There is no add pointer;
// it is computed as needed.
extern volatile uint8_t txq_nbytes ;


// State for command state sequencer.
extern uint8_t cmd_state ;

// Received character for processing.
// Moved to register variable defined in the main file.  Cannot
// be extern.
//extern uint8_t received_char ;

// Flag indicating whether the received character is valid
// Moved to register variable defined in the main file.  Cannot
// be extern.
//extern boolean rx_char_valid ;

// Variables used in parsing numeric input
extern boolean cmd_arg_positive ;
extern uint16_t cmd_arg_int ;
extern boolean valid_digit ;

// EEW and EER command address information
#if EEPROM_ENABLE
extern uint16_t eerw_addr ;		// The address for an EER or EEW command
extern boolean eerw_internal ;	// TRUE if the EER/EEW command is for internal EEPROM
extern uint8_t eew_index ;		// Index of EEW data bytes
#endif

// Fractional portion of argument, if enabled
#if FRACTION_ARG_ENABLE
extern uint16_t cmd_arg_frac ;
extern uint8_t num_frac_digits ;
#endif

// Pointer to function to be called after numeric input is successfully
// parsed.  This function will store the numeric value in the
// appropriate location.
extern void ( * ptr_store_arg_func ) ( void ) ;

// Type of command being parsed.
extern uint8_t cmd_type ;

// Received channel number.
extern uint16_t rx_ch_num ;

// Received command execution time in millieconds.
extern uint16_t rx_time ;

// Received pulse width
extern uint8_t rx_pulse_width ;

// Array for storage of command name characters as they are
// received, and index into the array.
extern uint8_t rx_cmd_name [ MAX_CMD_NAME_LEN + 1 ] ;
extern uint8_t cmd_name_index ;

// Array of bytes for holding incoming commands.  Interpretation of
// the bytes depends on the type of command.
extern command_workspace_t cmd_wks ;

// Array for holding incoming servo commands and sequencer steps.
extern servo_command_def_t servo_cmd_wks [ 32 ] ;

// Array of edge structures.  Needs to start at address 176 and be 200
// bytes long.
extern edge_def_t edge_array [ ] ;

// Array of pulse structures, one for each of the 32 channels.
extern pulse_def_t pulse_array [ ] ;

// Array of pulse offsets, one for each of the 32 channels.
extern int8_t pulse_offset_array [ ] ;

// Array of bytes for holding incoming commands.  Interpretation of
// the bytes depends on the type of command.
extern uint8_t command_workspace [ ] ;

// Flag indicating that a byte was received.  Used to control the LED.
extern boolean rx_char_status ;

// Number of 20ms ticks required by the command just processed
extern uint16_t ms_in_command ;

/* a group-move is pending, waiting for the current move to complete */
boolean move_pending ;

// Number of 20ms ticks remaining in the current command
extern uint16_t ms_remaining_in_command ;

// Debounce flags for input pins.  Initialize to value '1'.
extern uint8_t in_A_dbnc_flags ;
extern uint8_t in_B_dbnc_flags ;
extern uint8_t in_C_dbnc_flags ;
extern uint8_t in_D_dbnc_flags ;

// Flags for the hex walking configuration command and working values
extern xconfig_t xconfig ;

// Pointer to the argument for hex config commands
extern uint16_t * xargptr ;

// State for hex sequencer
extern uint8_t hex_state ;

// Countdown timer for hex moves
extern uint16_t hex_move_timer ;

// Computed horizontal positions for right & left sides
extern hex_pulse_def_set_t hex_left_pulse_def ;
extern hex_pulse_def_set_t hex_right_pulse_def ;

// Global values for analog input channels
extern uint16_t va ;
extern uint16_t vb ;
extern uint16_t vc ;
extern uint16_t vd ;

// Number of cycles for pre-transmit delay and pacing delay
extern uint16_t tx_delay_cycles ;
extern uint16_t tx_pacing_delay_cycles ;

// Starting servo number for move commands
extern uint16_t starting_servo_num ;


// State number for debug command strings used in the main loop
#if DEBUGCMD_ENABLE
extern uint8_t debugcmd ;
#endif

// Index for servo commands being entered
extern int8_t servo_cmd_index ;

// Array of sequence players
#if GENERAL_SEQUENCER_ENABLE
extern seq_player_control_t seq_player [ NUM_SEQ_PLAYERS ] ;
extern uint8_t cmd_player_num ;		// Player number from command
extern uint8_t check_player_num ;	// Player number for periodic processing
#endif

// Global indicating the number of bytes for a sequential read from
// external EEPROM.
#if EXTERNAL_EEPROM_ENABLE
extern uint8_t eex_read_nbytes ;
#endif

// Create an array in flash containing pointers to the pulse functions in
// the OCR1A ISR.
extern void bank1_pulse(void);
extern void bank1_pulse_loop(void);
extern void bank2_pulse(void);
extern void bank2_pulse_loop(void);
extern void bank3_pulse(void);
extern void bank3_pulse_loop(void);
extern void bank4_pulse(void);
extern void bank4_pulse_loop(void);
extern void no_pulse(void);

extern flash uint16_t oc1a_isr_function_ptrs [ 9 ] ;

extern flash uint8_t pulse_data_array [ ] ;

// Software version number string
extern flash uint8_t swver [ ] ;

// Command parsing table, associates the command name strings with
// the functions to perform the commands.
//extern flash cmd_parse_table_entry_t cmd_parse_table [ NUM_CMD_PARSE_TABLE_ENTRIES ] ;
extern flash cmd_parse_table_entry_t cmd_parse_table [ ] ;
extern flash cmd_parse_table_entry_t cmd_parse_table_sentinel_end ;

// Constants for converting decimal fractions to binary fraction with
// scale factor 2^16.
extern flash uint32_t frac_mul [ ] ;

// Register definition table
extern flash register_defs_t register_groups [ ] ;

// EEPROM variables
// Reserved, @0
extern EEMEM uint8_t eeprom_reserved_0 ;
// Startup string length, @1
extern eeprom uint8_t startup_string_nbytes ;
// Startup string, @2-101
extern eeprom uint8_t startup_string [ 100 ] ;
// Reserved, @102-255
extern eeprom uint8_t eeprom_reserved_1 [ 154 ] ;
// Maximum speed array, R160-R191, @256-319
extern eeprom uint16_t reg_maxspeed [ 32 ] ;
// Maximum pulse width array, R128-R159, @320-351
extern eeprom uint16_t reg_maxpw [ 32 ] ;
// Minimum pulse width array, R96-R127, @352-383
extern eeprom uint16_t reg_minpw [ 32 ] ;
// Starting position array, R64-R95, @384-447
extern eeprom uint16_t reg_startpos [ 32 ] ;
// Pulse offset array, R32-63, @448-479
extern eeprom uint8_t reg_poffs [ 32 ] ;
// Reserved, @480-503
extern eeprom uint8_t eeprom_reserved_2 [ 24 ] ;
// Baud rate, R4, @504
extern eeprom uint8_t reg_baudrate ;
// Unit address, R3, @505
extern eeprom uint8_t reg_unitaddr ;
// TX pacing, R2, @506-507
extern eeprom uint16_t reg_tpa ;
// TX delay, R1, @508-509
extern eeprom uint16_t reg_tdl ;
// Enable, R0, @510-511
extern eeprom uint16_t reg_enable ;

#endif // _globals_h_
