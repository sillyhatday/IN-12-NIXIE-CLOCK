// -------------------------------------------------------------------------------------- //
//                                ATmega8 clock firmware                                  //
//                               Test software version 1.2                                //
//                          Used in strip board protoype for ISP                          //
// -------------------------------------------------------------------------------------- //

bool colonState = false;                                             // Tracks the state of the colon seperator
bool prevColonState = false;                                         // Tracks the state colon was previously
bool resetButtonState = false;                                       // Tracks the state of the button
bool prevResetButtonState = false;                                   // Tracks the state button was previously
bool minuteButtonState = false;                                      // Tracks the state of the button
bool prevMinuteButtonState = false;                                  // Tracks the state button was previously
bool hourButtonState = false;                                        // Tracks the state of the button
bool prevHourButtonState = false;                                    // Tracks the state button was previously
bool modeButtonState = false;
bool prevModeButtonState = false;
bool afternoon = false;                                              // track the state of the hour mode setting
bool twelveHourMode = false;                                         // What hour format are we in

byte digits[6] = {0, 0, 0, 0, 0, 0};                                 // Digits to display
byte counterSeconds = 0;                                             // Current seconds
byte counterMinutes = 0;                                             // current minutes
byte counterHours = 0;                                               // current hours
byte currentDigit = 0;                                               // Digit active - 0 to 5

const byte bcdCodes[10][4] = {                                       // Array for BCD value of numbers
  {0,0,0,0}, // 0
  {1,0,0,0}, // 1
  {0,1,0,0}, // 2
  {1,1,0,0}, // 3
  {0,0,1,0}, // 4
  {1,0,1,0}, // 5
  {0,1,1,0}, // 6
  {1,1,1,0}, // 7
  {0,0,0,1}, // 8
  {1,0,0,1}  // 9
                              };

const int bcdPins[4] = {2, 3, 4, 5};                                 // BCD output pins
const int digitPins[6] = {6, 7, 8, 9, 10, 11};                       // Cathode pins
const int colonPin = 12;                                             // decimal point pin
const int resetPin = A3;                                             // reset button input pin
const int hourPin = A1;                                              // hour increment input pin
const int minutePin = A2;                                            // minute increment input pin
const int modePin = A0;                                             // 12 or 12 hour mode selection

const unsigned int refreshInterval = 1000;                           // microseconds to display each digit (1mS)
const unsigned int timerInterval = 1000;                             // milliseconds for definition of a second (1000mS)
const unsigned int colonInterval = 500;                              // milliseconds for colon off/on time (500mS)
const unsigned int debounceDelay = 150;                              // time after button press to ignore inputs

unsigned long previousMicros = 0;                                    // last update time of multiplex
unsigned long previousMillis = 0;                                    // last update time of seconds count
unsigned long previousMillisColon = 0;                               // last update time of DP
unsigned long minuteButtonPrevPush = 0;                              // time when minute button was pressed last
unsigned long hourButtonPrevPush = 0;                                // time when hour button was pressed last
unsigned long modeButtonPrevPush = 0;                                // time when mode button was pressed last

// ---------------------------------------------------- One time setup ------------------------------------------------------------------ //

void setup() {
  
     for (int i = 0; i < 4; i++) {
        pinMode(bcdPins[i], OUTPUT);                                  // use bcdPins const values as pin numbers
        pinMode(digitPins[i], OUTPUT);                                // use digitPins const values as pin numbers
        pinMode(colonPin, OUTPUT);                                    // decimal point between minutes and seconds
        pinMode(resetPin, INPUT_PULLUP);
        pinMode(minutePin, INPUT_PULLUP);
        pinMode(modePin, INPUT_PULLUP);
        digitalWrite(digitPins[i], LOW);                              // set all digitPins off
        digitalWrite(colonPin, LOW);
  }
}

// ---------------------------------------------------- hour button handler ------------------------------------------------------------- //

void hourButtonHandler() {

  counterHours = (counterHours + 1) % 24;                              // increment safely, wrap at 24, else counts past 24 and must wait
  digits[0] = counterHours / 10;                                       // until timer function picks up for us, this does it immedietly
  digits[1] = counterHours % 10;
}


// ---------------------------------------------------- minute button handler ----------------------------------------------------------- //

void minuteButtonHandler() {
 
  counterMinutes = (counterMinutes + 1) % 60;                          // increment safely, wrap at 60, else counts past 60 and must wait
  digits[2] = counterMinutes / 10;                                     // until timer function picks up for us, this does it immedietly
  digits[3] = counterMinutes % 10;
}

// ---------------------------------------------------- reset button handler ------------------------------------------------------------ //

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

// --------------------------------------------------- determine afternoon or morning ----------------------------------------------------------- //

void hourMode() {

    twelveHourMode = !twelveHourMode;

    if (counterHours >= 12 && twelveHourMode == true) {                // if past 12 hours and we are in 12 hour mode
      afternoon = true;                                                // then it is afternoon
      counterHours -= 12;                                              // subtract 12 to bring into 12 hour format
      }
      else if (counterHours < 12 && twelveHourMode == true) {          // if 12 hasn't passed and we are in 12 hour mode
        afternoon = false;                                             // it's still morning
      }
//      else if (afternoon == true && twelveHourMode == false) {          // if its past 12 and we are in 24 hour mode
//        counterHours += 12;                                            // add 12 to hours count  
//      }
     
      digits[0] = (counterHours / 10) % 10;                            // update multiplex now to change screen quickly
      digits[1] = (counterHours % 10);
    
}

// ---------------------------------------------------- multiplexing routine ------------------------------------------------------------ //

void multiplex() {

    if (micros() - previousMicros >= refreshInterval) {                // Check time passed since digit was activated
      previousMicros += refreshInterval;                               // drew code appplied here too from timer func
                                                                   
   {                                                                   // Turn off all digits
      digitalWrite(digitPins[0], LOW);
      digitalWrite(digitPins[1], LOW);
      digitalWrite(digitPins[2], LOW);
      digitalWrite(digitPins[3], LOW);
      digitalWrite(digitPins[4], LOW);
      digitalWrite(digitPins[5], LOW);
   }

    int num = digits[currentDigit];                                    // Load BCD for current digit
    for (int i = 0; i < 4; i++) {
      digitalWrite(bcdPins[i], bcdCodes[num][i]);
   }

    bool colonOutputRequired = (!colonState && !resetButtonState);     // Checks if time up and change colon state
    if (colonOutputRequired != prevColonState) {                       // colonstate already worked out, much faster and in time
      digitalWrite(colonPin, colonOutputRequired ? HIGH : LOW);        // only perform digitalwrite when needed, not every loop
      prevColonState = colonOutputRequired;                            // saves cpu time
    }
   
    digitalWrite(digitPins[currentDigit], HIGH);                       // Turn display on for new digit
   
    currentDigit++;                                                    // Cycle to next digit
    if (currentDigit >= 6) currentDigit = 0;
   }
}
// ---------------------------------------------------- Count one second routine & assign digits ---------------------------------------- //

void timer() {
  
    if (millis() - previousMillis >= timerInterval) {                  // if current time - previous time greater than timerinterval
    previousMillis += timerInterval;                                   // timerinterval added to previousmillis
    counterSeconds++;                                                  // loop execution time is unknown
    }

    if (counterSeconds >= 60) {                                        // if 60 seconds pass, wrap around and increment minutes
      counterSeconds = 0;
      counterMinutes++;
    }
                                                                       // convert counter values
    digits[4] = (counterSeconds / 10) % 10;                            // divide counter by 10, divide by 10, give remainder
    digits[5] = (counterSeconds % 10);                                 // divide by 10, give remainder

    if (counterMinutes >= 60) {                                        // if 60 minutes pass, wrap around and increment hours
      counterMinutes = 0;
      counterHours++;
    }

    digits[2] = (counterMinutes / 10) % 10;                            // divide counter by 10, divide by 10, give remainder
    digits[3] = (counterMinutes % 10);                                 // divide by 10, give remainder

    if (twelveHourMode == false && (counterHours >= 24)) {//|| (afternoon == false && counterHours == 12))) {               // Check if we are in 12 or 24 hour mode & if past 23 hours
          counterHours = 0;                                            // if so wrap around
    }

    if (twelveHourMode == true && counterHours > 12) {                 // Check if we are in 12 or 24 hour mode and if past 12 hours
//          afternoon = !afternoon;                                      // toggle afternoon flag when warrping around
          counterHours = 1;                                            // wrap around to 1 not 0
    }      
//    else if (twelveHourMode == true && counterHours == 0) {            // Only called if switching to 12 hour mode at 0 hours
//      counterHours = 12;
//      afternoon = false;
//    }

    digits[0] = (counterHours / 10) %10;                               // divide counter by 10, divide by 10, give remainder
    digits[1] = (counterHours % 10);                                   // divide by 10, give remainder
  
}

// --------------------------------------------------- Colon timer ---------------------------------------------------------------------- //

void blinkColon() { 
                                                                        // figure out colon state out of multiplex loop for timing
  unsigned long now = millis();                                         // with the state known before going into the loop
  if (now - previousMillisColon < colonInterval) return;
  previousMillisColon = now;
  colonState = !colonState;                                                  
}

// ---------------------------------------------------- Main program -------------------------------------------------------------------- //

void loop() {
  
  unsigned long currentButtonReferenceTime = millis();                  // log time into variable for comparing with last time button pressed
  //unsigned long hourBounce = millis();                                // useless tracking every button, no buttons work before debounce over
                                                                        // saves tiny cpu time
  multiplex();
  blinkColon();
  
    resetButtonState = !digitalRead(resetPin);
    if (resetButtonState && !prevResetButtonState) {
      resetHandler();
  } prevResetButtonState = resetButtonState;    

    modeButtonState = !digitalRead(modePin);
    if (modeButtonState && !prevModeButtonState && (currentButtonReferenceTime - modeButtonPrevPush > debounceDelay)) {
      hourMode();
      modeButtonPrevPush = currentButtonReferenceTime;
  } prevModeButtonState = modeButtonState;
    
    minuteButtonState = !digitalRead(minutePin);
    if (minuteButtonState && !prevMinuteButtonState && (currentButtonReferenceTime - minuteButtonPrevPush > debounceDelay)) {
      minuteButtonHandler();
      minuteButtonPrevPush = currentButtonReferenceTime;                                
  } prevMinuteButtonState = minuteButtonState;

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ // 
// unsigned long currentButtonReferenceTime = millis();
//
//    grab current time at start of loop running, stick it in currentButtonReferenceTime
//
// minuteButtonState = !digitalRead(minutePin);
//
//    each time loop function runs, the IO register is read. The inverse of its state is set into minuteButtonState
//
// if (minuteButtonState && !prevMinuteButtonState && (currentButtonReferenceTime - minuteButtonPrevPush > debounceDelay)) {
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
// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //

      hourButtonState = !digitalRead(hourPin);                             
    if (hourButtonState && !prevHourButtonState && (currentButtonReferenceTime - hourButtonPrevPush > debounceDelay)) {
      hourButtonHandler();
      hourButtonPrevPush = currentButtonReferenceTime;      
  } prevHourButtonState = hourButtonState;
    
  timer();

}
