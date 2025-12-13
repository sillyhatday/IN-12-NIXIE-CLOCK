// ------------------------------------------------------------------------------------------------------------------------------------- //
//                                                      ATmega8 clock firmware                                                           //
//                                                    Test software version 1.61                                                         //
//                                               Used in strip board protoype for ISP                                                    //
// ------------------------------------------------------------------------------------------------------------------------------------- //

// Todo List:
// Reduce amount of buttons. Use one button for multi function. Double click, hold, etc.

// ******************************************************* Include/Define ************************************************************** //

#include <EEPROM.h>
#include <digitalWriteFast.h>

// *************************************************** ATmega8A Pin Assignments ******************************************************** //
//
//  AVR  PIN  IDE   Description
//  PD0 = 2  = 0  = ISP Rx Pin
//  PD1 = 3  = 1  = ISP Tx Pin
//  PD2 = 4  = 2  = BCD 1 Pin
//  PD3 = 5  = 3  = BCD 2 Pin
//  PD4 = 6  = 4  = BCD 4 Pin
//  PD5 = 11 = 5  = BCD 8 Pin
//  PD6 = 12 = 6  = Hours Tens Cathode
//  PD7 = 13 = 7  = Hours Units Cathode
//  PB0 = 14 = 8  = Minutes Tens Cathode
//  PB1 = 15 = 9  = Minutes Units Cathode
//  PB2 = 16 = 10 = Seconds Tens Cathode
//  PB3 = 17 = 11 = Seconds Units Cathode
//  PB4 = 18 = 12 = Colon seperator
//  PB5 = 19 = 13 = 180v On/Off
//  PB6 = 9  = -- = Crystal In
//  PB7 = 10 = -- = Crystal In
//  PC0 = 23 = A0 = Nixie Run Time Button
//  PC1 = 24 = A1 = Hour Increment Button
//  PC2 = 25 = A2 = Minute Increment Button
//  PC3 = 26 = A3 = Debugging Afternoon Flag
//  PC4 = 27 = A4 = Hour Mode Button
//  PC5 = 28 = A5 = Power Monitor Input
//  PC6 = 1  = -- = Reset Input
//
// ********************************************************** Constants **************************************************************** //

const byte BCD_PINS[4] =               {2, 3, 4, 5};                  // BCD output pins
const byte DIGIT_PINS[6] =    {6, 7, 8, 9, 10, 11,};                  // Digit cathode pins
const byte COLON_PIN =                           12;                  // Decimal point pin
const byte NIXIE_PSU =                           13;                  // Gate drive for 180v PSU MOSFET
const byte STATUS_PIN =                          A0;                  // For showing EEPROM time data, implement double function as reset
const byte HOUR_PIN =                            A1;                  // Hour increment input pin
const byte MINUTE_PIN =                          A2;                  // Minute increment input pin
const byte AFTERNOON_FLAG =                      A3;                  // AM/PM indicator
const byte MODE_PIN =                            A4;                  // 24 or 12 hour mode selection
const byte PWR_PIN =                             A5;                  // Pin to monitor if main power is removed
const byte RUN_TIME_HOURS =                       0;                  // EEPROM address for nixie tube run time, 4 bytes 0 - 3
const byte EEPROM_INITIALISED =                   4;                  // EEPROM address to check if EEPROM is initialised
const unsigned short REFRESH_INTERVAL =         1000;                  // Microseconds to display each digit (1mS)
const unsigned short TIMER_INTERVAL =            500;                  // Milliseconds for definition of a half second
const unsigned short DEBOUNCE_DELAY =            150;                  // Time after button press to ignore inputs
const unsigned short SHOW_DATA_DURATION =       2000;                  // How long to show which hour mode we switched too
const byte BCD_CODES[11][4] = {                                      // Array for BCD value of numbers
  {0,0,0,0},                                                         // 0
  {1,0,0,0},                                                         // 1
  {0,1,0,0},                                                         // 2
  {1,1,0,0},                                                         // 3
  {0,0,1,0},                                                         // 4
  {1,0,1,0},                                                         // 5
  {0,1,1,0},                                                         // 6
  {1,1,1,0},                                                         // 7
  {0,0,0,1},                                                         // 8
  {1,0,0,1},                                                         // 9
  {1,1,1,1}                                                          // Invalid Data
};
// ******************************************************** Button states ************************************************************** //

bool colonState =                 false;                             // Colon pin on or off            
bool prevColonState =             false;                             // Sanity checking of colon state
bool resetButtonState =           false;                             // Is reset pressed
bool prevResetButtonState =       false;                             // Used for debouncing of reset button
bool minuteButtonState =          false;
bool prevMinuteButtonState =      false;
bool hourButtonState =            false;
bool prevHourButtonState =        false;
bool modeButtonState =            false;
bool prevModeButtonState =        false;
bool runTimeButtonState =         false;
bool prevRunTimeButtonState =     false;

// ******************************************************* Internal flags ************************************************************** //

bool afternoon =                  false;                             // Is it AM or PM   
bool twelveHourMode =             false;                             // Which hour mode is active
bool showHourMode =               false;                             // Display hour mode or not
bool showRunTime =                false;                             // Display run time or not
bool batteryBackup =              false;                             // Main power present or not
bool standbyDone =                false;                             // Tracking to only run standby once

// ******************************************************** Time counters ************************************************************** //

byte counterHalfSecond =              0;                             // Colon counter
byte counterSeconds =                 0;                             // Seconds counter
byte counterMinutes =                 0;                             // Minutes counter
byte counterHours =                   0;                             // Hours counter

// *************************************************** Multiplexing & displays ********************************************************* //

byte currentDigit =                   0;                             // Which display is active 0 to 5
byte digits[6] =     {0, 0, 0, 0, 0, 0};                             // Array for each 7seg display digit
byte twoDigitMultiplexCycle =         0;                             // How many times the function has run
byte twoDigitMultiplexIndex =         0;                             // Which digit is the function is on
byte eepromCounter =                  0;                             // Counter for incrementing hours independant of timer counter
unsigned long showHourModeStart =     0;                             // When did hour mode start showing
unsigned long showRunTimeStart =      0;                             // When did run time start showing
unsigned long nixieRunTime =          0;                             // Value read from EEPROM of nixie run time

// ******************************************************* Button Handling ************************************************************* //

unsigned long previousMicros =        0;                             // Last update time of multiplex
unsigned long previousMillis =        0;                             // Last update time of seconds count
unsigned long previousMillisColon =   0;                             // Last update time of DP
unsigned long minuteButtonPrevPush =  0;                             // Time when minute button was pressed last
unsigned long hourButtonPrevPush =    0;                             // Time when hour button was pressed last
unsigned long modeButtonPrevPush =    0;                             // Time when mode button was pressed last
unsigned long runTimeButtonPrevPush = 0;                             // Time when run time button was pressed last

// ========================================================= Functions ================================================================ //

void buttonHandler(int avrPin, bool &buttonState, bool &prevButtonState, unsigned long currentReferenceTime, unsigned long &buttonPrevPush);
void debug(void);
void eepromRead(void);
void eepromWrite(void);
void hourButtonHandler(void);
void minuteButtonHandler(void);
void multiplex(void);
void nixieRunTimeDisplay(void);
void resetHandler(void);
void standby(void);
void timer(void);

// ======================================================= One time setup ============================================================== //

void setup() {

     byte nixieStorageInit = EEPROM.read(EEPROM_INITIALISED);        // Read initialised data from EEPROM
  
     for (int i = 0; i < 4; i++) {                                   // Setup loop
        pinMode(BCD_PINS[i],       OUTPUT);
     }
     for (int i = 0; i < 6; i++) {
        pinMode(DIGIT_PINS[i],     OUTPUT);
        digitalWrite(DIGIT_PINS[i],   LOW);
     }
        pinMode(HOUR_PIN,    INPUT_PULLUP);
        pinMode(MINUTE_PIN,  INPUT_PULLUP);                             
        pinMode(MODE_PIN,    INPUT_PULLUP);
        pinMode(STATUS_PIN,  INPUT_PULLUP);
        pinMode(PWR_PIN,            INPUT);
        pinMode(COLON_PIN,         OUTPUT);
        pinMode(NIXIE_PSU,         OUTPUT);
        pinMode(AFTERNOON_FLAG,    OUTPUT);
        
        digitalWrite(COLON_PIN,       LOW);
        digitalWrite(DIGIT_BLANKING, HIGH);                          // To remove now digits blanked with invalid data

      if (nixieStorageInit != 0xAA) {                                // Check if EEPROM was previously initialised.
        unsigned long zero = 0;                                      // EEPROM.write will only write one byte not all 4 as below
        EEPROM.put(RUN_TIME_HOURS, zero);                            // EEPROM.put write thes 4 bytes of the long starting at byte 0
        EEPROM.write(EEPROM_INITIALISED, 0xAA);                      // Address 4 is out the way of the previous data
     }
}
// =========================================================== Main program ============================================================ //

void loop() {

  batteryBackup = !digitalRead(PWR_PIN);                             // Read GPIO just before loop to use in loop
  
  unsigned long currentButtonReferenceTime = millis();               // Log time for comparing with last time button pressed

  if (!standbyDone && batteryBackup) {                               // Check to see if standby routine is done
    standby();
  } else if (standbyDone && !batteryBackup) {
    digitalWriteFast(NIXIE_PSU, HIGH);
    standbyDone = false;
  }

  if (!standbyDone) {                                                // If on battery power, skip displaying data
    multiplex();
  }
  
  if (eepromCounter >= 60) {
    eepromWrite();
    eepromCounter = 0;
  }

  buttonHandler(HOUR_PIN,   hourButtonState,    prevHourButtonState,    currentButtonReferenceTime, hourButtonPrevPush);
  buttonHandler(MINUTE_PIN, minuteButtonState,  prevMinuteButtonState,  currentButtonReferenceTime, minuteButtonPrevPush); 
  buttonHandler(MODE_PIN,   modeButtonState,    prevModeButtonState,    currentButtonReferenceTime, modeButtonPrevPush);
  buttonHandler(STATUS_PIN, runTimeButtonState, prevRunTimeButtonState, currentButtonReferenceTime, runTimeButtonPrevPush);
  
  timer();  
  hourModeDisplay();
  nixieRunTimeDisplay();
  debug();

}
// ========================================================= Button Handler ============================================================ //
/*
 * The first time I have done a reusable function like this. Please bully me. Function handles button presses, the function prevents button
 * bounce from giving multiple inputs, it also prevents holding of buttons causing further inputs.
 *
 * Values are passed into this function. When this is done, the software will run with a copy of those variables. The & symbol in the
 * function header tells the system to not create local variable. This way the variables are updated system wide, not in the function
 * and updated after. This is needed as other functions use these variables, probably not while this is running though. Best to be safe.
 *
 * This works as follows. The time recorded when the last button was pressed is taken away from current recorded time. If the result is
 * greater than the debounce delay, the result is true. Button state is read as the inverse of its state. Low is when the button is pressed.
 * If the button state is true, the previous state of the button is false (used to stop holding the button and triggering again after the
 * debounce delay) and the time difference calculation is true, then go to switch. The switch uses the avrPin variable to choose which
 * case to run. 
*/

void buttonHandler(int avrPin, bool &buttonState, bool &prevButtonState, unsigned long currentReferenceTime, unsigned long &buttonPrevPush) {

   buttonState = !digitalReadFast(avrPin);
   
   if (buttonState && !prevButtonState && (currentReferenceTime - buttonPrevPush > DEBOUNCE_DELAY)) {        // If true, go to switch.
     switch(avrPin) {                                                                                        // value of avrPin selects case.
      case MINUTE_PIN:
        minuteButtonHandler();
        break;
      case HOUR_PIN:
        hourButtonHandler();
        break;
      case MODE_PIN:
        twelveHourMode = !twelveHourMode;
        showHourMode = true;
        showHourModeStart = millis();
        break;
      case STATUS_PIN:
        showRunTime = true;
        showRunTimeStart = millis();
        break;
     }
     buttonPrevPush = currentReferenceTime;
   } 
   prevButtonState = buttonState;
}
// ==================================================== Development debug ============================================================== //
/* Used during debugging. Can be deleted. */

void debug() {

  digitalWrite(AFTERNOON_FLAG, afternoon);
//  digitalWrite(HOUR_MODE_FLAG, twelveHourMode);
//  batteryBackup = false;
//  standbyDone = false;
//  Serial.print(digitalRead(PWR_PIN));
}
// =========================================================== EEPROM Reads ============================================================ //
/* Tiny function to grab information from EEPROM. Pointless really. */

void eepromRead() {

    EEPROM.get(RUN_TIME_HOURS, nixieRunTime);
}
// ========================================================== EEPROM Writes ============================================================ //
/* Simple routine to update the EEPROM with the run time of the displays. Dataa gets read out from EEPROM, incremented and written 
 * back into EEPROM. Probably could be optimised to not pull data from EEPROM each time after being read once. I suppose it keeps
 * things more error proof this way. */

void eepromWrite() {

    unsigned long eepromData = 0;

    EEPROM.get(RUN_TIME_HOURS, eepromData);
    eepromData++;
    EEPROM.put(RUN_TIME_HOURS, eepromData);
}
// =================================================== Hour button handler ============================================================= //
/* Called from the button handler functions. Increments the hours counter that is controlled by timer(). If hours pass 23 then modulo
 * will return 1. 8 o clock / 24 = 0 modulo returns input value of 8. 24 / 24 = 1. Hours counter is now equal to 1. The value that 
 * multiplex() uses is still at the old value of counterHours. Waiting for timer() to update these values, causes the displays to 
 * still show the old values breifly. The display values get updated here to fix that. */

void hourButtonHandler() {

    counterHours = (counterHours + 1) % 24;
    digits[0] = counterHours / 10;
    digits[1] = counterHours % 10;
}
// ================================================= Show hour mode when toggled ======================================================= //
/* While showHourMode is true, this function is run. This function continues to be run until the function start time minus the system 
 * timer is greater than display time. After that showHourMode is set to false. This function overrides the display data to show the user
 * which hour mode has been selected. 12 hour or 24 hour mode. "10" is assigned from the array as it is invalid data (15 or 0xF) for
 * display blanking */

void hourModeDisplay() {

    if (showHourMode && (millis() - showHourModeStart > SHOW_DATA_DURATION)) {
        showHourMode = false;
    }
    if (showHourMode) {
        if (twelveHourMode) {                                         // Display "12"
           digits[0] = 10;
           digits[1] = 10;
           digits[2] = 1;
           digits[3] = 2;
           digits[4] = 10;
           digits[5] = 10;
        } else {                                                      // Display "24"
           digits[0] = 10;
           digits[1] = 10;
           digits[2] = 2;
           digits[3] = 4;
           digits[4] = 10;
           digits[5] = 10;
        }
    }
}
// ================================================== Minute button handler ============================================================ //
/* Called from the button handler functions. Increments the minutes counter that is controlled by timer(). If minutes pass 59 then modulo
 * will return 1. 45 mins / 60 = 0 modulo returns input value of 45. 60 / 60 = 1. Minutes counter is now equal to 1. The value that 
 * multiplex() uses is still at the old value of counterMinutes. Waiting for timer() to update these values, causes the displays to 
 * still show the old values breifly. The display values get updated here to fix that. */

void minuteButtonHandler() {
 
    counterMinutes = (counterMinutes + 1) % 60;
    digits[2] = counterMinutes / 10;
    digits[3] = counterMinutes % 10;
}
// =================================================== Multiplexing Routine ============================================================ //
/* The main display function. Changes here must be calculated. Small changes can screw up the display stability easily. digitalWrite
 * has been reduced in use to keep the function faster and therefore the display more solid and smooth. This can be optimised more, but 
 * it isn't worth when direct register writes would make more difference than that ever would. Direct writes to be implemented.
 * As a stop gap, the fast write library is used where possible. This did make a nice difference in display brightness.
 * 
 * Code compensates for being called late. Should other functions take longer than desired before multiplex is called, it will account 
 * for this time loss and compensate. If previous micros minus current micros is greater than the refresh interval, then previous micros
 * is equal to previous micros plus refresh interval. This way any error is accounted for on the next loop. Thanks Drew for that input!
 * 
 * Original code turned off all digits before moving to next digit. This stops ghosting across the displays. This is updated to only switch
 * off the previous digit, as only one digit is ever on and turning off other digits is a waste of time as they are off already. Display
 * is much crisper after this.
 * 
 * Make a local variable equal the required data from the digits array. This is decided on the currentDigit updated by timer().
 * 
 * Removed the second multiplex routine. Instead of using the blanking input on the CD4511. Now invalid data is being sent to cause
 * its outputs to be low. */

void multiplex() {
    
    if (micros() - previousMicros >= REFRESH_INTERVAL) {
       previousMicros += REFRESH_INTERVAL;

       int digitPrevious = currentDigit - 1;                              // This is way faster than turning off digits
         if (digitPrevious < 0) {                                         // That were already off
           digitPrevious = 5;                                             // Only turn off the digit that was last on 
         }
       digitalWrite(DIGIT_PINS[digitPrevious], LOW);
          
       int num = digits[currentDigit];                                     // Load BCD for current digit
       
         for (int i = 0; i < 4; i++) {                                     // Set BCD outputs, repeat 4 times
           digitalWrite(BCD_PINS[i], BCD_CODES[num][i]);
         } 

       digitalWrite(DIGIT_PINS[currentDigit], HIGH);                       // Turn display on for new digit
          
       currentDigit++;                                                     // Cycle to next digit
       
       if (currentDigit >= 6) {                                            // If next digit is 6 or more, wrap to 0
         currentDigit = 0;
       }   
    }
}
// =================================================== Show nixie run time ============================================================= //
/* 123456 / 10 = 12345.6. 10 goes into 123456 evenly 12345 times. 10 doesn't go into 6. Remainder is 6. 123456 % 10 = 6.
 * Temp variable containing 123456 is now equal to 123456 / 10 = 12345.6 interger division drops remainders, loose 0.6.
 * Repeat for 12345 % 10 = 5. 12345 / 10 = 1234. Etc.
 * 
 * In other words, each time through the for loop grabs one of the digits of the 6 digit number in EEPROM, then spits back out the
 * remaining numbers until none are left. Counts backwards to assign the digits right to left, not left to right. Each remainder 
 * from the modulo is assigned to the digit. Modulo by 10 means that there is never a */

void nixieRunTimeDisplay() {

    if (showRunTime && (millis() - showRunTimeStart > SHOW_DATA_DURATION)) {
        showRunTime = false;                                               // Check time to know when to stop displaying
    }
    if (showRunTime) {
        eepromRead();
        unsigned long temp = nixieRunTime;
        for (int i = 5; i >= 0; i--) {
           digits[i] = temp % 10;                                          // Divide temp by ten, assign remainder to digit[i]
           temp /= 10;                                                     // Same as x = x / 10. Decimal place dropped.
        }
    }
}
// ================================================== Reset button handler ============================================================= //
/* Function zeros all counters, resets system counters and switches off colon. Updates the currently displayed digits else there will
 * be a delay before they are updated by timer(). */

void resetHandler() {

    for (int i = 0; i < 6; i++) {                                      // Set all digit cathodes OFF
      digits[i] =                0;      
    }
    counterHalfSecond =          0;                                    // Reset half second count
    counterSeconds =             0;                                    // Reset seconds count
    counterMinutes =             0;                                    // Reset minutes count
    counterHours =               0;                                    // Reset hours count
    previousMillis =      millis();                                    // Reset comparison time for timer function
    previousMillisColon = millis();                                    // Reset comparison time for colon function
    colonState =             false;                                    // Forces colon into known state for restart
}
// =================================================== Battery backup routine ========================================================== //
/* Turn off all digit cathode pins. Turn off colon output if it is on. Mark function as performed. For nixie tube version, a GPIO
 * controlling the 180v supply input will be needed. Update the standby flag so this is only performed once. */

void standby() {

   for (int i = 0; i < 6; i++) {
     digitalWrite(DIGIT_PINS[i], LOW);
   }
   digitalWriteFast(COLON_PIN, LOW);
   digitalWriteFast(NIXIE_PSU, LOW);
   standbyDone = true;
}
// ============================================ Count one second routine & assign digits =============================================== //
/* Code compensates for being called late. Check if time to run. 950uS, too early. Run other functions. Check time. 1100uS too late. 
 * Next ready time is now 900uS, not 1000uS from now. Otherwise error adds up slowly. eg, 1010mS + 1004mS + 1020mS = 3034mS error adds up. 
 * The code below accounts for that.eg, called at 1010mS, next ready time = 990mS. Each further late call is compensated. Only the first 
 * error is not fixed. Much the same as the multiplexing function.
 * 
 * When timing check is true, the 500mS counter is incremented by 1 and the colon state is inverted. Next increment resets counter to 0 &
 * increments the seconds count. When seconds counter passes 59, it gets reset and the minutes counter is incremented. Same thing for the
 * hours counter.
 * 
 * Each time one of the counters are incremented, the digits to display are updated for the multiplex funciton to use. This function
 * continues to run even when the input power source is removed and running on battery backup.
 * 
 * Extra code. Total display time counter for nixies is incremented every minute providing that the input power is present. Wrap and reset
 * of counter is dealt with in loop. There is also the twelve hours mode. Depending if the hours count is above or below 12, sets AM/PM flag.
 * The counter for hours is moved to a local variable. This is then manipulated depending on the selected mode. If the time is over 12,
 * then - 12 from number to convert to 12 hours format. If time is 0 hours, then variable = 12.
 * 
 * Finally the colon state is used to set its output on or off. */

void timer() {

    byte displayHours = counterHours;                                  // Variable used for 12 hour mode
  
    if (millis() - previousMillis >= TIMER_INTERVAL) {
      previousMillis += TIMER_INTERVAL;
      counterHalfSecond++;
      colonState = !colonState;
    }
    if (counterHalfSecond >= 2) {
      counterHalfSecond = 0;
      counterSeconds++;
    }
    if (counterSeconds >= 60) {                                        // If 60 seconds pass, wrap around and increment minutes
      counterSeconds = 0;
      counterMinutes++;
      if (!batteryBackup) {                                            // Only increment nixie run time counter if not on battery
        eepromCounter++;
      }
    }                                                                      
    digits[4] = (counterSeconds / 10) % 10;                            // Divide counter by 10, divide by 10, give remainder
    digits[5] = (counterSeconds % 10);                                 // Divide by 10, give remainder

    if (counterMinutes >= 60) {
      counterMinutes = 0;
      counterHours++;
    }
    digits[2] = (counterMinutes / 10) % 10;
    digits[3] = (counterMinutes % 10);
    if (counterHours >= 24) {
      counterHours = 0;
    }
    if (counterHours >= 12 && counterHours <= 23)                      // Dicide if it's AM or PM
    {             
      afternoon = true;
    } else {
      afternoon = false;
    }
    if (twelveHourMode && counterHours > 12) {                         // Convert 24 hour mode into 12 hour mode data
      displayHours -= 12;
    } else if (twelveHourMode && counterHours == 0) {
      displayHours = 12;
    } 
    digits[0] = (displayHours / 10) % 10;                              // Divide counter by 10, divide by 10, give remainder
    digits[1] = (displayHours % 10);                                   // Divide by 10, give remainder

    if (!batteryBackup && !standbyDone) {                              // Colon code moved here to keep it locked in perfectly
      digitalWrite(COLON_PIN, colonState ? LOW : HIGH);                // LOW : HIGH keeps colon on for 0 to 500ms, off 501 to 1000ms
    } else {
      digitalWriteFast(COLON_PIN, LOW);                                // Fail back to being off if staement is false
    }
}
// ======================================================= Further information ========================================================= //
// ------------------------------------------------------------------------------------------------------------------------------------- // 
// unsigned long currentButtonReferenceTime = millis();
//
//    grab current time at start of loop running, stick it in currentButtonReferenceTime
//
// minuteButtonState = !digitalRead(MINUTE_PIN);
//
//    each time loop function runs, the IO register is read. The inverse of its state is set into minuteButtonState
//
// if (minuteButtonState && !prevMinuteButtonState && (currentButtonReferenceTime - minuteButtonPrevPush > DEBOUNCE_DELAY)) {
//
//    if the button state is true and the previous button state is false and, when the loop ran it grabbed current time - time when button
//    was last pressed, is greater than 50, goto function.
//    eg. 400ms system time - 20ms system time at last push = 380ms > 50ms, then goto function
//    
// minuteButtonPrevPush = currentButtonReferenceTime;
//
//    After running function of button handler, the time of this latest button push is saved in minuteButtonPrevPush.
//
// } prevMinuteButtonState = minuteButtonState;
//
//    Regardless of if the function ran or not, the previous button state is recorded to whatever the current state is.
//    Every loop without button pressed will record that no button is pressed. This makes the previous logic only work once per loop.
// ------------------------------------------------------------------------------------------------------------------------------------- //
