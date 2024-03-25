# FnBkn
ATTINY-85 Beacon Controller
This project implements a simple beacon controller in an 8-pin MCU form-factor.
HW folder contains schematic and layout artifacts for the project.
SW Folder contains the Atmel Studio 7 project files
The message syntax is defined as follows:
*	Morse characters are all upper-case.
*	"ax":	key-up delay, no RF. "x" = "0" to "9" for delays between 1 and 10 sec.
*	"bx":	key-dn delay, RF = on. "x" = "0" to "9" for delays between 1 and 10 sec.
*	"zxx":	Set Morse speed in WPM. "xx" = "05" to "30", invalid values are ignored
*	"u":	ultra-low power, PWR[2:1] = %11 to turn on two, series attenuators
*	"l":	<lower-case "L"> low power, PWR[2:1] = %10 to turn on one, series attenuator
*	"m":	med power, PWR[2:1] = %01 to turn on one, series attenuator
*	"h":	low power, PWR[2:1] = %00 to turn off all attenuators

An example message:

"z20h VVV DE KE0FF/B EM10GOa0mb2 lb2a5"

The above message produces a 20wpm call and grid square message at high-power followed by 1 sec of silence,
then 3 sec of tone at medium power, followed by a word space (4 dit times) ending with 3 sec of tone at low power.
