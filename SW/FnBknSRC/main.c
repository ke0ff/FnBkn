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
 * The repeating message uses embedded control commands to effect the change of attenuator outputs, change of CW
 *	speed, and key-up or key-down delays.  The current command list:
 *
 *	Morse characters are all upper-case.
 *	"ax":	key-up delay, no RF. "x" = "0" to "9" for delays between 1 and 10 sec.
 *	"bx":	key-dn delay, RF = on. "x" = "0" to "9" for delays between 1 and 10 sec.
 *	"zxx":	Set Morse speed in WPM. "xx" = "05" to "30", invalid values are ignored
 *	"u":	ultra-low power, PWR[2:1] = %11 to turn on two, series attenuators
 *	"l":	<lower-case "L"> low power, PWR[2:1] = %10 to turn on one, series attenuator
 *	"m":	med power, PWR[2:1] = %01 to turn on one, series attenuator
 *	"h":	low power, PWR[2:1] = %00 to turn off all attenuators
 *
 * An example message:
 *
 *	"z20h VVV DE KE0FF/B EM10GOa0mb2 lb2a5"
 *
 *	The above message produces a 20wpm call and grid square message at high-power followed by 1 sec of silence,
 *	then 3 sec of tone at medium power, followed by a word space (4 dit times) ending with 3 sec of tone at low power.
 *
 */ 

// Version 0.0, 03-23-24
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

	PORTB = PORTB & ~KEY;						// Init PORTB & set for output
	DDRB = DDRB | KEY | PWR1| PWR2| TONE;
	wpmg(30);									// init speed
	while(1)
	{
		puts_cw("z20h VVV DE KE0FF/B EM10GOa0b2 mb2 lb2a5", wpmg(0));
//		puts_cw("z20 OK u l m h", wpmg(0));
	}

	return EXIT_SUCCESS;						// should never get here (fault-tolerant exit)
}

// end main.c