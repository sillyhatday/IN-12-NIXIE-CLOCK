// ------------------------------------------------------------------------------------------------------------------------------------- //
//                                                      ATmega8 clock firmware                                                           //
//                                                    Test software version 1.54                                                         //
//                                               Used in strip board protoype for ISP                                                    //
// ------------------------------------------------------------------------------------------------------------------------------------- //

// Todo List:
// Why is standby all fucked up? Is it a hardware issue? Swapping chip fixed it. Tested old chip with simple sketch and it works?!
// Look at getting digit blanking to work with the multiplex function and the new blanking output pin
// Reduce amount of buttons. Use one button for multi function. Double click, hold, etc.

// ******************************************************* Include/Define ************************************************************** //

#include <EEPROM.h>

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
//  PB5 = 19 = 13 = Debugging Hour Mode Flag
//  PB6 = 9  = -- = Crystal In
//  PB7 = 10 = -- = Crystal In
//  PC0 = 23 = A0 = Reset Button
//  PC1 = 24 = A1 = Hour Increment Button
//  PC2 = 25 = A2 = Minute Increment Button
//  PC3 = 26 = A3 = Debugging Afternoon Flag
//  PC4 = 27 = A4 = Hour Mode Button
//  PC5 = 28 = A5 = Power Monitor Input
//  PC6 = 1  = -- = Reset Input
//

// ********************************************************** Constants **************************************************************** //

const int BCD_PINS[4] =               {2, 3, 4, 5};                  // BCD output pins
const int DIGIT_PINS[6] =    {6, 7, 8, 9, 10, 11,};                  // Digit cathode pins
const int COLON_PIN =                           12;                  // Decimal point pin
const int DIGIT_BLANKING =                      13;                  // Output enable for CD4511 display driver
//const int RESET_PIN =                           A0;                  // reset button input pin
const int STATUS_PIN =                          A0;                  // DEBUG - Status button for showing EEPROM time data
const int HOUR_PIN =                            A1;                  // Hour increment input pin
const int MINUTE_PIN =                          A2;                  // Minute increment input pin
const int AFTERNOON_FLAG =                      A3;                  // AM/PM indicator
const int MODE_PIN =                            A4;                  // 24 or 12 hour mode selection
const int PWR_PIN =                             A5;                  // Pin to monitor if main power is removed
const int RUN_TIME_HOURS =                       0;                  // EEPROM address for nixie tube run time, 4 bytes 0 - 3
const int EEPROM_INITIALISED =                   4;                  // EEPROM address to check if EEPROM is initialised
const unsigned int REFRESH_INTERVAL =         1000;                  // Microseconds to display each digit (1mS)
const unsigned int TIMER_INTERVAL =            500;                  // Milliseconds for definition of a half second
const unsigned int DEBOUNCE_DELAY =            150;                  // Time after button press to ignore inputs
const unsigned int SHOW_DATA_DURATION =       2000;                  // How long to show which hour mode we switched too
const byte BCD_CODES[10][4] = {                                      // Array for BCD value of numbers
  {0,0,0,0},                                                         // 0
  {1,0,0,0},                                                         // 1
  {0,1,0,0},                                                         // 2
  {1,1,0,0},                                                         // 3
  {0,0,1,0},                                                         // 4
  {1,0,1,0},                                                         // 5
  {0,1,1,0},                                                         // 6
  {1,1,1,0},                                                         // 7
  {0,0,0,1},                                                         // 8
  {1,0,0,1}                                                          // 9
};
// ******************************************************** Button states ************************************************************** //

bool colonState =                 false;                             // Colon pin on or off            
bool prevColonState =             false;                             // Sanity checking of colon state
bool resetButtonState =           false;                             // Is reset pressed
bool prevResetButtonState =       false;                             // Used for debouncing of reset button
bool minuteButtonState =          false;                             // Is minute increment pressed
bool prevMinuteButtonState =      false;                             // Used for debouncing of minute button
bool hourButtonState =            false;                             // Is hour increment pressed
bool prevHourButtonState =        false;                             // Used for debouncing of hour button
bool modeButtonState =            false;								             // Is mode button pressed
bool prevModeButtonState =        false;                             // Used for debouncing of mode button
bool runTimeButtonState =         false;                             // DEBUG
bool prevRunTimeButtonState =     false;                             // DEBUG

// ******************************************************* Internal flags ************************************************************** //

bool afternoon =                  false;                             // Is it AM or PM   
bool twelveHourMode =             false;                             // Which hour mode is active
bool showHourMode =               false;                             // Display hour mode or not
bool showRunTime =                false;                             // Display run time or not
bool batteryBackup =              false;                             // Main power present or not
bool standbyDone =                false;                             // Tracking to only run standby once
//volatile bool colonToggleFlag =   false;

// ******************************************************** Time counters ************************************************************** //

byte counterHalfSecond =              0;
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
unsigned long resetButtonPrevPush =   0;                             // Time when reset button was pressed last
unsigned long modeButtonPrevPush =    0;                             // Time when mode button was pressed last
unsigned long runTimeButtonPrevPush = 0;                             // DEBUG Time when run time button was pressed last

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
     }                                                               // Setup
//        pinMode(RESET_PIN,   INPUT_PULLUP);
        pinMode(HOUR_PIN,    INPUT_PULLUP);
        pinMode(MINUTE_PIN,  INPUT_PULLUP);                             
        pinMode(MODE_PIN,    INPUT_PULLUP);
        pinMode(STATUS_PIN,  INPUT_PULLUP);
        pinMode(PWR_PIN,            INPUT);
        pinMode(COLON_PIN,         OUTPUT);
        pinMode(DIGIT_BLANKING,    OUTPUT);
        pinMode(AFTERNOON_FLAG,    OUTPUT);
        
        digitalWrite(COLON_PIN,       LOW);
        digitalWrite(DIGIT_BLANKING, HIGH);                          // DEBUG

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
    standbyDone = false;
  }

  if (!standbyDone) {                                                // If on battery power, skip displaying data
    multiplex();
  }
  
  if (eepromCounter >= 60) {
    eepromWrite();
    eepromCounter = 0;
  }

//  buttonHandler(RESET_PIN,  resetButtonState,  prevResetButtonState,  currentButtonReferenceTime, resetButtonPrevPush);
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
// ------------------------------------------------------------------------------------------------------------------------------------- //
//
// The first time I have done a reusable function like this. Please bully me. Function handles button presses, the function prevents button
// bounce from giving multiple inputs, it also prevents holding of buttons causing further inputs.
//
// Values are passed into this function. When this is done, the software will run with a copy of those variables. This causes the fucnction
// to break. The & symbol in the function header tells the system to not create local variable. This way the variables are updated system wide.
// There are also uses for these values in other functions called from here, so that allows those functions to work also.
//
// Break is needed else the switch will loop forever. Break is the get out of loop thingy.
//
// ------------------------------------------------------------------------------------------------------------------------------------- //

void buttonHandler(int avrPin, bool &buttonState, bool &prevButtonState, unsigned long currentReferenceTime, unsigned long &buttonPrevPush) {

   buttonState = !digitalRead(avrPin);
   
   if (buttonState && !prevButtonState && (currentReferenceTime - buttonPrevPush > DEBOUNCE_DELAY)) {        // If true, go to switch.
    switch(avrPin) {                                                                                         // value of avrPin selects case.
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
//      case RESET_PIN:
//        resetHandler();
//        break;
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

void debug() {

  digitalWrite(AFTERNOON_FLAG, afternoon);
//  digitalWrite(HOUR_MODE_FLAG, twelveHourMode);
//  batteryBackup = false;
//  standbyDone = false;
//  Serial.print(digitalRead(PWR_PIN));
}
// =========================================================== EEPROM Reads ============================================================ //

void eepromRead() {

  EEPROM.get(RUN_TIME_HOURS, nixieRunTime);
}
// ========================================================== EEPROM Writes ============================================================ //

void eepromWrite() {

  unsigned long eepromData = 0;

  EEPROM.get(RUN_TIME_HOURS, eepromData);
  eepromData++;
  EEPROM.put(RUN_TIME_HOURS, eepromData);
}
// =================================================== Hour button handler ============================================================= //
// Increment safely, wrap at 24, else counts past 24 and must wait until timer function picks up for us, this does it immedietly

void hourButtonHandler() {

  counterHours = (counterHours + 1) % 24;
  digits[0] = counterHours / 10;
  digits[1] = counterHours % 10;
}
// ================================================= Show hour mode when toggled ======================================================= //
// While showHourMode is true, this function is run. It sets the digit registers each time, overriding the timer function as it is
// run after the timer function has set the digits.

void hourModeDisplay() {

    if (showHourMode && (millis() - showHourModeStart > SHOW_DATA_DURATION)) {
        showHourMode = false;                                         // Time expired, go back to normal display
    }
    if (showHourMode) {
        if (twelveHourMode) {                                         // Display "12"
           digits[0] = 0;
           digits[1] = 0;
           digits[2] = 1;
           digits[3] = 2;
           digits[4] = 0;
           digits[5] = 0;
        } else {                                                      // Display "24"
           digits[0] = 0;
           digits[1] = 0;
           digits[2] = 2;
           digits[3] = 4;
           digits[4] = 0;
           digits[5] = 0;
        }
    }
}
// ================================================== Minute button handler ============================================================ //
// increment safely, wrap at 60, else counts past 60 and must wait until timer function picks up for us, this does it immedietly

void minuteButtonHandler() {
 
  counterMinutes = (counterMinutes + 1) % 60;
  digits[2] = counterMinutes / 10;
  digits[3] = counterMinutes % 10;
}
// =================================================== Multiplexing Routine ============================================================ //
// The main display function. Changes here must be calculated. Small changes can screw up the display.
// Only do digitalWrite when needed to save more time.
// Code compensates for being called late. Should other functions take longer before multiplex is called, it will account and compensate.
// Check if time to run. 550uS, too early. Run other functions. Check time. 600uS too late. Next ready time is now 400uS, not 500uS
// from now. Otherwise error adds up slowly.

void multiplex() {
    
    if (micros() - previousMicros >= REFRESH_INTERVAL) {                  // Check time passed since digit was activated
       previousMicros += REFRESH_INTERVAL;                                // Drews input here. Code compensates for being called late.
       
          for (int i = 0; i < 6; i++)  {                                             
            digitalWrite(DIGIT_PINS[i], LOW);                             // Turn off all digit cathodes
          }
          
          int num = digits[currentDigit];                                 // Load BCD for current digit
       
          if (!showHourMode) {                                            // If we aren't show the hour mode
            digitalWrite(DIGIT_BLANKING, HIGH);                           // Set CD4511 outputs ON
            for (int i = 0; i < 4; i++) {                                 // Set BCD outputs, repeat 4 times
                digitalWrite(BCD_PINS[i], BCD_CODES[num][i]);
            } 
          }
          else if (showHourMode) {                                        // If hour mode is to be displayed
            if (currentDigit == 2 || currentDigit == 3) {                 // Only switch cathode on for middle digits
              digitalWrite(DIGIT_BLANKING, HIGH);                         // Set CD4511 outputs ON
              for (int i = 0; i < 4; i++) {                               // Set BCD outputs, repeat 4 times
                digitalWrite(BCD_PINS[i], BCD_CODES[num][i]);
              }
            } else {
              digitalWrite(DIGIT_BLANKING, LOW);                          // If we are not on digits 2 or 3
              for (int i = 0; i < 4; i++) {                               // Turn of each BCD output
                digitalWrite(BCD_PINS[i], LOW);                           // Not needed, but helps to use up time
              }
            }
          }

          digitalWrite(DIGIT_PINS[currentDigit], HIGH);                    // Turn display on for new digit
          
          currentDigit++;                                                  // Cycle to next digit
       
          if (currentDigit >= 6) {                                         // If next digit is 6 or more, wrap to 0
            currentDigit = 0;
          }   
    }
}
// =================================================== Show nixie run time ============================================================= //
// Notes on FOR loop to myself. eg, temp = 123456. Step 1. temp % 10 = 6. i = 6. digits[6]. Look up 6 in digits table for BCD.
// /= is same as x = x / 10. temp /= 12345. 123456 divide 10 = 12345, in interger division drops the decimal place. Repeat until
// no digits are left over. 

void nixieRunTimeDisplay() {

    if (showRunTime && (millis() - showRunTimeStart > SHOW_DATA_DURATION)) {
        showRunTime = false;                                               // Check time to know when to stop displaying
    }
    if (showRunTime) {                                                     // If hour mode to be displayed
        eepromRead();                                                      // Jump to EEPROM function
        unsigned long temp = nixieRunTime;                                 // Temporary variable = eeprom data read previously
        for (int i = 5; i >= 0; i--) {                                     // Repeat 6 times counting backwards
           digits[i] = temp % 10;                                          // Divide temp by ten, assign remainder to digit[i]
           temp /= 10;                                                     // Same as x = x / 10. Decimal place dropped.
        }
    }
}
// ================================================== Reset button handler ============================================================= //
// There is a delay until timer() updates multiplex().  Resetting counters will have that delay. Update digits now to avoid that.

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
// Turn off all digit cathode pins. Turn off colon output if it is on. Mark function as performed.

void standby() {

   for (int i = 0; i < 6; i++) {                                       // Set all digit cathode OFF
     digitalWrite(DIGIT_PINS[i], LOW);
   }
   digitalWrite(COLON_PIN, LOW);                                       // Turn off colon pin
   standbyDone = true;                                                 // Report standby is done
}
// ============================================ Count one second routine & assign digits =============================================== //
// Code compensates for being called late. Should other functions take longer before multiplex is called, it will account and compensate.
// Check if time to run. 950uS, too early. Run other functions. Check time. 1100uS too late. Next ready time is now 900uS, not 1000uS
// from now. Otherwise error adds up slowly. eg, 1010mS + 1004mS + 1020mS = 3034mS error adds up. The code below accounts for that.
// eg, called at 1010mS, next ready time = 990mS. Each further late call is compensated. Only the first error is not fixed.

void timer() {

    byte displayHours = counterHours;                                  // Variable used for 12 hour mode
  
    if (millis() - previousMillis >= TIMER_INTERVAL) {                 // If current time - previous time greater than TIMER_INTERVAL
      previousMillis += TIMER_INTERVAL;                                // TIMER_INTERVAL added to previousmillis. Account for error.
      counterHalfSecond++;
      colonState = !colonState;
    }
    if (counterHalfSecond >= 2) {                                      // 2 counts of half a second make one full second
      counterHalfSecond = 0;                                           // Increment seconds counter and reset half second count
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

    if (counterMinutes >= 60) {                                        // If 60 minutes pass, wrap around and increment hours
      counterMinutes = 0;
      counterHours++;                                                  // Add one to hours counter
    }
    digits[2] = (counterMinutes / 10) % 10;                            // Divide counter by 10, divide by 10, give remainder
    digits[3] = (counterMinutes % 10);                                 // Divide by 10, give remainder

    if (counterHours >= 24) {                                          // If 24 hours pass, wrap around to 0
      counterHours = 0;
    }
    if (counterHours >= 12 && counterHours <= 23) {                    // Decide if it's AM/PM
      afternoon = true;
    } else {
      afternoon = false;
    }
    if (twelveHourMode && counterHours > 12) {                         // Work out what to display in 12 hour mode
      displayHours -= 12;                                              // Over 12, minus 12
    } else if (twelveHourMode && counterHours == 0) {                  // If zero display 12
      displayHours = 12;
    } 
    digits[0] = (displayHours / 10) % 10;                              // Divide counter by 10, divide by 10, give remainder
    digits[1] = (displayHours % 10);                                   // Divide by 10, give remainder

    if (!batteryBackup && !standbyDone) {                              // Colon code moved here to keep it locked in perfectly
      digitalWrite(COLON_PIN, colonState ? LOW : HIGH);                // LOW : HIGH keeps colon on for 0 to 500ms, off 501 to 1000ms
    } else {
      digitalWrite(COLON_PIN, LOW);                                    // Fail back to being off if staement is false
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
