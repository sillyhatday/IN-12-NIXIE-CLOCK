## 🟦 Clock Functions

* The clock will power on and display time starting at 00:00:00 when it is plugged in. Whenever the clock is powered on, it will default to 24 hour mode. This only applies to the first time it is powered or the backup battery is depleted.

* The "hours" button is to increment the hours digits to the desired time, the same logic applies to the "minutes" button. The seconds count cannot be set. If the user wishes to sync the clock down to the second, the reset function can be used to set the clock back to 00:00:00. NEEDS REIMPLEMENTING IN V2.0

* To reset the clock to 00:00:00, hold down the "status" button for 3 seconds. Continuing to hold the button will keep the clock at this time until it is released, allowing it to be synchronised easily. NOT IMPLEMENTED

* The "status" button is multi-function. It is used to reset the clock time and to display the total run time of the clock with the display tubes powered on. A single press of the "status" button will show the hours counter. This information is stored in EEPROM memory and is saved permanently, unless the micro controller is reprogrammed and EEPROM is selected for erase. Powering off the clock, by removing the DC supply, and/or removing the internal backup battery, will not erase the counter.

* The counter uses an internal timer to record data. Pressing the "hours" button does not increase the recorded hours.

* When the clock is removed from USB-C power, the displays will turn off. Internally the clock is still running and keeping time, providing the backup battery is not depleted. This same condition can be entered using the "standby" button. This simulates the removal of USB-C power, but will not deplete the backup battery. The clock will continue to run in this state indefinitely.

* The clock will only record run time, when it is powered by the USB-C input. If the clock is on backup battery, the counter will no longer increment. The same applies for manually putting the clock in standby with the "standby" button.

* A 12 hour mode is selectable with the "mode" button. When the button is pressed, the display will briefly display 12 or 24, depending on the mode that has been set. The button will toggle between 12 and 24 hour modes.

* While the clock is plugged in, the backup batteries are charged at 8 - 11mA depending on the battery state of charge. When the batteries are full, they are charged constantly at 8mA. Due to the battery technology of NiMh, they can be left charging at a low current for a long time. If this function is not wanted, omit R39 and the batteries will not be charged. As the function of the batteries is to keep time during a power outage, having them charge is not critical. While plugged into USB-C power, even when in standby, they are being charged. It's likely the life of the batteries will be shortened from being on charge for so long, it may be better to remove the batteries and recharge them manually every 6 months or replace alkaline batteries once a year.
