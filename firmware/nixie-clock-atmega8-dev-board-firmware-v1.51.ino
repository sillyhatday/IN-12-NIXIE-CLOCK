// ------------------------------------------------------------------------------------------------------------------------------------- //
//                                                      ATmega8 clock firmware                                                           //
//                                                    Test software version 1.50                                                         //
//                                               Used in strip board protoype for ISP                                                    //
// ------------------------------------------------------------------------------------------------------------------------------------- //

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

// ******************************************************* Include/Define ************************************************************** //

#include <EEPROM.h>

// ********************************************************** Constants **************************************************************** //

const int BCD_PINS[4] =             {2, 3, 4, 5};                    // BCD output pins
const int DIGIT_PINS[6] =   {6, 7, 8, 9, 10, 11};                    // Cathode pins
const int COLON_PIN =                         12;                    // decimal point pin
//const int RESET_PIN =                         A0;                    // reset button input pin
const int HOUR_PIN =                          A1;                    // hour increment input pin
const int MINUTE_PIN =                        A2;                    // minute increment input pin
const int MODE_PIN =                          A4;                    // 24 or 12 hour mode selection
//const int PWR_PIN =                           A5;                    // pin to monitor if main power is removed
const int STATUS_PIN =                        A0;                    // Status button for showing EEPROM time data
const int RUN_TIME_HOURS =                     0;                    // EEPROM address for nixie tube run time
const int EEPROM_INITIALISED =                 4;
const unsigned int REFRESH_INTERVAL =       1000;                    // microseconds to display each digit (1mS)
const unsigned int TIMER_INTERVAL =         1000;                    // milliseconds for definition of a second (1000mS)
const unsigned int COLON_INTERVAL =          500;                    // milliseconds for colon off/on time (500mS)
const unsigned int DEBOUNCE_DELAY =          150;                    // time after button press to ignore inputs
const unsigned int SHOW_DATA_DURATION =     2000;                    // how long to show which hour mode we switched too
const byte TWO_DIGIT_MULTIPLEX_DUTY =          3;                    // Amount of fake loops vs real loops (on vs off time)
const byte BCD_CODES[10][4] = {                                      // Array for BCD value of numbers
  {0,0,0,0},                                // 0
  {1,0,0,0},                                // 1
  {0,1,0,0},                                // 2
  {1,1,0,0},                                // 3
  {0,0,1,0},                                // 4
  {1,0,1,0},                                // 5
  {0,1,1,0},                                // 6
  {1,1,1,0},                                // 7
  {0,0,0,1},                                // 8
  {1,0,0,1}                                 // 9
};
// ******************************************************* Debug constants ************************************************************* //

const int AFTERNOON_FLAG = A3;
//const int HOUR_MODE_FLAG =  13;

// ******************************************************** Button states ************************************************************** //

bool colonState =                 false;                                 
bool prevColonState =             false;                                 
bool resetButtonState =           false;                            
bool prevResetButtonState =       false;                                 
bool minuteButtonState =          false;                                 
bool prevMinuteButtonState =      false;                                 
bool hourButtonState =            false;                                 
bool prevHourButtonState =        false;                                 
bool modeButtonState =            false;								 
bool prevModeButtonState =        false;
bool runTimeButtonState =         false;
bool prevRunTimeButtonState =     false;

// ******************************************************* Internal flags ************************************************************** //

bool afternoon =                  false;                                 
bool twelveHourMode =             false;                                 
bool showHourMode =               false;                             // Show mode we are in or not?
bool showRunTime =                false;
bool batteryBackup =              false;                             // are we in battery backup mode?
bool standbyDone =                false;                             // has the standby routine been performed

// ******************************************************** Time counters ************************************************************** //

byte counterSeconds =                 0;                             // Seconds counter
byte counterMinutes =                 0;                             // Minutes counter
byte counterHours =                   0;                             // Hours counter

// *************************************************** Multiplexing & displays ********************************************************* //

byte currentDigit =                   0;                             // which display is active 0 to 5
byte digits[6] =     {0, 0, 0, 0, 0, 0};                             // Array for each 7seg display digit
byte twoDigitMultiplexCycle =         0;                             // How many times the function has run
byte twoDigitMultiplexIndex =         0;                             // Which digit is the function is on
byte eepromCounter =                  0;                             // Counter for incrementing hours independant of timer counter
unsigned long showHourModeStart =     0;                             // when did we start showing hour mode on display
unsigned long showRunTimeStart =      0;
unsigned long nixieRunTime =          0;                             // value read from EEPROM of nixie run time

// ******************************************************* Button Handling ************************************************************* //

unsigned long previousMicros =        0;                             // last update time of multiplex
unsigned long previousMillis =        0;                             // last update time of seconds count
unsigned long previousMillisColon =   0;                             // last update time of DP
unsigned long minuteButtonPrevPush =  0;                             // time when minute button was pressed last
unsigned long hourButtonPrevPush =    0;                             // time when hour button was pressed last
unsigned long resetButtonPrevPush =   0;                             // time when reset button was pressed last
unsigned long modeButtonPrevPush =    0;                             // time when mode button was pressed last
unsigned long runTimeButtonPrevPush = 0;

// ======================================================= One time setup ============================================================== //

void setup() {

     byte nixieStorageInit = EEPROM.read(EEPROM_INITIALISED);
  
     for (int i = 0; i < 4; i++) {
        pinMode(BCD_PINS[i], OUTPUT);                                // use BCD_PINS const values as pin numbers
        pinMode(DIGIT_PINS[i], OUTPUT);                              // use DIGIT_PINS const values as pin numbers
        pinMode(COLON_PIN, OUTPUT);                                  // decimal point between minutes and seconds
//        pinMode(RESET_PIN, INPUT_PULLUP);
        pinMode(MINUTE_PIN, INPUT_PULLUP);  
        pinMode(HOUR_PIN, INPUT_PULLUP);                           
        pinMode(MODE_PIN, INPUT_PULLUP);
        pinMode(13, OUTPUT);
//        pinMode(PWR_PIN, INPUT);                                     // No pull up. Externally pulled down. 5v input means HIGH
        pinMode(AFTERNOON_FLAG, OUTPUT);                           // debug only
        //pinMode(HOUR_MODE_FLAG, OUTPUT);                           // debug only
        digitalWrite(DIGIT_PINS[i], LOW);                            // set all DIGIT_PINS off
        digitalWrite(COLON_PIN, LOW);
        digitalWrite(13, HIGH);
     }

      if (nixieStorageInit != 0xAA) {                                // Initialise EEPROM address once ever, not each power up
        unsigned long zero = 0;
        EEPROM.put(RUN_TIME_HOURS, zero);                            // EEPROM.put write thes 4 bytes of the long starting at byte 0
        EEPROM.write(EEPROM_INITIALISED, 0xAA);                      // address 4 is out the way of the previous data
     }
}
// =================================================== Battery backup routine ========================================================== //

void standby() {

  if (batteryBackup == true) {
     digitalWrite(DIGIT_PINS[0], LOW);
     digitalWrite(DIGIT_PINS[1], LOW);
     digitalWrite(DIGIT_PINS[2], LOW);
     digitalWrite(DIGIT_PINS[3], LOW);
     digitalWrite(DIGIT_PINS[4], LOW);
     digitalWrite(DIGIT_PINS[5], LOW);
     digitalWrite(COLON_PIN, LOW);     
     standbyDone = true;
  }  
}
// ================================================= Show hour mode when toggled ======================================================= //

void hourModeDisplay() {

    if (showHourMode && (millis() - showHourModeStart > SHOW_DATA_DURATION)) {
        showHourMode = false;                                              // time expired, go back to normal display
}
    if (showHourMode) {
        if (twelveHourMode) {                                           // Display "12" on two digits, blank others
           digits[0] = 15;                                              // Send invalid data to the display driver to disable outputs
           digits[1] = 15;
           digits[2] = 1;
           digits[3] = 2;
           digits[4] = 15;
           digits[5] = 15;
        } else {                                                       // Display "24"
           digits[0] = 15;
           digits[1] = 15;
           digits[2] = 2;
           digits[3] = 4;
           digits[4] = 15;
           digits[5] = 15;
        }
    }    
}
// =================================================== Show nixie run time ============================================================= //

void nixieRunTimeDisplay() {

    if (showRunTime && (millis() - showRunTimeStart > SHOW_DATA_DURATION)) {
        showRunTime = false;                                              // time expired, go back to normal display
    }
    if (showRunTime) {
        eepromRead();
        unsigned long temp = nixieRunTime;
        for (int i = 5; i >= 0; i--) {
           digits[i] = temp % 10;                                            // get right-most digit
           temp /= 10;                                                       // remove that digit
        }
    }
}
// ==================================================== Development debug ============================================================== //

void debug() {

  digitalWrite(AFTERNOON_FLAG, afternoon);
  //digitalWrite(HOUR_MODE_FLAG, twelveHourMode);
  batteryBackup = false;  
}
// =================================================== Hour button handler ============================================================= //

void hourButtonHandler() {

  counterHours = (counterHours + 1) % 24;                              // increment safely, wrap at 24, else counts past 24 and must wait
  digits[0] = counterHours / 10;                                       // until timer function picks up for us, this does it immedietly
  digits[1] = counterHours % 10;
}
// ================================================== Minute button handler ============================================================ //

void minuteButtonHandler() {
 
  counterMinutes = (counterMinutes + 1) % 60;                          // increment safely, wrap at 60, else counts past 60 and must wait
  digits[2] = counterMinutes / 10;                                     // until timer function picks up for us, this does it immedietly
  digits[3] = counterMinutes % 10;
}
// ================================================== Reset button handler ============================================================= //

void resetHandler() {
  
    counterSeconds = 0;                                                // reset counters
    counterMinutes = 0;
    counterHours = 0;
    digits[0] = 0;                                                     // reset digits loaded into multiplex routine
    digits[1] = 0;                                                      
    digits[2] = 0;                                                       
    digits[3] = 0;
    digits[4] = 0;
    digits[5] = 0;                                                         
    previousMillis = millis();                                         // reset comparison time
    previousMillisColon = millis();
    colonState = false;                                                // forces colon into known state for restart
}
// ==================================================== Multiplex showMode ============================================================= //
// ------------------------------------------------------------------------------------------------------------------------------------- //
//
// This function is here to correct the brightness increase when only showing two digits. With 2 digits, each get a duty of (100/2)%
// This compared to (100/6)% causes a brightness increase. This function cycles through the unused digits as if they were being displayed.
// The time for time the loops where digits are not displayed, is not the same as for when they are being displayed. This is due to the
// digitalWrites not being done. The difference is minimal and I can't tell a difference. It works well enough. Written with the help of
// AI and me sanity checking what is going on as it couldn't get it quite right. I'm slowly learning and less dependant on AI from when
// I started this. The function could be a lot simpler if I had spare output pins to blank the CD4511. I do have spare pins, but I don't
// want to waste them on a such little used function as the hour mode display.
//
// ------------------------------------------------------------------------------------------------------------------------------------- //

void showDataMultiplex() {

    if (micros() - previousMicros >= REFRESH_INTERVAL) {                  // Check time passed since digit was activated
        previousMicros += REFRESH_INTERVAL; {                             // drew code appplied here too from timer func                                                         
            digitalWrite(DIGIT_PINS[0], LOW);
            digitalWrite(DIGIT_PINS[1], LOW);
            digitalWrite(DIGIT_PINS[2], LOW);
            digitalWrite(DIGIT_PINS[3], LOW);
            digitalWrite(DIGIT_PINS[4], LOW);
            digitalWrite(DIGIT_PINS[5], LOW);
        }

        int digitPin = 2  + twoDigitMultiplexIndex;                       // setup interger and assign value of 2 + twoDigitMultiplexIndex eg 2+0=2
        int num = digits[digitPin];                                       // setup interger num as number from byte selected from above 

        for (int j = 0; j < 4; j++) {                                     // Count through BCD outputs as in normal multiplex function
            digitalWrite(BCD_PINS[j], BCD_CODES[num][j]);
        }

        twoDigitMultiplexCycle++;                                         // Increment cycle count for this function
        if (twoDigitMultiplexCycle >= TWO_DIGIT_MULTIPLEX_DUTY) {         // If it's greater than the duty cycle const
            twoDigitMultiplexCycle = 0;                                   // then 0
        }
        if (twoDigitMultiplexCycle == 0) {                                // If we are at 0, then we can display some digits.
            digitalWrite(DIGIT_PINS[digitPin], HIGH);                     // Whenever on cycle 1 or 2, no digits displayed. keeps brightness correct
        }

        twoDigitMultiplexIndex++;                                         // increment the count for which digit to display
        if (twoDigitMultiplexIndex > 1) {                                 // if we are past 1, then reset
            twoDigitMultiplexIndex = 0;
        }
       
        bool colonOutputRequired = (!colonState && !resetButtonState);    // Checks if time up and change colon state
        if (colonOutputRequired != prevColonState) {                      // colonstate already worked out, much faster and in time
            digitalWrite(COLON_PIN, colonOutputRequired ? HIGH : LOW);    // only perform digitalwrite when needed, not every loop
            prevColonState = colonOutputRequired;                         // saves cpu time
        }
    }
}
// =================================================== multiplexing routine ============================================================ //

void multiplex() {
    
    if (micros() - previousMicros >= REFRESH_INTERVAL) {                  // Check time passed since digit was activated
        previousMicros += REFRESH_INTERVAL; {                             // drew code appplied here too from timer func                                                         
            digitalWrite(DIGIT_PINS[0], LOW);
            digitalWrite(DIGIT_PINS[1], LOW);
            digitalWrite(DIGIT_PINS[2], LOW);
            digitalWrite(DIGIT_PINS[3], LOW);
            digitalWrite(DIGIT_PINS[4], LOW);
            digitalWrite(DIGIT_PINS[5], LOW);
            }

       int num = digits[currentDigit];                                    // Load BCD for current digit
       for (int i = 0; i < 4; i++) {
            digitalWrite(BCD_PINS[i], BCD_CODES[num][i]);
            }

       bool colonOutputRequired = (!colonState && !resetButtonState);     // Checks if time up and change colon state
       if (colonOutputRequired != prevColonState) {                       // colonstate already worked out, much faster and in time
            digitalWrite(COLON_PIN, colonOutputRequired ? HIGH : LOW);    // only perform digitalwrite when needed, not every loop
            prevColonState = colonOutputRequired;                         // saves cpu time
            }
   
       digitalWrite(DIGIT_PINS[currentDigit], HIGH);                      // Turn display on for new digit
       currentDigit++;                                                    // Cycle to next digit
       
       if (currentDigit >= 6) { 
            currentDigit = 0;
            }   
    }
}
// ============================================ Count one second routine & assign digits =============================================== //

void timer() {

    byte displayHours = counterHours;
  
    if (millis() - previousMillis >= TIMER_INTERVAL) {                 // if current time - previous time greater than TIMER_INTERVAL
    previousMillis += TIMER_INTERVAL;                                  // TIMER_INTERVAL added to previousmillis
    counterSeconds++;                                                  // loop execution time is unknown
    }

    if (counterSeconds >= 60) {                                        // if 60 seconds pass, wrap around and increment minutes
      counterSeconds = 0;
      counterMinutes++;
      eepromCounter++;
    }
                                                                       // convert counter values
    digits[4] = (counterSeconds / 10) % 10;                            // divide counter by 10, divide by 10, give remainder
    digits[5] = (counterSeconds % 10);                                 // divide by 10, give remainder

    if (counterMinutes >= 60) {                                        // if 60 minutes pass, wrap around and increment hours
      counterMinutes = 0;
      counterHours++;                                                  // Add one to hours counter
    }

    digits[2] = (counterMinutes / 10) % 10;                            // divide counter by 10, divide by 10, give remainder
    digits[3] = (counterMinutes % 10);                                 // divide by 10, give remainder

    if (counterHours >= 24) {
      counterHours = 0;
    }

    if (counterHours >= 12 && counterHours <= 23) {
      afternoon = true;
    } else {
      afternoon = false;
    }

    if (twelveHourMode && counterHours > 12) {
      displayHours -= 12;
    } else if (twelveHourMode && counterHours == 0) {
      displayHours = 12;
    } 

    digits[0] = (displayHours / 10) % 10;                              // divide counter by 10, divide by 10, give remainder
    digits[1] = (displayHours % 10);                                   // divide by 10, give remainder
  
}
// ========================================================== EEPROM Writes ============================================================ //

void eepromWrite() {

  unsigned long eepromData = 0;

  EEPROM.get(RUN_TIME_HOURS, eepromData);
  eepromData++;
  EEPROM.put(RUN_TIME_HOURS, eepromData);
}
// =========================================================== EEPROM Reads ============================================================ //

void eepromRead() {

  EEPROM.get(RUN_TIME_HOURS, nixieRunTime);
}
// =========================================================== Colon timer ============================================================= //

void blinkColon() { 

  if (standbyDone == true) {
    standbyDone = false;
  }
                                                                        // figure out colon state out of multiplex loop for timing
  unsigned long now = millis();                                         // with the state known before going into the loop
  
  if (now - previousMillisColon < COLON_INTERVAL) return;
      previousMillisColon = now;
      colonState = !colonState;                                                  
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
			  break;                                                                                                // Breaks are required else
//		  case RESET_PIN:                                                                                         // system loops forever
//			  resetHandler();
//			  break;
      case STATUS_PIN:
        showRunTime = true;
        showRunTimeStart = millis();
        break;
	  }
    buttonPrevPush = currentReferenceTime;                                
  } 
  prevButtonState = buttonState;
}
// =========================================================== Main program ============================================================ //

void loop() {

//  batteryBackup = !digitalRead(PWR_PIN);                               // removed for development
  unsigned long currentButtonReferenceTime = millis();                 // log time into variable for comparing with last time button pressed

  if (standbyDone == false) {
    standby();
  }

  if (!batteryBackup) {
    if (!showHourMode) {
      multiplex();
    } else {
        showDataMultiplex();
      }
    blinkColon();
  }  
//  buttonHandler(RESET_PIN,  resetButtonState,  prevResetButtonState,  currentButtonReferenceTime, resetButtonPrevPush);
  buttonHandler(HOUR_PIN,   hourButtonState,   prevHourButtonState,   currentButtonReferenceTime, hourButtonPrevPush);
  buttonHandler(MINUTE_PIN, minuteButtonState, prevMinuteButtonState, currentButtonReferenceTime, minuteButtonPrevPush); 
  buttonHandler(MODE_PIN,   modeButtonState,   prevModeButtonState,   currentButtonReferenceTime, modeButtonPrevPush);
  buttonHandler(STATUS_PIN,  runTimeButtonState,  prevRunTimeButtonState,  currentButtonReferenceTime, runTimeButtonPrevPush);
  timer();

  if (eepromCounter >= 60) {
    eepromWrite();
    eepromCounter = 0;
  }
  
  hourModeDisplay();
  nixieRunTimeDisplay();
  debug();
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
