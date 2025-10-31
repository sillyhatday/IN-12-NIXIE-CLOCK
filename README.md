Initial version: 1

A generally functioning clock.

6 Digits for hours, minutes and seconds.
24 hour mode only.
Flashing colon function in time with seconds changing.
Reset button to clear contents of hours, minutes and seconds counters.
Runs on ATmega8A and Atmega328P at 16MHz external crystal

Version: 1.1

Fixed bug with hours counter not resetting properly when reset button pressed. Didn't clear hours count.
Fixed bug with hours being able to go past 23 when using hours button. Incorrect modulo math.

Version 1.2:

12 hour time mode added. Selectable with button. Keeps time when switching to 12 hour mode only.

Version 1.3:

12 hour mode improved. No longer screwing with the hours counter. Leaving that counter alone and translating the 24 hour format on the fly.
Now keeps time switching back and forth between 12/24h modes.
Added display funtion to show what hour mode the system has been switched to.
Display routine for showing the hour mode written from scratch due to brightness increase when multiplexing 2 digits vs 6.
