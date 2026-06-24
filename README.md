# IN-12 Nixie Tube Clock

## Introduction

This clock was designed with the idea of using IN-12 Nixie Tubes as its display. The design was intended to look similar to something from the 1960s with a cyberpunk style. I'm not sure I have managed this or not. I think it is more art deco style.

This did not work out and I went for something that took inspo from the 1950s. I'd say more Fallout than cyberpunk. Maybe American diner or 50's American car. I like it.

The design is to be modular and 3D printing or machining friendly. The final build I would like making from aluminium or stainless steel.

---

## 🔷 Description

The internal electronics are mounted on two PCBs with a connecting pin header. One PCB for power supplies and main logic, the second for the displays and display driver. The choice of putting the display driver on the display PCB was to reduce the amount board to board connections.

For the PCBs to stack and not require extra components, the housing itself is designed in such a way that each PCB is keyed. This allows the PCBs to fit in only one position.

The logic board contains two power supplies, MCU and anode drivers. One power supply is to provide the 180v required for the displays to operate. The second power supply provides power to the logic circuit. It provides 5v output through a diode to prevent the backup battery from feeding current into the buck converter output when not energised.

A 9 - 12v DC supply was decided based on testing. The 180v power supply operates more reliably with a higher input voltage, to a point. 5v requires a large high current to bring the output voltage rail up to 180v. The system will operate from 8 to 16v on the input.

The anode drivers are made up of discrete components. An integrated solution couldn't be found to handle the 180v in a through hole package. The standard way of doing this is to use two transistors to make a high side switching driver. A more detailed explanation can be found easily online. I like the visual of this approach also, it fits the old look of the device I wanted.

---

## 🫐 Progress

### 🟢 Done
- First revision of PCBs
- Working firmware with some features implemented.
- Stable high voltage power supply
- Case design first revision made
- All remaining case parts, buttons, legs, back cover.
- Updated PCB & fixed errors. Changed fuse, standby circuit, rearranged PCB.
- Added onboard USB-C 9 - 12v support.
- Finished adding firmware features, standby, battery backup, EEPROM clear.
- Porting to ATMEGA328 to use lower voltage and more efficient core, LOD 1.8v vs LOD 2.7v.
- Finishing case cosmetics.

### 🟡 Working On
- Digit ghosting fix. Problem in hardware somewhere.
- Power measurement testing. MCU power reduction on battery power.
- Standby code needs inverting for new 180v power control circuit.
- Tidying up lettering on the back.
- New photos of finished product and new PCBs
- LCSC or Mouser cart ready to buy.

### ⚪ Next
- Done!

---

## Photos

### Final Concept
<img width="800" height="618" alt="Nixie clock assembled smol" src="https://github.com/user-attachments/assets/1e9a8212-0402-4403-ae0a-39a79d97e26e" />
<img width="800" height="618" alt="Nixie clock angled rear assembled smol" src="https://github.com/user-attachments/assets/b2e335b6-4297-470a-801e-8ea8237b3a22" />
<img width="800" height="618" alt="Nixie clock assembled rear smol" src="https://github.com/user-attachments/assets/f9d1d854-db19-4c00-a30b-db975b41e8d6" />

### Internal View
<img width="800" height="618" alt="nixie clock internal top view" src="https://github.com/user-attachments/assets/56e1505e-102d-4b6d-bf5e-d885416d1973" />
<img width="800" height="618" alt="nixie clock internal front angle" src="https://github.com/user-attachments/assets/8c229f4f-3645-4528-afa7-dc3dd4bc3fb2" />
<img width="800" height="618" alt="nixie clock internal back angle" src="https://github.com/user-attachments/assets/3e675bef-5c9b-4115-9544-618f865e6af4" />

### Current State
<img width="800" height="450" alt="WhatsApp Image 2026-06-04 at 9 53 20 PM (3)" src="https://github.com/user-attachments/assets/33768385-b777-4f27-adf4-5650587a16cc" />
<img width="800" height="450" alt="WhatsApp Image 2026-06-04 at 9 53 20 PM" src="https://github.com/user-attachments/assets/fc3a82e6-0a45-44be-8f08-ae0c306f62af" />
<img width="800" height="450" alt="nixie clock back smol" src="https://github.com/user-attachments/assets/cce45c70-2b98-4239-8bac-afe90024f1c8" />

---

## Complete Parts List

| Part | Part No | Qty |
| --- | --- | --- |
| JST Connector | - | 1 |
| Capacitor | 100nF | 5 |
| Capacitor | 20pF | 2 |
| Capacitor | 1nF | 1 |
| Capacitor | 220pF | 1 |
| Capacitor | 4.7uF | 1 |
| Capacitor | 100uF | 4 |
| Diode | 1N5819 | 4 |
| Diode | UF4007 | 1 |
| Fuse | 300mA | 1 |
| Pin Header | 1x2 | 1 |
| Pin Header | 1x16 | 1 |
| Pin Socket | 1x16 | 1 |
| Inductor | 100uH | 1 |
| Inductor | 220uH | 1 |
| Inductor | 1uH | 1 |
| Transistor | BC516 1 |
| Transistor | KSP92 | 6 |
| Transistor | MPSA42 | 6 |
| Transistor | 2N3904 | 1 |
| MOSFET | IRF9540N | 1 |
| MOSFET | IRF740 | 1 |
| Resistor | 0R75 | 1 |
| Resistor | 0R1 | 1 |
| Resistor | 10K | 21 |
| Resistor | 412K | 6 |
| Resistor | 1K | 5 |
| Resistor | 15K | 1 |
| Resistor | 4K7 | 1 |
| Resistor | 100K | 1 |
| Resistor | 6K8 | 1 |
| Resistor | 1K5 | 2 |
| Resistor | 220K | 1 |
| Potentiometer | 2K | 1 |
| Potentiometer | 1K | 1 |
| Button | SHOU HAN 5.8 WSPT | 4 |
| Button | SHOU HAN 5.8 WSPT | 1 |
| MCU | ATMEGA8 | 1 |
| IC | MC34063A | 2 |
| IC | K155ID1 | 1 |
| Tube | IN-12 Nixie | 6 |
| Crystal | 1MHz | 1 |

---

## 💻 Firmware Changelog

| Version | Status | Notes |
| ------- | ------ | ----- |
| V1.0.0  | Initial | A generally functioning clock. 6 Digits for hours, minutes and seconds. 24 hour mode only. Flashing colon function in time with seconds changing. Reset button to clear contents of hours, minutes and seconds counters. Runs on ATmega8A and Atmega328P at 16MHz external crystal |
| V1.1.0  | Defunct | Fixed bug with hours counter not resetting properly when reset button pressed. Didn't clear hours count. Fixed bug with hours being able to go past 23 when using hours button. Incorrect modulo math |
| V1.2.0  | Defunct | 12 hour time mode added. Selectable with button. Keeps time when switching to 12 hour mode only |
| V1.3.0  | Defunct | 12 hour mode improved. No longer screwing with the hours counter. Leaving that counter alone and translating the 24 hour format on the fly. Now keeps time switching back and forth between 12/24h modes. Added display function to show what hour mode the system has been switched to. Display routine for showing the hour mode written from scratch due to brightness increase when multiplexing 2 digits vs 6 |
| V1.3.1  | Defunct | Fixed a bug caused by adding 12 hour mode functionality where in normal operation, the time doesn't wrap at 24 back to 0 |
| V1.4.0  | Defunct | Added battery backup mode to disable the MCU trying to display digits on the Nixies when 12v input is removed. Hope to save some power for running on battery power |
| V1.4.5  | Defunct | Updated the dev board to bring all features back in the code. Testing required. Moved button logic into one function. Now it is called from main |
| V1.5.0  | Defunct | Added counter for number of hours clock has been running in total. Display mode triggered when button is pressed. Hijacked the reset button for development. Run time is saved into EEPROM at addresses 0, 1, 2, 3 for unsigned long. Routine runs once. With new MCU addresses are cleared at boot and flag set in address 4 to indicate the EEPROM addresses are initialised. On boot the flag is checked, so as not to overwrite the runtime count |
V1.5.1-1.5.4 | Defunct | Various different attempts at solving the syncing issue of the colon with the seconds tick. The bug appeared after more testing. Removed ISR function that was tried in attempt to keep things in sync and in phase. Rewrote entire colon handling from scratch. Modified timer function to count 500mS ticks to use as reference for colon. Two ticks now count 1 second. Used GPIO pin to control blanking of CD4511 outputs. Removed long complex routine for multiplexing 2 digits. Reduced down to an if statement in main multiplex |
| V1.6.0  | Defunct | Added back in second multiplex routine but inside the main multiplex behind an if statement. Can't recall why I had to add it back in. Must have been buggy. At least it's more stream lined than before. Improved speed of multiplex function by tracking which digit was last displayed and only blanking that one, not all six. Huge improvement to the display crispness. Blanking just one digit saves 83% of the time it took to blank 6 digits. 5 of those did not need to be. Used digitalWriteFast library as a stop gap to using direct port manipulation much later on |
| V1.6.1  | Defunct | General tidy up of formatting. Added lots of comments and removed most comments from each line unless needed for clarity. Most comments are in the top of each function in one text block. Removed some unused variables. Removed reset button references ready to trigger reset on mode button hold, or status hold |
| V1.6.5  | Defunct | Once again removed the second multiplex function for two digit mode. No longer using any digitalWriteFast in multiplex function. Blanking digits with invalid data sent to the CD4511. This was tried before and didn't work as I didn't remember to expand the array for the new data to send. It was sending random data in RAM at that moment instead! This was needed realising that the nixie tube driver doe not have a blanking input. Nixie PSU gate control output added ready for prototype PSU and logic PCB |
| V1.7.0 | Defunct | Removed some debugging code using GPIO pins. Added an output to drive a MOSFET gate that will control the 180v boost supply, depending on 12v input being present or not. Code was added onto the section controlling display blanking as the input to perform the display blanking was already implemented |
| V1.7.1 | Defunct | Changed setup function to deal with stored EEPROM data differently and how it is initialised on first power up. Added in a basic check routine to confirm EEPROM is working, else show 999,999 on the run time display. Not sure it was needed as the problems I was having went away when shifting the EEPROM storage addresses. I think this unknown Aliexpress chip is somewhat dodgy. It is probably worn out. The chip has been reprinted on top while the underside is beat to hell |
| V1.8.0 | Defunct | Implemented a multi-function button routine. Status button now serves dual function. It is a new function in code that handles just that button, where it should be merged with the existing button handler. I don't plan on any more multi function buttons, so I'll leave this alone. Holding the status button now does a software reset of the time. Pressing the button works as before but activates on release of the button, not on the press |
| V1.8.1 | Defunct | Fixed a bug that caused the run time counter to roll over at 255 |
| V2.0.0 | Defunct | Complete rewrite of the whole software. Code is built around state machines now, no more blocking flag everywhere. Some features have been removed or not yet implemented. Missing code for standby function. Removed blinking colon, don't like it anymore |
| V2.1.0 | Defunct | Code modified to work on the real nixie tube setup. Only small things like the multiplex scan direction and stuff I've forgotten. Fixed bug with button debouncing not working great since the re-write. Attempted to fix problem with run time hours data getting messed up. Not wure why as it didnt have any issue on the dev board |
| V2.2.0 | Defunct | Added standby feature back to system. Power loss or power switch perform the same function. Displays off, 180v supply off, sleep cpu. CPU wakes once per second to keep time and check if escape standby. Not sure eeprom fix is working still. Need to add code to stop eeprom code being updated while in sleep. It's only to track tube run time |
| V2.2.3 | Current | Moved software over to ATmega328. Implemented clock divider to reduce clock speed to 2MHz. At 1MHz there was a slight percievable flicker when seconds incremented |
