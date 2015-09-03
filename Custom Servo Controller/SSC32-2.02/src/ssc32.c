//*********************************************************************
//
// 32-channel serial servo control based on the ATMega8 processor.
// Copyright 2005-2006, Mike Dvorsky.
//
//*********************************************************************
// Change Log:
//
// V1.01 - 06APR05 - MAD
//    First production version.
// V1.02 - 14APR05 - MAD
//    Fixed bug in hex sequencer that caused state change every 131
//    seconds with XS=0.
// V1.03 - 02MAY05 - MAD
//    Added PO (Pulse Offset) command.
//    Added EXTENDED_RANGE flag to support 500-2500us pulse width
//    range as a compile time option.
//*********************************************************************

#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "compiler.h"

#include "numtypes.h"
#include "appdefs.h"
#include "globals.h"
#include "cmdproc.h"
#include "serial.h"
#include "pulsedge.h"
#include "abcd.h"
#include "hexapod.h"
#include "eeprom.h"
#include "gpseq.h"

#if defined(__CODEVISIONAVR__)
//*********************************************************************
// Register global variables in CVAVR apparently must be defined in
// the top-level source file in order to be allocated properly.  They
// also need not (and must not) be declared in header files.
//*********************************************************************

// Edge data pointer used by OC1A ISR, in R3:R2
REGISTER(uint16_t, edge_data_ptr, 2);

// Temporary storage for use by the ISR, R6, R7, R8, R9, and R14.
#pragma used+
REGISTER(uint8_t, isr_tmp1, 6);
REGISTER(uint8_t, isr_tmp2, 7);
REGISTER(uint8_t, isr_tmp3, 8);
REGISTER(uint8_t, isr_tmp4, 9);
REGISTER(uint8_t, isr_tmp5, 14);
#pragma used-

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

#elif defined(__GNUC__)
/* Register global variables should be declared in every compilation unit */
#endif

#if DEBUGCMD_ENABLE
flash char debug_eeprom [] =
{
	0x00, 0x00,	// @0,1 => seq 0
	0xFF, 0xFF, // @2,3 => seq 1
	0x00, 0x00, // @4,5 => seq 2
	0x00, 0x00, // @6,7 => seq 3
	0x00, 0x00, // @8,9 => seq 4
	0x00, 0x10, // @A,B => seq 5
	0x00, 0x00, // @C,D => seq 6
	0x00, 0x00, // @E,F => seq 7
	// Start sequence 5
	// Header
		0x05,		// sequence number (5)
		0x02,		// number of servos (2)
		0x03,		// number of steps (3)
	// List of servos & speeds
       	0x04,		// servonum[0] (4)
       	0x07, 0xD0,	// speed[0] (2000)
       	0x05,		// servonum[1] (5)
       	0x09, 0xC4,	// speed[1] (2500)
	// Step 2->0 time
		0x03, 0xE8,	// time (1000)
	// Step 0 pulse widths
		0x04, 0xB0,	// pw[0] (1200)
		0x05, 0x14,	// pw[1] (1300)
	// Step 0->1 time
		0x01, 0xF4,	// time (500)
	// Step 1 pulse widths
		0x05, 0xDC,	// pw[0] (1500)
		0x05, 0xDC,	// pw[1] (1500)
	// Step 1->2 time
		0x02, 0xBC,	// time (700)
	// Step 2 pulse widths
		0x07, 0x08,	// pw[0] (1800)
		0x05, 0xDC,	// pw[1] (1500)
	// Step 2->0 time
		0x03, 0xE8,	// time (1000)

	// Final value
	0
} ;
	
//flash char debug_startup_string1 [] = "\x00\xFF" ;
//flash char debug_start_string2 [] = "#0P1500\x0D" ;
//flash char debug_start_string3 [] = "#31P2000T2000\x0D" ;
#endif

//*********************************************************************
// Initialize all peripherals
//*********************************************************************
void peripheral_init ( )
{
    // Port B initialization
    // Func7=In Func6=In Func5=Out Func4=In Func3=Out Func2=Out Func1=Out Func0=In 
    // State7=T State6=T State5=0 State4=P State3=0 State2=0 State1=0 State0=T 
    PORTB=0x10;
    DDRB=0x2E;
    
    // Port C initialization
    // Func6=In Func5=In Func4=In Func3=In Func2=In Func1=In Func0=In 
    // State6=T State5=T State4=T State3=P State2=P State1=P State0=P 
    PORTC=0x0F;
    DDRC=0x00;
    
    // Port D initialization
    // Func7=Out Func6=Out Func5=Out Func4=In Func3=In Func2=Out Func1=Out Func0=In 
    // State7=0 State6=0 State5=0 State4=P State3=P State2=1 State1=0 State0=T 
    PORTD=0x1C;
    DDRD=0xE6;
    
    // Timer/Counter 0 initialization
    // Clock source: System Clock
    // Clock value: 230.400 kHz
    // Mode: Normal top=FFh
    // OC0A output: Disconnected
    // OC0B output: Disconnected
    TCCR0A=0x00;
    TCCR0B=0x03;
    TCNT0=0x00;
    OCR0A=0x00;
    OCR0B=0x00;
        
    // Timer/Counter 1 initialization
    // Clock source: System Clock
    // Clock value: 14745.600 kHz
    // Mode: Normal top=FFFFh
    // OC1A output: Discon.
    // OC1B output: Discon.
    // Noise Canceler: Off
    // Input Capture on Falling Edge
    TCCR1A=0x00;
    TCCR1B=0x01;
    TCNT1H=0x00;
    TCNT1L=0x00;
    ICR1H=0x00;
    ICR1L=0x00;
    OCR1AH=0x00;
    OCR1AL=0x00;
    OCR1BH=0x00;
    OCR1BL=0x00;
    
    // Timer/Counter 2 initialization
    // Clock source: System Clock
    // Clock value: Timer 2 Stopped
    // Mode: Normal top=FFh
    // OC2 output: Disconnected
    ASSR=0x00;
    TCCR2A=0x00;
    TCCR2B=0x00;
    TCNT2=0x00;
    OCR2A=0x00;
    OCR2B=0x00;
    
    // External Interrupt(s) initialization
    // INT0: Off
    // INT1: Off
    MCUCR=0x00;
    
    // Timer(s)/Counter(s) Interrupt(s) initialization,
    // all disabled by default.
    TIMSK0=0x00;
    TIMSK1=0x00;
    TIMSK2=0x00;
    
    // Analog Comparator initialization
    // Analog Comparator: Off
    // Analog Comparator Input Capture by Timer/Counter 1: Off
    // Analog Comparator Output: Off
    ACSR=0x80;
    ADCSRB=0x00;
    
    // SPI initialization
    // SPI Type: Master
    // SPI Clock Rate: 2*1843.200 kHz
    // SPI Clock Phase: Cycle Half
    // SPI Clock Polarity: Low
    // SPI Data Order: LSB First
    SPCR=0x70;
    SPSR=0x01;

    // USART initialization
    // Communication Parameters: 8 Data, 1 Stop, No Parity
    // USART Receiver: On, RX interrupt enabled
    // USART Transmitter: On
    // USART Mode: Asynchronous
    // USART Baud rate: set elsewhere
    UCSR0A=0x00;
    UCSR0B=0x98;
    UCSR0C=0x8E;
    
	// ADC initialization
	// ADC Clock frequency: 115.200 kHz
	// ADC Voltage Reference: AREF pin
	// ADC High Speed Mode: Off
	ADCSRA=0x87;

#if EXTERNAL_EEPROM_ENABLE
    // 2 Wire Bus initialization
    // Generate Acknowledge Pulse: Off
    // 2 Wire Bus Slave Address: 0h
    // General Call Recognition: Off
    // Bit Rate: 388.042 kHz
    TWBR=0x0B;
    TWAR=0x00;
    TWCR=0x04;
#endif
}

#if DEBUGCMD_ENABLE
//*********************************************************************
// For debug purposes only.
//*********************************************************************
void rx_put_string ( flash_uint8_t * ch_ptr )
{
	while ( * ch_ptr != 0 )
	{
		* rxq_add_ptr = pgm_byte(*ch_ptr++);
		if ( ++rxq_add_ptr == & rx_queue [ RXQ_NBYTES ] )
		{
		    rxq_add_ptr -= RXQ_NBYTES ;
		}
	}
}
#endif

//*********************************************************************
// Read startup commands from internal EEPROM.  Startup string
// format:
// * 0x01 - 0x7F are valid characters; all others are end-of-string
// * Address 0 is unused
// * Address 1 (variable startup_string_nbytes) contains the length
//   of the startup string in bytes
// * Addresses 2 - 101 (variable startup_string) contain the
//   startup string.
//
// Several conditions must be met for the startup string to be
// processed:
//   - Bit 0 of reg_enable must be '1' (tested in the caller);
//   - startup_string_nbytes must be 0-100 inclusive;
//
// While processing the startup string, any ';' characters are
// replaced with a carriage return.  Any byte in the startup string
// that is ==0 or >0x80 will terminate the startup string.  At the
// end of the startup string an ESC character is appended to
// terminate any incomplete string.
//*********************************************************************
void process_startup_string ( void )
{
	uint8_t ss_index ;
	uint8_t data ;
	uint8_t nbytes = eep_byte(startup_string_nbytes);

	if ( nbytes <= STARTUP_STRING_MAX_BYTES )
	{
		ss_index = 0 ;
		++ rxq_add_ptr ;
		for ( ss_index = 0; ss_index < nbytes ; ++ss_index )
		{
		    data = eep_byte(startup_string[ss_index]);
		    if ( ( data == 0 ) || ( data >= 0x80 ) ) 
		    {
		        break ;
		    }
		    if ( data == ';' )
		    {
		        data = ASCII_CR ;
		    }
		    *rxq_remove_ptr = data ;
			process_commands ( ) ;
			--rxq_remove_ptr ;
		}
		
		*rxq_remove_ptr = ASCII_ESC ;
		process_commands ( ) ;
		
         // Set RX status to indicate that no characters have been
        // received, so the LED will remain lit until an actual
        // character is received.
   	    rx_char_status = RX_CHAR_INIT ;	
	}
	else
	{
	    eep_wbyte(startup_string_nbytes, 0);
	}
}

//*********************************************************************
// Initialize from EEPROM registers.
//*********************************************************************
void init_from_eeprom ( void )
{
    uint16_t enable ;
    uint8_t index ;
    
    enable = eep_word(reg_enable) ;    
    if ( enable & REG_ENABLE_GLOBAL_DISABLE )
    {
        return ;
    }
    
    // TDL and TPA.  If enabled, set to EEPROM value, else leave at
    // default value.
    if ( enable & REG_ENABLE_TXDELAY_ENABLE )
    {
        serial_set_tdl ( eep_word(reg_tdl) ) ;
        serial_set_tpa ( eep_word(reg_tpa) ) ;
    }
    
    // Pulse offsets.  If enabled, set to EEPROM value, else leave at 0.
    if ( enable & REG_ENABLE_STARTING_POFFS_ENABLE )
    {
        int8_t po ;
        for ( index = 0 ; index < 32 ; ++ index )
        {
            po = eep_byte(reg_poffs[index]);
            if ( po > 100 )
            {
                po = 100 ;
            }
            else if ( po < -100 )
            {
                po = -100 ;
            }
            pulse_offset_array [ index ] =  po ;
        }
    }

    // Starting positions.  If enabled, set to EEPROM value, else leave at 0.
    if ( enable & REG_ENABLE_STARTING_POSN_ENABLE )
    {
        uint16_t pw ;
        
        // Get address of servo command structure.    
        for ( index = 0 ; index < 32 ; ++ index )
        {
            // Determine pulse width, clipped to min/max and converted
            // to 14.7456 MHz clock cycles.  (Register value is in us.)
            pw = eep_word(reg_startpos[index]);
            if ( ( pw == 0 ) || ( pw == 65535 ) )
            {
                // Special cases, full off or full on
            }
            else if ( pw > 2500 )
            {
                // Clip to max pulse width, converted to clock cycles
                pw = (uint16_t)( ( 2500 * 14.7456) + 0.5 ) ;
            }
            else if ( pw < 500 )
            {
                // Clip to min pulse width, converted to clock cycles
                pw = (uint16_t)( ( 500 * 14.7456) + 0.5 ) ;
            }
            else
            {
                // Convert pulse width to 14.7456MHz clocks
                pw = ( pw * (uint32_t)( 65536L * 14.7456 ) + 32768 ) >> 16 ;
            }

            // Store pulse width, max speed, and servo num in command array
            servo_cmd_wks [ index ] . target_width = pw ;
            servo_cmd_wks [ index ] . servo_num = index ;
            servo_cmd_wks [ index ] . speed = 65535 ;
        }
   		convert_servo_cmd_to_pulse_arr (
  		    32,         // Number of servos
  		    0,          // Starting servo
   		    0 ) ;       // Move time
   	}
    
    // Startup string.  If enabled, call the function.
	if ( enable & REG_ENABLE_STARTUP_STRING_ENABLE )
    {
        process_startup_string ( ) ;
   	}
}

//*********************************************************************
// Process periodic events based on the TMR0 overflows, which occur
// every 1.11 ms.
// IMPORTANT NOTE:  There is no guarantee that the periodic functions
// will be called exactly every 1.11 ms.  Calls may be delayed or
// skipped entirely if the processor is busy with other things.
//*********************************************************************
void process_periodic ( void )
{
	// Read the analog input channels
	read_v_abcd ( ) ;
	
	// Test for TMR0 overflow.
	if ( TIFR0 & TOV0_MASK )
	{
		// Reset overflow flag
		TIFR0 |= TOV0_MASK ;

        // Debounce the discrete inputs
		debounce_abcd ( ) ;
	}
}

//*********************************************************************
// Main loop
//*********************************************************************
int main ( void )
{
	peripheral_init ( ) ;
	set_baud(0) ;
	pulse_and_edge_init ( ) ;
 
	// Initialize RX and TX queue pointers.  The rxq starts at 0x80
	// and the txq starts at 0x100, so the low bytes of the pointers
	// are set to 0x80 and 0x00.
	rxq_add_ptr = rx_queue ;
	rxq_remove_ptr = rx_queue ;
	txq_nbytes = 0 ;
	txq_remove_ptr = tx_queue ;	//(uint8_t)tx_queue ;
	
#if 0
	while (1) {
	    tx_char('#');
	}
#endif

	init_from_eeprom ( ) ;

    // Global enable interrupts
	sei();
	
	// Verify command array length
	if (pgm_byte(
		((uint8_t *)pgm_word(
		    cmd_parse_table[NUM_CMD_PARSE_TABLE_ENTRIES-1].ptr_cmdstr))[0])  != 0xFF )
	{
	    tx_char ( '!' ) ;
	}

	while (1)
	{
	    // This point should be reached just after the Bank4 edges are output.
		
		// Do debug stuff if enabled
#if DEBUGCMD_ENABLE
//				rx_put_string ( "#0P1500#1P1500#2P1500#3P1500#4P1500#5P1500#6P1500#7P1500 \x0D" ) ;
    	switch ( debugcmd )
    	{
    		case 1:
 				rx_put_string ( "#0P1500 \x0D" ) ;
     			break ;
    		case 2:
 				rx_put_string ( "#0PO-100 \x0D" ) ;
    			break ;
    		case 3:
 				rx_put_string ( "#0P1501T60 \x0D" ) ;
    			break ;
    		case 4:
    			break ;
    		case 5:
    			break ;
    		case 6:
    			break ;
    		case 7:
    			break ;
    		case 8:
    			break ;
    		case 9:
    			break ;
    		case 10:
 				rx_put_string ( "#0PO100 \x0D" ) ;
    			break ;
    		case 11:
    			break ;
   		case 12:
    			break ;

#if 0
 				rx_put_string ( "SSDEL 255 \x0D" ) ;
 				rx_put_string ( "SSCAT #0P2000T1000;\x0D" ) ;
 				rx_put_string ( "R0=0\x0D" ) ;
 				rx_put_string ( "SS \x0D" ) ;
				rx_put_string ( "SSCAT #0P2000T5000;\x0D" ) ;
 				rx_put_string ( "SS \x0D" ) ;
 				rx_put_string ( "SSCAT xxxx\x0D" ) ;
 				rx_put_string ( "SS \x0D" ) ;
 				rx_put_string ( "SSDEL 4 \x0D" ) ;
 				rx_put_string ( "SS \x0D" ) ;
 				rx_put_string ( "SSDEL 6 \x0D" ) ;
 				rx_put_string ( "SS \x0D" ) ;
 				rx_put_string ( "SSCAT #1P1000T4000;PL0SQ5;\x0D" ) ;
 				rx_put_string ( "SS \x0D" ) ;
 				rx_put_string ( "SSDEL 0 \x0D" ) ;
 				rx_put_string ( "SS \x0D" ) ;
 				rx_put_string ( "SSCAT \x0D" ) ;
 				rx_put_string ( "SS \x0D" ) ;
 				rx_put_string ( "SSDEL 255 \x1B" ) ;    //ESC out
 				rx_put_string ( "SS \x0D" ) ;
 				rx_put_string ( "SSCAT abc \x1B" ) ;    //ESC out
 				rx_put_string ( "SS \x0D" ) ;
 				rx_put_string ( "SSCAT 0123456789 \x0D" ) ;    //Big string
 				rx_put_string ( "SSCAT 0123456789 \x0D" ) ;    //Big string
 				rx_put_string ( "SSCAT 0123456789 \x0D" ) ;    //Big string
 				rx_put_string ( "SSCAT 0123456789 \x0D" ) ;    //Big string
 				rx_put_string ( "SS \x0D" ) ;
 				rx_put_string ( "R64=0 \x0D" ) ;
 				rx_put_string ( "R65=65535 \x0D" ) ;
 				rx_put_string ( "#0P1500 \x0D" ) ;
 				rx_put_string ( "R0=12345 \x0D" ) ;
 				rx_put_string ( "R1=700 \x0D" ) ;
 				rx_put_string ( "R2=800 \x0D" ) ;
 				rx_put_string ( "R3=12 \x0D" ) ;
 				rx_put_string ( "R4=2 \x0D" ) ;
 				rx_put_string ( "RDFLT \x0D" ) ;
 				rx_put_string ( "R0 \x0D" ) ;
 				rx_put_string ( "R1 \x0D" ) ;
 				rx_put_string ( "R2 \x0D" ) ;
 				rx_put_string ( "R3 \x0D" ) ;
 				rx_put_string ( "R4 \x0D" ) ;
 				rx_put_string ( "#0P1501T60 \x0D" ) ;
 				rx_put_string ( "R32=75 \x0D" ) ;
 				rx_put_string ( "R63=-75 \x0D" ) ;
 				rx_put_string ( "R64=1600 \x0D" ) ;
 				rx_put_string ( "R95=1400 \x0D" ) ;
 				rx_put_string ( "R96=800 \x0D" ) ;
 				rx_put_string ( "R127=1000 \x0D" ) ;
 				rx_put_string ( "R32 \x0D" ) ;
 				rx_put_string ( "R63 \x0D" ) ;
 				rx_put_string ( "R64 \x0D" ) ;
 				rx_put_string ( "R95 \x0D" ) ;
 				rx_put_string ( "R96 \x0D" ) ;
 				rx_put_string ( "R127 \x0D" ) ;
 				rx_put_string ( "#0PO100 \x0D" ) ;
 				rx_put_string ( "R128=2100 \x0D" ) ;
 				rx_put_string ( "R159=2000 \x0D" ) ;
 				rx_put_string ( "R160=3000 \x0D" ) ;
 				rx_put_string ( "R191=10000 \x0D" ) ;
 				rx_put_string ( "R128 \x0D" ) ;
 				rx_put_string ( "R159 \x0D" ) ;
 				rx_put_string ( "R160 \x0D" ) ;
 				rx_put_string ( "R191 \x0D" ) ;
#endif
    	}
    	++ debugcmd ;
#endif

		// Update the pulse array
   		update_pulse_array ( ) ;

		// Subtract 20ms from the number of ms remaining in the
		// latest command.
		if ( ms_remaining_in_command > 20 )
		{
		  ms_remaining_in_command -= 20 ;
		}
		else
		{
		  ms_remaining_in_command = 0 ;
		  /* Could use this for asynchronous group-move
		   * commands, which would be a very nice feature.
		   */
		  if (move_pending) {
		    convert_servo_cmd_to_pulse_arr(servo_cmd_index + 1,
						   0,
						   rx_time);
		    ms_remaining_in_command = ms_in_command ;
		    move_pending = FALSE;
		    tx_char ( '.' ) ; /* next */
		  }
		}

		process_periodic ( ) ;
	
		// If there is time, do Bank edges
		if ( get_edge_data_pointer () <= (uint16_t)& edge_array [ 38 ] )
		{
			copy_pulses_to_sorted_edges ( 0, 0 ) ;
			set_edge_data_and_function_ptr ( 0, 0 ) ;
			process_periodic ( ) ;
			copy_pulses_to_sorted_edges ( 8, 9 ) ;
			set_edge_data_and_function_ptr ( 9, 2 ) ;
			process_periodic ( ) ;
			copy_pulses_to_sorted_edges ( 16, 18 ) ;
			set_edge_data_and_function_ptr ( 18, 4 ) ;
			process_periodic ( ) ;
			copy_pulses_to_sorted_edges ( 24, 27 ) ;
			set_edge_data_and_function_ptr ( 27, 6 ) ;
		}
		
		process_periodic ( ) ;
		process_commands ( ) ;

		// Execute the hexapod sequencer if enabled
#if HEXAPOD_SEQUENCER_ENABLE
		hex_sequencer ( ) ;
#endif

		// Turn the LED on for a 20ms loop if at least one character
		// has been received.  Turn on LED continuously after init
		// and before first character is received.
		switch ( rx_char_status )
		{
			case RX_CHAR_RECEIVED :
				rx_char_status = RX_CHAR_NO_CHAR ;
			case RX_CHAR_INIT :
				LED_BIT = LED_ON ;
				break ;
			default :
				LED_BIT = LED_OFF ;
		}

#if GENERAL_SEQUENCER_ENABLE
		// Start with player 0
		check_player_num = 0 ;
#endif
		
		// Wait for 20ms interval to complete
		do
		{
			process_commands ( ) ;
#if GENERAL_SEQUENCER_ENABLE
			process_gpseq ( ) ;
#endif
			process_periodic ( ) ;
		}
		while ( get_edge_data_pointer () > (uint16_t)&edge_array[35] ) ;

		do
		{
			process_commands ( ) ;
#if GENERAL_SEQUENCER_ENABLE
			process_gpseq ( ) ;
#endif
			process_periodic ( ) ;
		}
		while ( get_edge_data_pointer () <= (uint16_t)&edge_array[35]) ;
	}
}
