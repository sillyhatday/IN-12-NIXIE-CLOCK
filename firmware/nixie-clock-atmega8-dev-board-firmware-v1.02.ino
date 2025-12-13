const int bcdPins[4] = {2, 3, 4, 5};                              // BCD output pins
const int digitPins[6] = {6, 7, 8, 9, 10, 11};                    // Cathode pins
const int colonPin = 12;                                          // decimal point pin assignment
const int resetPin = A0;
const int hourPin = A1;
const int minutePin = A2;

const unsigned int refreshInterval = 1000;                        // microseconds per digit (2ms)
const unsigned int timerInterval = 1000;
const unsigned int colonInterval = 500;
const unsigned int debounceDelay = 100;                           // time delay for ignoring button inputs

bool colonState = false;
bool prevColonState = false;
bool resetButtonState = false;
bool prevResetButtonState = false;
bool minuteButtonState = false;
bool prevMinuteButtonState = false;
bool hourButtonState = false;
bool prevHourButtonState = false;

byte digits[6] = {0, 0, 0, 0, 0, 0};                               // Digits to display
byte counterSeconds = 0;                                           // Current seconds
byte counterMinutes = 0;                                           // current minutes
byte counterHours = 0;

unsigned long previousMicros = 0;                                  // last update time of multiplex
unsigned long previousMillis = 0;                                  // last update time of seconds count
unsigned long previousMillisColon = 0;                              // last update time of DP
unsigned long minuteButtonTimeStamp = 0;                           // time when minute button was pressed
unsigned long hourButtonTimeStamp = 0;

byte currentDigit = 0;                                            // which digit is active in counting loop 0 to 3

const byte bcdCodes[10][4] = {                                    // Table for pin assignments vs number
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

// ---------------------------------------------------- One time setup

void setup() {
     for (int i = 0; i < 4; i++) {
        pinMode(bcdPins[i], OUTPUT);                                   // use bcdPins const values as pin numbers
        pinMode(digitPins[i], OUTPUT);                                 // use digitPins const values as pin numbers
        pinMode(colonPin, OUTPUT);                                     // decimal point between minutes and seconds
        pinMode(resetPin, INPUT_PULLUP);
        pinMode(minutePin, INPUT_PULLUP);
        digitalWrite(digitPins[i], LOW);                               // set all digitPins off
        digitalWrite(colonPin, LOW);
  }
}

// ---------------------------------------------------- hour button handler

void hourButtonHandler() {

  counterHours = (counterHours + 1) % 60;                          // increment safely, wrap at 24
  digits[0] = counterHours / 10;
  digits[1] = counterHours % 10;
}


// ---------------------------------------------------- minute button handler

void minuteButtonHandler() {
 
  counterMinutes = (counterMinutes + 1) % 60;                          // increment safely, wrap at 60
  digits[2] = counterMinutes / 10;
  digits[3] = counterMinutes % 10;
}

// ---------------------------------------------------- reset button handler

void resetHandler() {
    counterSeconds = 0;                                                // reset counters
    counterMinutes = 0;
    digits[0] = 0;                                                      // reset digits loaded into multiplex routine
    digits[1] = 0;                                                      
    digits[2] = 0;                                                       
    digits[3] = 0;
    digits[4] = 0;
    digits[5] = 0;                                                         
    previousMillis = millis();
    previousMillisColon = millis();
    colonState = false;                                                // forces colon into known state for restart
}

// ---------------------------------------------------- multiplexing routine

void multiplex() {

    if (micros() - previousMicros >= refreshInterval) {           // Check time passed since digit was activated
      previousMicros += refreshInterval;                          // drew code appplied here too from timer func
                                                                  // 
   {                                                               // Turn off all digits
      digitalWrite(digitPins[0], LOW);
      digitalWrite(digitPins[1], LOW);
      digitalWrite(digitPins[2], LOW);
      digitalWrite(digitPins[3], LOW);
      digitalWrite(digitPins[4], LOW);
      digitalWrite(digitPins[5], LOW);
   }

    int num = digits[currentDigit];                                 // Load BCD for current digit
    for (int i = 0; i < 4; i++) {
      digitalWrite(bcdPins[i], bcdCodes[num][i]);
   }

    bool colonOutputRequired = (!colonState && !resetButtonState);
    if (colonOutputRequired != prevColonState) {                                          // colonstate already worked out, much faster and in time
      digitalWrite(colonPin, colonOutputRequired ? HIGH : LOW);                           // removed digital write ever time function called
      prevColonState = colonOutputRequired;                                               // only write when needed, save cpu time
    }
   
    digitalWrite(digitPins[currentDigit], HIGH);                    // Turn display on for new digit
   
    currentDigit++;                                                 // Cycle to next digit
    if (currentDigit >= 6) currentDigit = 0;
   }
}
// ---------------------------------------------------- Count one second routine & assign digits

void timer() {
    if (millis() - previousMillis >= timerInterval) {         // if current time - previous time greater than timerinterval
    previousMillis += timerInterval;                          // timerinterval added to previousmillis
    counterSeconds++;                                         // loop execution time is unknown

    if (counterSeconds >= 60) {
      counterSeconds = 0;
      counterMinutes++;
    }
                                                                                // convert counter values
    digits[4] = (counterSeconds / 10) % 10;                                 // divide counter by 10, divide by 10, give remainder
    digits[5] = (counterSeconds % 10);                                      // divide by 10, give remainder

    if (counterMinutes >= 60) {
      counterMinutes = 0;
      counterHours++;
    }

    digits[2] = (counterMinutes / 10) % 10;                                 // divide counter by 10, divide by 10, give remainder
    digits[3] = (counterMinutes % 10);                                      // divide by 10, give remainder
    
    if (counterHours >= 24) {
      counterHours = 0;
    }

    digits[0] = (counterHours / 10) %10;
    digits[1] = (counterHours % 10);
  }
}

// --------------------------------------------------- Colon timer

void blinkColon() {                                                         // figure out colon state out of multiplex loop for timing
  unsigned long now = millis();                                             // with the state known before going into the loop
  if (now - previousMillisColon < colonInterval) return;
  previousMillisColon = now;
  colonState = !colonState;                                                  
}

// ---------------------------------------------------- Main program

void loop() {
  unsigned long minBounce = millis();
  unsigned long hourBounce = millis();
  
  multiplex();
  blinkColon();
  
    resetButtonState = !digitalRead(resetPin);
    if (resetButtonState && !prevMinuteButtonState) {                      // button logic not really needed for reset
      resetHandler();                                                     // doesn't matter if reset is triggered when held
  } prevResetButtonState = resetButtonState;                              // and it doesnt work anyway

    minuteButtonState = !digitalRead(minutePin);               // logic tracks button state and allows one function call per press
    if (minuteButtonState && !prevMinuteButtonState && (minBounce - minuteButtonTimeStamp > debounceDelay)) {
      minuteButtonHandler();
      minuteButtonTimeStamp = minBounce;      
  } prevMinuteButtonState = minuteButtonState;

      hourButtonState = !digitalRead(hourPin);               // logic tracks button state and allows one function call per press
    if (hourButtonState && !prevHourButtonState && (hourBounce - hourButtonTimeStamp > debounceDelay)) {
      hourButtonHandler();
      hourButtonTimeStamp = hourBounce;      
  } prevHourButtonState = hourButtonState;
    
  timer();

}
