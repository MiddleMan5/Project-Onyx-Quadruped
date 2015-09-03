;****************************************************************************
;* Simple page-oriented boot loader for AVR parts with boot block.
;* (c) 2005 Mike Dvorsky
;*
;* Target:  Any AVR with boot loader capability.
;*
;* Function:  Accepts serial commands to erase, write, or read FLASH memory,
;* or to transfer control to a specified address.
;* 
;* Supported commands:
;* 1. Erase page at address AAAA
;*		command: "eAAAA\n"
;*		response: "."
;* 2. Read page at address AAAA; response is 1 page (32 words) of data
;*		command: "rAAAA\n"
;*		response: "DDDD...DDDD."
;* 3. Write page at address AAAA with 1 page of data DDDD...DDDD
;*		command: "wAAAA\nDDDD...DDDD\n"  (\n = carriage return)
;*		response: "."
;* 4. Transfer control to address AAAA.
;*		command:  "gAAAA\n"
;*		response:  none
;*
;* "AAAA" = 2-byte address in hex
;* "DDDD...DDDD" = page of data in hex = 32 words = 64 bytes = 128 hex digits
;* "\n" = carriage return = ASCII 0x0D
;*
;* Received characters are not echoed.  Any line feeds (ASCII 0x0A) are
;* ignored.  The command bytes MUST be lower case.  The hex address and
;* data characters MUST be upper case.  Any incorrect character will
;* cause the command to be terminated.
;*
;* Communication parameters = 8 data bits, 1 stop bit, no parity.  Baud
;* rate is platform dependent, but will be 115.2 kBaud if that rate is
;* supported by the platform.
;*
;* At any time, the following sequence will terminate any partially
;* entered command and restore the boot loader to the state where it is
;* waiting for a new command.  The sequence is ".\n" (a period character
;* followed by a carriage return).
;*
;* The order of data bytes for a page read or page write is LSB first.
;*
;* Following a page erase or page write command, the processor will be
;* unavailable for several milliseconds while the FLASH is being
;* modified.  The device sending the data must account for this in one
;* of the following ways:
;* 1.  Wait for receipt of the '.' character indicating the erase/write
;*     command is complete.  At this time the processor will be ready
;*     to receive and process another command.
;* 2.  Delay at least 5 milliseconds following the erase/write command
;*     before starting the next command.
;* 3.  Send enough "fill" bytes to ensure at least a 5 millisecond delay.
;*     The fill byte must not be a valid command byte.  Examples of valid
;*     fill bytes are carriage return, line feed, space, '.', etc.  The
;*     fill bytes will be ignored, and should be terminated with a
;*     carriage return.  60 fill bytes will be sufficient to ensure the
;*     required delay at all Baud rates up to and including 115.2 kBaud.
;*
;* There are no restrictions on which pages can be read or written, but
;* in order to prevent accidental corruption of the boot section, it is
;* recommended that the boot lock bits be set to disallow SPM to write
;* to the boot loader section.
;*
;* In addition to the serial commands described above, the boot loader
;* makes two services available to the application:
;* 1.  Jump to boot.  If the application jumps to the last location in 
;*     FLASH, the boot loader will be started  This allows the flash
;*     process to be started without the need to install a jumper.
;* 2.  Modify FLASH memory.  If the application calls the next
;*     to last location in FLASH, the boot loader will spin until any
;*     previous SPM instruction completes, load the value in R16 into
;*     SPMCR, and then execute an SPM instruction.  (The SPM instruction
;*     can only be used in the boot section, so this is the only way for
;*     the application to modify FLASH memory.)
;*
;*     When the SPM service is called, interrupts must be disabled and
;*     there must be no EEPROM write in progress.  It is the application's
;*     responsibility to ensure this.  Also, if the function being
;*     performed is a write to the page buffer, then R0, R1, and Z must
;*     be set to the appropriate values.
;*
;* Normally, after reset, the boot loader will branch to the reset vector
;* in the application section (unless it finds the reset vector erased).
;* There is a method to force the boot loader to remain in boot, which
;* varies with different platforms.  Usually there will be one or more I/O
;* pins that can be jumpered or driven in a special way that indicates to
;* the boot loader that it should remain in boot and not jump to the
;* application.  This is useful for recovering from a defective application.
;****************************************************************************

.include "m8def.inc"			;* Processor-specific defines

;****************************************************************************
;* Global constants
;****************************************************************************

.equ	BAUD_DIVISOR = 7		;* 14.7456 MHz, 115.2k
;.equ	BAUD_DIVISOR = 12		;* 4 MHz, 19.2k, for testing on STK200

.equ	ASCII_LF = 10			;* ASCII carriage return
.equ	ASCII_CR = 13			;* ASCII line feed

.equ	PAGESIZEW = PAGESIZE
.equ	PAGESIZEB = PAGESIZE*2

;****************************************************************************
;* Global register definitions
;****************************************************************************

.def	zeroreg			= R2
.def	imm1			= R16
.def	imm2			= R17
.def	imm3			= R18
.def	counter			= R19
.def	command			= R20

;****************************************************************************
;* Start of code.  Interrupts will be disabled for all boot loader
;* operations, so no interrupt vector table is needed.  The code starts at
;* the reset vector address.
;****************************************************************************
.cseg
.org	SMALLBOOTSTART

	;* Init a register with the value 0
	clr		zeroreg

;****************************************************************************
;* Determine whether the boot loader should remain in boot or jump to the
;* application.  There are two parts to this test:
;* 1.  Check the first word of the application section (the reset
;*     vector).  If it is erased (i.e. 0xFFFF), assume there is not a
;*     valid application and remain in boot.  Otherwise continue with the
;*     next test.
;* 2.  Test the I/O pins for the presence of a jumper or other signal that
;*     the code should remain in the boot loader.  This is useful for
;*     recovery from a run-away application.  The specific I/O pins and the
;*     test performed are platform specific.
;*
;* Also, before jumping to the application, it is necessary to set the vector
;* table to the application section.  This can be done at any time, since the
;* boot loader does not use interrupts.
;*
;* The operations of setting the vector table and testing the first word of
;* the application are interspersed with the I/O pin test.  They are used
;* to introduce delays after the pins are set up, in order to allow the 
;* signals time to settle.
;****************************************************************************
;* For the SSC-32 platform, pins PD3 and PD4 are normally left open or
;* jumpered to GND in order to select one of 4 Baud rates.  If PD3 and PD4
;* are jumpered together, this indicates to the boot loader that it should
;* remain in boot and not automatically jump to the application.
;****************************************************************************
	;* Test Baud Select pins (PD3, PD4); if jumpered together stay in boot
	;* loader.
	
	;* First, set PD4 for input w/o pullup, and PD3 for input w/pullup.
	out		DDRD, zeroreg		;* PORTD = input
	ldi		imm1, 0x08			;* Pullup on PD3
	out		PORTD, imm1

	;*************
	;* The next few instructions are placed here rather than separately,
	;* in order to allow them to be used for double-duty as delay instructions
	;* to allow the input value to settle before the pin is read.
	;*
	;* Set vector table to app section
	ldi		imm1, (1<<IVCE)
	out		GICR, imm1
	out		GICR, zeroreg
	;* Read the first word of the application section (the reset
	;* vector).  If it is erased (i.e. 0xFFFF), stay in boot.
	ldi		ZL, 0
	ldi		ZH, 0
	lpm		R28, Z+
	lpm		R29, Z
	adiw	R28, 1
	breq	bootmain
	;*************

	;* Now the input value should have settled. Check PD3.  If the
	;* PD3 value is 0, then the "stay in boot" jumper is not
	;* installed, so jump to the app.  If the PD3 value is 1, then
	;* we need to continue testing to determine whether the jumper
	;* is installed.
	sbis	PIND, PD3
	rjmp	$000				;* Jump to app reset vector

	;* Next, set PD4 for output with value 0.  Check PD3.  If the PD3
	;* value is 0, then the "stay in boot" jumper is installed, so fall
	;* through to boot.  If the PD3 value is 1, then the "stay in boot"
	;* jumper is not installed, so jump to the app.
	sbi		DDRD, DDD4			;* PD4 = output
	lpm		imm1, Z				;* 3 cycle delay for input to settle
	sbic	PIND, PD3
	rjmp	$000				;* Go to the app reset vector

;****************************************************************************
;* Bootloader main code.
;****************************************************************************
bootmain:
	;* Clear zeroreg again in case of jump to boot from app
	clr		zeroreg

	;* Initialize the UART to 1 stop bit, no parity, interrupts
	;* disabled, and the baud rate set by BAUD_DIVISOR.
	out		UBRRH, zeroreg
	ldi		imm1, low(BAUD_DIVISOR)
	out		UBRRL, imm1
	out		UCSRA, zeroreg
	ldi		imm1, (1<<TXEN) | (1<<RXEN)
	out		UCSRB, imm1	
	ldi		imm1, (1<<URSEL) | (1<<UCSZ1) | (1<<UCSZ0)
	out		UCSRC, imm1

	;* Branch back to this point if an invalid command is received.
wait_for_command_byte:
	;* Set stack pointer to a known location in RAM.
	;* NOTE:  prior to this point, the stack pointer is invalid, so no
	;* instructions may be used that depend on the stack (i.e. rcall,
	;* ret, push, pop).
	;* NOTE:  the stack pointer is re-initialized at the end of every
	;* command.  This allows subroutines to "bail out" in case of error,
	;* without needing to worry about fixing the stack.
	out		SPH, zeroreg
	ldi		imm1, 0xFF
	out		SPL, imm1

	;* Receive and process a command byte.  Ignore any unrecognized bytes.
command_loop:
	rcall	rx_byte			;* Get command
	cpi		imm1, ASCII_CR	;* If carriage return, discard
	breq	wait_for_command_byte
	mov		command, imm1
	rcall	get_addr		;* Get address for command
	;* Check for Go command
	cpi		command, 'g'
	breq	go_command
	;* Check for Read Page command
	cpi		command, 'r'
	breq	read_page
	;* Check for Write Page command
	cpi		command, 'w'
	breq	write_page
	;* Check for Erase Page command
	cpi		command, 'e'
	brne	command_loop

;****************************************************************************
;* Process the Erase command.
;****************************************************************************
erase_page:
	;* Page erase.  Address is in Z.
	ldi		imm1, ( 1 << PGERS ) | ( 1 << SPMEN )
	rcall	do_spm

	;* Enable the RWW section.  Needed following a page erase or page write.
	;* The call to do_spm will not return until the read or write is
	;* complete, so the '.' character will not be transmitted until the
	;* read/write is complete.
enable_rww:
	ldi		imm1, ( 1 << RWWSRE ) | ( 1 << SPMEN )
	rcall	do_spm

	;* Send a '.' character to the UART to indicate previous command (if
	;* any) is complete.  Branch back to this point if a valid command
	;* has been received and processed.
tx_command_complete:
	ldi		imm1, '.'
	rcall	tx_byte
	rjmp	wait_for_command_byte

;****************************************************************************
;* Process the Read command.
;****************************************************************************
read_page:
	;* Interate through the page, sending bytes. Starting address is in Z.
	ldi		counter, PAGESIZEB
read_page_loop:
	lpm		imm1, Z+
	rcall	tx_hex_2
	dec		counter
	brne	read_page_loop

	;* Transmit "command complete" byte and Resume command processing
	rjmp	tx_command_complete

;****************************************************************************
;* Process the Write command.
;****************************************************************************
write_page:
	;* Read and store 1 page worth of data.  Starting address is in Z.
	ldi		counter, PAGESIZEW
write_page_loop:
	rcall	rx_hex_4		;* Data into imm2:imm1
	mov		R0, imm2
	mov		R1, imm1
	ldi		imm1, ( 1 << SPMEN )
	rcall	do_spm
	adiw	ZL, 2
	dec		counter
	brne	write_page_loop

	;* Wait for carriage return, bail out if any other character received
	rcall	wait_cr

	;* Complete page of data received without error; write page
	subi	ZL, low(PAGESIZEB)
	sbci	ZH, high(PAGESIZEB) 
	ldi		imm1, ( 1 << PGWRT ) | ( 1 << SPMEN )
	rcall	do_spm

	;* Transmit "command complete" byte and Resume command processing
	rjmp	enable_rww

;****************************************************************************
;* Wait for a carriage return.  If any other character is received, jump
;* to the error routine.
;****************************************************************************
wait_cr:
	rcall	rx_byte
	cpi		imm1, ASCII_CR
	brne	wait_for_command_byte
	ret

;****************************************************************************
;* Read a hex digit from the UART.  If the value is '0' - '9' or 'A' - 'F',
;* convert to an integer 0-15 in register 'imm1'.  Otherwise jump to the
;* error routine.
;****************************************************************************
rx_hex_1:
	rcall	rx_byte

	;* Test for '0' - '9'
	subi	imm1, '0'
	brcs	wait_for_command_byte
	cpi		imm1, 10
	brcs	rx_hex_1_end

	;* Byte was greater than '9', test for 'A' - 'F'
	subi	imm1, ( 'A' - '0' - 10 )
	brcs	wait_for_command_byte
	cpi		imm1, 16
	brcc	wait_for_command_byte

rx_hex_1_end:
	ret

;****************************************************************************
;* Process the Go command.  Jump to the address specified (in Z).
;****************************************************************************
go_command:
	ijmp

;****************************************************************************
;* Read a 2-byte (4 hex digits) address from the UART and store it in Z.
;* Wait for carriage return after address is received.
;****************************************************************************
get_addr:
	rcall	rx_hex_4
	movw	ZL, imm1		;* Store address in Z
	rjmp	wait_cr			;* Wait for carriage return


;****************************************************************************
;* Read 4 hex digits from the UART.  Return the value in 'imm2:imm1'.
;****************************************************************************
rx_hex_4:
	rcall	rx_hex_2
	mov		imm2, imm1
	; Fall through to rx_hex_2

;****************************************************************************
;* Read 2 hex digits from the UART.  Return the value in 'imm1'.
;****************************************************************************
rx_hex_2:
	rcall	rx_hex_1
	swap	imm1
	mov		imm3, imm1
	rcall	rx_hex_1
	or		imm1, imm3
	ret

;****************************************************************************
;* Read a byte from the UART.  Return the byte in register 'imm1'.
;* This function will spin until a byte has been received by the UART.
;****************************************************************************
rx_byte:
	sbis	UCSRA, RXC
	rjmp	rx_byte
	in		imm1, UDR
	cpi		imm1, ASCII_LF	;* Discard line feeds
	breq	rx_byte
	ret

;****************************************************************************
;* Send 2 hex digits to the UART.  The byte to be sent must be in register
;* 'imm1'.
;****************************************************************************
tx_hex_2:
	mov		imm2, imm1
	swap	imm1			;* Send high order nibble first
	rcall	tx_hex_1
	mov		imm1, imm2		;* Send low order nibble next
	; Fall through to tx_hex_1

;****************************************************************************
;* Send a hex digit to the UART.  The digit to be sent will be the low
;* nibble of register 'imm1'.  The high nibble will be ignored.
;****************************************************************************
tx_hex_1:
	andi	imm1, 0x0F
	subi	imm1, ( - '0' )				;* Convert 0-9 to ASCII
	cpi		imm1, ( '0' + 10 )			;* Was it >9 ?
	brcs	tx_byte
	subi	imm1, ( '0' - 'A' + 10 )	;* Convert A-F to ASCII
	; Fall through to tx_byte

;****************************************************************************
;* Send a byte to the UART.  The byte to be sent must be in register 'imm1'.
;* This function will spin until the UART is ready to transmit.
;****************************************************************************
tx_byte:
	sbis	UCSRA, UDRE
	rjmp	tx_byte
	out		UDR, imm1
	ret

;****************************************************************************
;* Issue the SPM instruction.  The SPM command register value must be in
;* 'imm1'.  Interrupts must be disabled, and no EEPROM write may be in
;* process.  For a write to the page buffer, R0, R1, and Z must be set up
;* prior to calling do_spm.
;****************************************************************************
do_spm:
	;* Wait for previous SPM to complete
	in		imm2, SPMCR
	sbrc	imm2, SPMEN
	rjmp	do_spm

	;* Write the SPMCR value from 'imm1' and issue the SPM instruction
	out		SPMCR, imm1
	spm
	
	ret

;****************************************************************************
;* Application services table.  This is a jump table for services that
;* are made available to the application.  The entry points for these
;* services are fixed addresses at the end of FLASH memory.
;****************************************************************************

;****************************************************************************
;* Write to program memory.  This function will wait for any previous SPM
;* instruction to complete, then load SPMCR with the value in R16 and
;* issue the SPM instruction.  R0, R1, and Z must be set up appropriately
;* for the function being performed.
;* Note:  no EEPROM access may in in process, and interrupts must be disabled
;****************************************************************************
.org	FLASHEND-1
appsrv_spm:
	rjmp	do_spm


;****************************************************************************
;* Start the boot loader.
;****************************************************************************
.org	FLASHEND
appsrv_go_boot:
	rjmp	bootmain
