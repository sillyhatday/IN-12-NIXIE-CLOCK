Version: 1

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

Version 1.31:

Fixed a bug caused by adding 12 hour mode functionality where in normal operation, the time doesn't wrap at 24 back to 0.

Version 1.4:

Added battery backup mode to disable the MCU trying to display digits on the Nixies when 12v input is removed. Hope to save some power for running on battery power.

Version 1.45:

Updated the dev board to bring all features back in the code. Testing required.
Moved button logic into one function. Now it is called from main.

Version 1.50:

Added counter for number of hours clock has been running in total. Display mode triggered when button is pressed. Hijacked the reset button for development.
Run time is saved into EEPROM at addresses 0, 1, 2, 3 for unsigned long. Routine runs once. With new MCU addresses are cleared at boot and flag set in address 4
to indicate the EEPROM addresses are initialised. On boot the flag is checked, so as not to overwrite the runtime count.

Version 1.51 - 1.54:

Various different attempts at solving the syncing issue of the colon with the seconds tick. The bug appeared after more testing.
Removed ISR function that was tried in attempt to keep things in sync and in phase.
Rewrote entire colon handling from scratch. Modified timer function to count 500mS ticks to use as reference for colon. Two ticks now count 1 second.
Used GPIO pin to control blanking of CD4511 outputs. Removed long complex routine for multiplexing 2 digits. Reduced down to an if statement in main multiplex.

Version 1.6:

Added back in second multiplex routine but inside the main multiplex behind an if statement. Can't recall why I had to add it back in. Must have been buggy.
At least it's more stream lined than before. Improved speed of multiplex function by tracking which digit was last displayed and only blanking that one, not all six. Huge improvment to the display crispness. Blanking just one digit saves 83% of the time it took to blank 6 digits. 5 of those did not need to be. Used
digitalWriteFast library as a stop gap to using direct port manipulation much later on.

Version 1.61:

General tidy up of formating. Added lots of comments and removed most comments from each line unless needed for clarity. Most comments are in the top of each function in one text block. Removed some unused variables. Removed reset button references ready to trigger reset on mode button hold, or status hold.

Version 1.65:

Once again removed the second multiplex function for two digit mode. No longer using any digitalWriteFast in multiplex function. Blanking digits with invalid
data sent to the CD4511. This was tried before and didn't work as I didn't remember to expand the array for the new data to send. It was sending random data
in RAM at that moment instead!
