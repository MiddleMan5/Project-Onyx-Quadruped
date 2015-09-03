/* Some defintions to aid using different compilers.
 *
 * In short: gcc does not have a "bit" data-type, and places constant
 * strings in SRAM by default.
 */
#ifndef _SSC32_COMPILER_H_
#define _SSC32_COMPILER_H_
                               
#include "numtypes.h"

#if defined(__GNUC__)

# include <avr/io.h>
# include <avr/interrupt.h>
# include <avr/pgmspace.h>
# include <avr/eeprom.h>

# define F_CPU 14.7456E6
# include <util/delay.h>

# define flash  PROGMEM
# define eeprom EEMEM
# define REGISTER(_type, _name, reg) register _type _name asm("r"#reg)

# define delay_ms(ms) _delay_ms(ms)

/* The next 5 lines are black magic. However, the ssc32 firmware was
 * written for empty interrupt prologues/epilogues.
 */
# undef __INTR_ATTRS
# if __GNUC__ >= 4 && __GNUC_MINOR__ >= 1
#  define __INTR_ATTRS naked, used, externally_visible
# else /* GCC < 4.1 */
#  define __INTR_ATTRS naked, used
# endif

typedef union byte_port {
    uint8_t data;
    struct {
	uint8_t Bit0:1;
	uint8_t Bit1:1;
	uint8_t Bit2:1;
	uint8_t Bit3:1;
	uint8_t Bit4:1;
	uint8_t Bit5:1;
	uint8_t Bit6:1;
	uint8_t Bit7:1;
    } bits;
} BYTE_PORT;

# define _PORT_BIT(port, nr) ((volatile BYTE_PORT *)&port)->bits.Bit##nr
# define PORT_BIT(port, nr)  _PORT_BIT(port, nr)

# define strcmpf(a, b) strcmp_P(a, b)

/* GCC is a little bit low-featured when it comes to access to program
 * space. Sigh.
 */
# define pgm_byte(var) pgm_read_byte(&(var))
# define pgm_word(var) pgm_read_word(&(var))
#if 0
# define eep_byte(var)  eeprom_read_byte(&(var))
# define eep_word(var)  eeprom_read_word(&(var))
# define eep_wbyte(var, val) eeprom_write_byte(&(var), (val))
# define eep_wword(var, val) eeprom_write_word(&(var), (val))
#else
# define eep_byte(var)  eei_get_byte((uint16_t)&(var))
# define eep_word(var)  eei_get_word((uint16_t)&(var))
# define eep_wbyte(var, val) eei_put_byte((uint16_t)&(var), (val))
# define eep_wword(var, val) eei_put_word((uint16_t)&(var), (val))
#endif

typedef flash uint8_t flash_uint8_t; 

/* Convert micro-seconds to cycles, the result is a 32bit integer
 * which can then be truncated to 16bits. The problem here is that int
 * is 16bits. If an implicit conversion between a float and an integer
 * does not fit into an integer, then the result is undefined (I
 * thing, C standard?). gcc then asserts UINT_MAX. CodeVision just
 * internally converts to 32 bits and then truncates.
 */
# define US_TO_CYCLES(cnt_us) \
  ((uint32_t)(((double)(cnt_us)) * (F_CPU*1e-6) + 0.5))

#elif defined(__CODEVISIONAVR__)

# include <mega168.h>
# include <spi.h>
# include <delay.h>

# define REGISTER(type, name, reg) register type name @reg
# define sei() #asm("sei")
# define cli() #asm("cli")

# define _PORT_BIT(port, nr) port . nr
# define PORT_BIT(port, nr) _PORT_BIT(port, nr)
# define PSTR(str) str

# define pgm_byte(var)  (var)
# define pgm_word(var)  (var)
# define eep_byte(var)  (var)
# define eep_word(var)  (var)
# define eep_wbyte(var, val)  (var) = (val)
# define eep_wword(var, val)  (var) = (val)
                
# define flash_uint8_t uint8_t flash

# define US_TO_CYCLES(cnt_us) ((cnt_us) * (F_CPU*1e-6) + 0.5)

#endif

#endif /* _SSC32_COMPILER_H_ */
