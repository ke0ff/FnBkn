/*************************************************************************
 *********** COPYRIGHT (c) 2012 by Joseph Haas (DBA FF Systems)  *********
 *
 *  File name: putcw.c
 *
 *  Module:    Peripheral
 *
 *  Summary:   This is a text to CW output driver.  It uses cw_key() to activate the annunciator
 *				resource (an MCU peripheral or I/O pin).
 *				puts_cw() processes a string (passed as a pointer) and closely resembles the standard
 *					puts() prototype with the addition of a "wpm" parameter.
 *				putchar_cw() processes a characer for morse translation and closely resembles the
 *					standard putchar() prototype with the addition of the "wpm" parameter.
 *
 *				The cw_key() and cw_delay() functions are application dependent and must be customized
 *					to produce an appropriate output (cw_key) with Morse timings (cw_delay)
 *
 *  Project scope declarations revision history:
 *    03-24-24 jmh:  Rev 0.0:
 *                   Copied from UX89-MR project for F'n Beacon project
 *					 Modified putcw() with process hooks for beacon control characters (which are non-CW characters, e.g., lower case ASCII)
 *					 Added wpmg() to manage Morse speed changes
 *					 added side-tone timer-tone for PB4 (freq fixed by a #define in main.h)
 *					 !!!-todo: cw_delay() modified to use a timer resource
 *
 *    Previous history:
 *    01-01-13 jmh:  Rev 1.0:
 *                   Initial release
 *    12-31-12 jmh:  Rev 1.0:
 *                   Initial project code
 *
 *******************************************************************/


/********************************************************************
 *  File scope declarations revision history:
 *    12-31-12 jmh:  creation date
 *
 *******************************************************************/


//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
// compile defines

#include <avr/io.h>
#include "putcw.h"
#include "main.h"

//-----------------------------------------------------------------------------
// Definitions
//-----------------------------------------------------------------------------

#define ON	1
#define OFF 0
#define DIT 1		// These define the basic relationships for the CW elements
					// DIT defines the dit length and also the inter-element spacing
#define DAH 3		// DAH defines the DAH length
#define DAH3 9		// DAH3 defines the inter-word spacing
#define LSPC 3		// LSPC defines the inter-letter spacing
					// DIT should always be 1, and DAH should always be 3.
					// DAH3 and LSPC may be adjusted to change the character weighting
					// Speed calibration should be done with the values as shown.
					// A simple calibration run can be performed by sending:
					// "PARIS PARIS PARIS E"
					// The time from the start of the first element in the first "P" to
					// the start of the "E" at the end of the string measures 3 words.
					// Thus, WPM = 3/t  Where t is measured in minutes.
					// See the cw_delay() fn for more calibration info.

//-----------------------------------------------------------------------------
// Local variables
//-----------------------------------------------------------------------------

// Morse code translation tables for the ASCII character set.
// The tables represent a segment of the ASCII code starting at the "!" character (FIRST_CW).
//	To save space, the underscore "_" character is re-mapped to follow the "Z" character.
// Morse elements table.  Characters are encoded as bits in an 8-bit unsigned array.  The LSb of the
//	byte is the first element of the character.  A "1" is a DIT, and a "0" is a DAH.
char	cw_table[] = 	{
//    !     "     #     $     %     &     '     (     )     *     +     ,     -     .     /     0
   0x0e, 0x2d, 0x00, 0x37, 0x00, 0x07, 0x21, 0x12, 0x12, 0x00, 0x15, 0x0c, 0x1e, 0x15, 0x16, 0x00,
//    1     2     3     4     5     6     7     8     9     :     ;     <     =     >     ?     @
   0x01, 0x03, 0x07, 0x0f, 0x1f, 0x1e, 0x1c, 0x18, 0x10, 0x38, 0x2a, 0x00, 0x0e, 0x00, 0x33, 0x29,
//    a     b     c     d     e     f     g     h     i     j     k     l     m     n     o     p
   0x01, 0x0e, 0x0a, 0x06, 0x01, 0x0b, 0x04, 0x0f, 0x03, 0x01, 0x02, 0x0d, 0x00, 0x02, 0x00, 0x09,
//    q     r     s     t     u     v     w     x     y     z     _   nul
   0x04, 0x05, 0x07, 0x00, 0x03, 0x07, 0x01, 0x06, 0x02, 0x0c, 0x13, 0x00
						};
// Morse character length table.  Indicates length of corresponding character.  A length of "0"
//	represents an un-supported character.
char	cwlen_table[] = 	{
//    !     "     #     $     %     &     '     (     )     *     +     ,     -     .     /     0
   0x06, 0x06, 0x00, 0x07, 0x00, 0x03, 0x06, 0x05, 0x06, 0x00, 0x05, 0x06, 0x06, 0x06, 0x05, 0x05,
//    1     2     3     4     5     6     7     8     9     :     ;     <     =     >     ?     @
   0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x06, 0x06, 0x00, 0x05, 0x00, 0x06, 0x06,
//    a     b     c     d     e     f     g     h     i     j     k     l     m     n     o     p
   0x02, 0x04, 0x04, 0x03, 0x01, 0x04, 0x03, 0x04, 0x02, 0x04, 0x03, 0x04, 0x02, 0x02, 0x03, 0x04,
//    q     r     s     t     u     v     w     x     y     z     _   nul
   0x04, 0x03, 0x03, 0x01, 0x03, 0x04, 0x03, 0x04, 0x04, 0x04, 0x06, 0x00
						};

char	wpm_reg;					// global WPM register

#define FIRST_CW '!'
#define USCORE_CW  ('Z'+1)
#define LAST_CW  ('Z'+2)

//-----------------------------------------------------------------------------
// Local Prototypes
//-----------------------------------------------------------------------------

void cw_key(char key   /* transmit on or off */ );
void cw_delay(char element_len, char wpm);

//-----------------------------------------------------------------------------
// Processor I/O assignments
//-----------------------------------------------------------------------------

//signal output port (application dependent)
//sbit cwkey = P3^3;							// cw output bit

// ********************************************
//  ************ PUTCW PUBLIC FNS ************
// ********************************************

//-----------------------------------------------------------------------------
//	puts_cw() outputs CW translation of input string (ASCII)
//	generally conforms to the puts() prototype with the addition of the wpm param 
//-----------------------------------------------------------------------------
int puts_cw(const char* s /* pointer to string to transmit */, char wpm)
{
	char	c;	//temps
	char	i;
	char	j;
	char	w=wpm;

	// send each character from string to CW character translator Fn
	while(*s) {
		c = *s++;
		if(c > LAST_CW){
			switch(c){
			default:
				break;

			case 'b':					// key-down with 1-16 sec of delay, 1-digit decimal param
				cw_key(1);
			case 'a':					// no-key-down with 1-16 sec of delay
				i = *s;
				if(i) s += 1;			// advance pointer if not the end of string
				i &= 0x0f;				// mask low nyb -- this is the #seconds to delay
				cw_delay(i+1, 0);		// delay for i seconds (i = 1-10 for a param of "0" to "9"
				cw_key(0);				// de-activate key
				break;

			case 'z':					// set CW speed, with 2-digit, decimal param
				i = *s;
				if(i) s += 1;			// advance pointer if not the end of string
				j = i & 0x0f;			// mask low nyb -- this is the #seconds to delay
				j *= 10;
				i = *s;
				if(i) s += 1;			// advance pointer if not the end of string				
				j = j + (i & 0x0f);		// mask low nyb -- this is the #seconds to delay
				if((j>(MIN_WPM-1)) && (j<(MAX_WPM+1))){	// only allow 5<=WPM<=31
					wpm_reg = j;
					w = j;
				}
				break;

			case 'u':					// ultra-low power
				PORTB = (PORTB & ~PWRMASK) | UPWR;
				break;

			case 'l':					// low power (lower-case "L")
				PORTB = (PORTB & ~PWRMASK) | LPWR;
				break;

			case 'm':					// med power
				PORTB = (PORTB & ~PWRMASK) | MPWR;
				break;

			case 'h':					// hi power
				PORTB = (PORTB & ~PWRMASK) | HPWR;
				break;
			}
		}else{
			putchar_cw(c, w);
		}
	}
	return(1);
}

//-----------------------------------------------------------------------------
//	putchar_cw() outputs CW translation of input char (ASCII)
//	s = an ASCII character (any character outside the supported morse set results
//		in no output, but otherwise is accepted without issue).
//	wpm = morse speed in words per minute based on the conventional 50 dits/word
//		definition.  A value of 0 aborts the function resulting in no key out
//		activity.  Thus, wpm = 0 has the effect of disabling the morse output.
//-----------------------------------------------------------------------------
void putchar_cw(char s   /* char to transmit */, char wpm)
{
	unsigned char temp;
	char cwchr;
	char cwlen;

	// range-limit wpm (can not == 0)
	if(wpm > 0){
		if(s == ' '){
			// delay 3dahs
			cw_delay(DAH3, wpm);
		}else{
			// trap underscore & re-map
			if(s == '_'){
				s = USCORE_CW;
			}
			// range limit the input character
			if(s > LAST_CW){
				s = LAST_CW;
			}
			if(s < FIRST_CW){
				s = LAST_CW;
			}
			// convert character to CW table index
			temp = s - FIRST_CW;
			// extract elements and lengths
			cwchr = cw_table[temp];
			cwlen = cwlen_table[temp];
			// pre-process "&"
			if(s == '&'){
				cw_key(ON);					// Here, '&' is treated as "ES"
				cw_delay(DIT, wpm);			// this code sends an 'E' with a letter space
				cw_key(OFF);				// and the CW table encodes '&' as an 'S' CW character
				cw_delay(LSPC, wpm);
			}
			// process CW character elements
			while(cwlen != 0){
				cw_key(ON);
				if((cwchr & 0x01) == 0x01){
					// delay dit
					cw_delay(DIT, wpm);
				}else{
					// delay dah
					cw_delay(DAH, wpm);
				}
				cw_key(OFF);
				// inter-element spacing, delay dit
				cw_delay(DIT, wpm);
				// advance to next element
				cwchr >>= 1;
				cwlen -= 1;
			}
			// inter-character spacing
			cw_delay(LSPC, wpm);
		}
	}
	return;
}

//-----------------------------------------------------------------------------
//	wpmg() sets global wpm register if >=5.  Returns current value on exit
//-----------------------------------------------------------------------------
char wpmg(char wpm   /* int wpm */ )
{
	if(wpm >= MIN_WPM){
		wpm_reg = wpm;
	}
	return wpm_reg;
}

// ********************************************
//  ************ PUTCW PRIVATE FNS ***********
// ********************************************

//-----------------------------------------------------------------------------
//	cw_key() activates or de-activates cw key signal.
//		This code is application dependent.  ON = signal active, OFF = signal
//		silent.  If the keying circuit uses inverse logic, it would be inverted
//		here (as in this example). 
//-----------------------------------------------------------------------------
void cw_key(char key   /* transmit on or off */ )
{
	if(key != ON){
		TCCR1 = TCCR_OFF;						// set up side-tone gen
		PORTB = PORTB & ~KEY;					// cwkey = 0;
	}else{
		TCCR1 = TCCR_ON;						// set up side-tone gen
		PORTB = PORTB | KEY;					// cwkey = 1;
	}
	return;
}

//-----------------------------------------------------------------------------
//	cw_delay() provides a delay increment of DITs.
//		element_len is the # of CW elements to delay.  Typical range is 1 to 9.
//		wpm = the CW code speed in words/min.  Typical range is 5 to 20.
//		This code is application dependent.  The example uses a for-loop to
//		establish the delay and must be tuned for each sysclk implementation.
//		A better scheme would be to use an un-committed timer to establish the
//		base delay increment and build from there.
//
//		Given the desired speed in WPM (N), the DIT time is defined by the
//		following equation:
//			T(dit) = 60 (sec/min) / 50 (dits/word) / N (words/min)
//                 = (1.2 / N) (sec/dit)
//                 = (1200 / N) (ms/dit)
//
//
//-----------------------------------------------------------------------------
void cw_delay(char element_len, char wpm)
{
	volatile long int base; // calibrated for att-85
	volatile long int i;
	volatile char j;

	if(wpm){
		base = 28387L / (long int)wpm;		// calibrated for att-85
	}else{
		base = 23655L;						// 1 sec
	}
	// do a delay * the base value
	for(j = 0; j < element_len; j++){
		for(i = 0; i < base; i++){
			continue;
		}
	}
	return;
}

//**************
// End Of File
//**************
