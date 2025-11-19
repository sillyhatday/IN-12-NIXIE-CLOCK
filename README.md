## IN-12 Nixie Tube Clock

This clock was designed with the idea of using IN-12 Nixie Tubes as its display. The design was intended to look similar to something from the 1960s with a cyberpunk style. You could say it is more like something from the Fallout franchise.

The design is to be modular and 3D printing or machining friendly. The final build I would like making from aluminium or stainless steel. The use of feet or legs is yet to be determined.

The internal electronics are mounted on two PCBs with a connecting pin header. One PCB for power supplies and main logic, the second for the displays and display driver. The choice of putting the display driver on the display PCB, was to reduce the amount board to board connections.

The logic board contains two power supplies, MCU and anode drivers (amongst other things). One power supply is to provide the voltage required for the displays to operate. 180v was selected to keep the voltage as low as possible while keeping display operation reliable. Too low of a voltage can could issues with the display striking. The second power supply provides power to the logic circuit. It provides 5v output.

A 12v DC supply was decided based on testing. The 180v power supply operates more reliably with a higher input voltage. A 5v DC supply causes reliability problems with the 180v power supply starting. 9v was also considered as it works just as well, but 12v was decided as I have a lot of 12v DC adaptors. A USB C supply hasn't been eliminated. I just haven't made a circuit before to negotiate higher voltages like 9v, 12v, 15v. This will come in a later version.

The anode drivers are made up of discrete components. An integrated solution couldn't be found to handle the 180v in a through hole package. The standard way of doing this is to use two transistors, one to handle the higher voltage and one to handle the logic voltage. Each transistor does not see the others voltage. Therefore the higher voltage cannot damage the more sensitive lower voltage components. A more detailed explanation can be found easily online.

It is currently unfinished. It is still in software development and finalisation of hardware. The logic PCB is underway, requiring some hardware changes before continuing.

## Clock Functions

The clock will power on and display time starting at 00:00:00 when it is plugged in. Whenever the clock is powered on, it will default to 24 hour mode. This only applies to the first time it is powered or the backup battery is depleted.

The "hours" button is to increment the hours digits to the desired time, the same logic applies to the "minutes" button. The seconds count cannot be set. If the user wishes to sync the clock down to the second, the reset function can be used to set the clock back to 00:00:00.

To reset the clock to 00:00:00, hold down the "status" button for 3 seconds. Continuing to hold the button will keep the clock at this time until it is released, allowing it to be synchronised easily. NOT FULLY IMPLEMENTED

The "status" button is multi-function. It is used to reset the clock time and to display the total run time of the clock with the display tubes powered on. A single press of the "status" button will show the hours counter. This information  is stored in the clocks EEPROM memory and is saved permanently, unless the micro controller is reprogrammed and EEPROM is selected for erase. Powering off the clock, by removing the DC supply, and/or removing the internal backup battery, will not erase the counter.

The counter uses an internal timer to record data. Pressing the "hours" button does not increase the recorded hours.

When the clock is removed from DC input power, the displays will turn off. Internally the clock is still running and keeping time, providing the backup battery is not depleted. This same condition can be entered using the "standby" button. This simulates the removal of DC power, but will not deplete the backup battery. The clock will continue to run in this state indefinitely until the DC supply is removed and the backup battery is depleted.

The clock will only record an increment in the run time, when it is powered by the DC input. If the clock is on backup battery, the counter will no longer increment. The same applies for manually putting the clock in standby with the "standby" button.

A 12 hour mode is selectable with the "mode" button. When the button is pressed, the display will briefly switch to a 2 digit mode displaying 12 or 24, depending on the mode that has been set. The button will toggle between 12 and 24 hour modes.

## Changelog

V1.0:
* A generally functioning clock. 6 Digits for hours, minutes and seconds. 24 hour mode only. Flashing colon function in time with seconds changing. Reset button to clear contents of hours, minutes and seconds counters. Runs on ATmega8A and Atmega328P at 16MHz external crystal
V1.1:
* Fixed bug with hours counter not resetting properly when reset button pressed. Didn't clear hours count. Fixed bug with hours being able to go past 23 when using hours button. Incorrect modulo math.
V1.2:
* 12 hour time mode added. Selectable with button. Keeps time when switching to 12 hour mode only.
V1.3:
* 12 hour mode improved. No longer screwing with the hours counter. Leaving that counter alone and translating the 24 hour format on the fly. Now keeps time switching back and forth between 12/24h modes. Added display function to show what hour mode the system has been switched to. Display routine for showing the hour mode written from scratch due to brightness increase when multiplexing 2 digits vs 6.
V1.31:
* Fixed a bug caused by adding 12 hour mode functionality where in normal operation, the time doesn't wrap at 24 back to 0.
V1.4:
* Added battery backup mode to disable the MCU trying to display digits on the Nixies when 12v input is removed. Hope to save some power for running on battery power.
V1.45:
* Updated the dev board to bring all features back in the code. Testing required. Moved button logic into one function. Now it is called from main.
V1.50:
* Added counter for number of hours clock has been running in total. Display mode triggered when button is pressed. Hijacked the reset button for development. Run time is saved into EEPROM at addresses 0, 1, 2, 3 for unsigned long. Routine runs once. With new MCU addresses are cleared at boot and flag set in address 4 to indicate the EEPROM addresses are initialised. On boot the flag is checked, so as not to overwrite the runtime count.
V1.51 - 1.54:
* Various different attempts at solving the syncing issue of the colon with the seconds tick. The bug appeared after more testing. Removed ISR function that was tried in attempt to keep things in sync and in phase. Rewrote entire colon handling from scratch. Modified timer function to count 500mS ticks to use as reference for colon. Two ticks now count 1 second. Used GPIO pin to control blanking of CD4511 outputs. Removed long complex routine for multiplexing 2 digits. Reduced down to an if statement in main multiplex.
V1.6:
* Added back in second multiplex routine but inside the main multiplex behind an if statement. Can't recall why I had to add it back in. Must have been buggy. At least it's more stream lined than before. Improved speed of multiplex function by tracking which digit was last displayed and only blanking that one, not all six. Huge improvement to the display crispness. Blanking just one digit saves 83% of the time it took to blank 6 digits. 5 of those did not need to be. Used digitalWriteFast library as a stop gap to using direct port manipulation much later on.
V1.61:
* General tidy up of formatting. Added lots of comments and removed most comments from each line unless needed for clarity. Most comments are in the top of each function in one text block. Removed some unused variables. Removed reset button references ready to trigger reset on mode button hold, or status hold.
V1.65:
* Once again removed the second multiplex function for two digit mode. No longer using any digitalWriteFast in multiplex function. Blanking digits with invalid data sent to the CD4511. This was tried before and didn't work as I didn't remember to expand the array for the new data to send. It was sending random data in RAM at that moment instead! This was needed realising that the nixie tube driver doe not have a blanking input. Nixie PSU gate control output added ready for prototype PSU and logic PCB.
