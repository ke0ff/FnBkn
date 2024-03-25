/*
 * FnBkn.c
 * The "F'n Beacon"
 *
 * Created: 03/24/2024 21:00:40
 * Author : KE0FF-Actual
 * main.c (c) 03/2024 by Joseph Michael Haas, all rights reserved
 *
 * This project implements a simple beacon controller using a compile-time edited message.
 *	Controller outputs a key signal to turn RF on/off as well as a pair of attenuator control bits.
 *	Finally, a fixed frequency sidetone output is provided for operator sidetone or FM CW.
 *
 * Port map:
 *	PB0 = KEY output, "high" = RF off, "low" = RF on.
 *	PB2 = "M" power.  "high" = medium attenuator on.
 *	PB3 = "L" power.  "high" = low attenuator on.
 *	PB[3:2] = "00" is high power (all attenuators off).
 *	PB4 = side-tone (square-wave).  Frequency set in main.h.
 *
 * The repeating message uses embedded control commands to effect the change of attenuator outputs, change of CW
 *	speed, and key-up or key-down delays.  The current command list:
 *
 *	Morse characters are all upper-case.
 *	"axx":	key-up delay, no RF. "xx" = "0" to "99" for delays between 0.5 and 49.5 sec
 *	"bxx":	key-dn delay, RF = on. "xx" = same as for "a"
 *	"zxx":	Set Morse speed in WPM. "xx" = "05" to "30", invalid values are ignored
 *	"u":	ultra-low power, PWR[2:1] = PB[3:2] = %11 to turn on two, series attenuators
 *	"l":	<lower-case "L"> low power, PWR[2:1] = %10 to turn on one, series attenuator
 *	"m":	med power, PWR[1:0] = %01 to turn on one, series attenuator
 *	"h":	low power, PWR[1:0] = %00 to turn off all attenuators
 *
 * An example message:
 *
 *	"z20h VVV DE KE0FF/B EM10GOa02mb04 lb04a10"
 *
 *	The above message produces a 20wpm call and grid square message at high-power followed by 1 sec of silence,
 *	then 3 sec of tone at medium power, followed by a word space (4 dit times) ending with 3 sec of tone at low power.
 *
 *	Edit the quoted string contained within the msg_string[] declaration below, then build and program the device.
 *	NOTE: The KEY output is shared with the programmer interface.  It is recommended that the RF section be disconnected
 *		or disabled during programming.
 *
 *	A power-on boot message is provided.  This may also be edited or removed as desired.
 *
 */ 

// Version 1.0, 03-25-24
//		* Inital release
// Version 0.0, 03-23-24
//		* Added "boot" message for power-on indication
//		* Added sidetone generator using timer1 in normal mode, CTC.  Design notes:
//			Timer1 for tone output: see main.h for tone equation
//			OCR1A/B/C = compare reg (only seems to work if all three are set to the same compare value
//			CTC = 1; COM1B[1:0] = 01; CS[13:10] = 0100 or 0000 for tone off
//		* Initial code & test


#include <avr/io.h>
#include "main.h"
#include "putcw.h"

// local function declarations

// local defines


////////////////////////////////////////////////////
// main() operates main-loop of control function
////////////////////////////////////////////////////

int main(void)
{
	char msg_string[] = {"z20h VVV DE KE0FF/B EM10GO a02b04 mb04 lb04a10"}; // edit the text inside the quotes

//	char string[] = {"OKb01a01b10 "}; // test string

	TCCR1 = TCCR_OFF;							// set up side-tone gen
	GTCCR = 0x10;
	OCR1A = TMR_FREQ;							// set side-tone freq
	OCR1B = TMR_FREQ;
	OCR1C = TMR_FREQ;
	PORTB = PORTB & ~KEY;						// Init PORTB & set for output
	DDRB = DDRB | KEY | PWR1| PWR0 | TONE;
	wpmg(20);									// init speed
	puts_cw("V1.0 BOOT ", wpmg(0));				// boot-up message
	while(1)
	{
		puts_cw(msg_string, wpmg(0));
	}

	return EXIT_SUCCESS;						// should never get here (fault-tolerant exit)
}

// end main.c