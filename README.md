# FnBkn
ATTINY-85 Beacon Controller</p>
Current release: 1.0</p>
This project implements a simple beacon controller in an 8-pin MCU form-factor.
HW folder contains schematic and layout artifacts for the project.
SW Folder contains the Atmel Studio 7 project files
The message syntax is defined as follows:
*	Morse characters are all upper-case.
*	"axx":	key-up delay, no RF. "xx" = "00" to "99" for delays between 0 and 45.5 sec in half-second increments
*	"bxx":	key-dn delay, RF = on. "xx" = same as "a"
*	"zxx":	Set Morse speed in WPM. "xx" = "05" to "30", invalid values are ignored
*	"u":	ultra-low power, PWR[1:0] = %11 to turn on two, series attenuators
*	"l":	<lower-case "L"> low power, PWR[1:0] = %10 to turn on one, series attenuator
*	"m":	med power, PWR[1:0] = %01 to turn on one, series attenuator
*	"h":	low power, PWR[1:0] = %00 to turn off all attenuators

An example message:

"z20h VVV DE KE0FF/B EM10GOa02mb04 lb04a10"

The above message produces a 20wpm call and grid square message at high-power followed by 1 sec of silence,
then 3 sec of tone at medium power, followed by a word space (4 dit times) ending with 3 sec of tone at low power.
This message is contained within an editable string in main.c.  Edit this string to the desired message, then
compile and program the device.  <i>Note: The programming port shares the KEY pin.  The RF section should be
disconnected or disabled when programming.</i>

ATTINY-85 port map:
 *	PB0 = KEY output, "high" = RF off, "low" = RF on (may be inverted by a build-option in putcw.c).
 *	PB2 = "M" power.  "high" = medium attenuator on.
 *	PB3 = "L" power.  "high" = low attenuator on.
 *	PB[3:2] = "00" is high power (all attenuators off).
 *	PB4 = side-tone (square-wave).  Frequency is set in main.h.
