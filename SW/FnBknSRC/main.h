/*
 * main.h
 *
 * header file for FnBkn project
 *
 * by Joe Haas, 03/23/2024
 *
 */ 

#ifndef ATTINY_SRC_MAIN_H_
	#define ATTINY_SRC_MAIN_H_

	#ifndef F_CPU
		#define F_CPU 7695360UL //3686400UL
	#endif

	#ifndef DEVICE
		#define DEVICE attiny85
	#endif

	#ifndef __AVR_ATtiny85__
		#define __AVR_ATtiny85__
	#endif

	#include <avr/io.h>
	#include <stdlib.h>
	#include <stdio.h>
	#include <stdint.h>
	#include <avr/interrupt.h>
	#include <stdbool.h>
	#include <util/delay.h>

#define	TONE_FREQ				600L			// (Hz) sets side-tone squarewave frequency

#define TCCR_ON					0x84			// enables timer (tone on)
#define TCCR_OFF				0x80			// stops timer (tone off)
#define	TMR_FREQ				(char)(62000L/(long int)TONE_FREQ)
#define MIN_WPM					(5)				// min/max wpm limits
#define MAX_WPM					(30)
#define KEY_PORT				PORTB
#define KEY_PIN					PINB
#define KEY_DDR					DDRB
#define KEY						0x01			//PB0
#define PWR0					0x04			//PB2
#define PWR1					0x08			//PB3
#define PWRMASK					(PWR1|PWR0)
#define UPWR					(PWR1|PWR0)
#define LPWR					(PWR1)
#define MPWR					(PWR0)
#define HPWR					(0)
#define TONE					0x10			//PB4

#endif /* ATTINY_SRC_MAIN_H_ */
