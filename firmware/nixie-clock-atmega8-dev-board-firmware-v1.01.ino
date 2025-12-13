const int bcdPins[4] = {2, 3, 4, 5};                              // BCD output pins
const int digitPins[4] = {8, 9, 10, 11};                          // Cathode pins
const int colonPin = 7;                                         // decimal point pin assignment
const int resetPin = 6;
const int secInc = 13;
const int minutePin = 12;

bool colonState = false;
bool resetButtonState = false;
bool minuteButtonState = false;

const unsigned int refreshInterval = 2000;                        // microseconds per digit (2ms)

const unsigned long timerInterval = 1000;
const unsigned long colonInterval = 500;

byte digits[4] = {0, 0, 0, 0};                                     // Digits to display
byte counterSeconds = 0;                                           // Current seconds
byte counterMinutes = 0;                                           // current minutes
byte currentDigit = 0;                                            // which digit is active in counting loop 0 to 3

unsigned long previousMicros = 0;                                 // last update time of multiplex
unsigned long previousMillis = 0;                                 // last update time of seconds count
unsigned long previousMillisColon = 0;                            // last update time of DP
unsigned long debounceDelay = 50;                                 // time delay for ignoring button inputs
unsigned long buttonTimeStamp = millis();                         // 

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

// ---------------------------------------------------- minute button handler

void minuteButtonHandler() {
  counterMinutes = (counterMinutes + 1) % 60;                          // increment safely, wrap at 60
  digits[0] = counterMinutes / 10;
  digits[1] = counterMinutes % 10;

  
}

// ---------------------------------------------------- reset button handler

void resetHandler() {
    counterSeconds = 0;                                                // reset counters
    counterMinutes = 0;
    digits[0] = 0;                                                      // reset digits loaded into multiplex routine
    digits[1] = 0;                                                      
    digits[2] = 0;                                                       
    digits[3] = 0;                                                         
    previousMillis = millis();                             // reset seconds counter, count whole second, not start halfway                             
    previousMillisColon = millis();                         // same as above
    colonState = false;                                                  
}

// ---------------------------------------------------- multiplexing routine

void multiplex() {
  unsigned long now = micros();

   if (now - previousMicros >= refreshInterval) {                   // Check time passed since digit was activated
    previousMicros = now;

   {                                                               // Turn off all digits
      digitalWrite(digitPins[0], LOW);
      digitalWrite(digitPins[1], LOW);
      digitalWrite(digitPins[2], LOW);
      digitalWrite(digitPins[3], LOW);
   }

    int num = digits[currentDigit];                                 // Load BCD for current digit
    for (int i = 0; i < 4; i++) {
      digitalWrite(bcdPins[i], bcdCodes[num][i]);
   }

    if (!colonState && !resetButtonState) {                     // state already worked out, much faster and in time
    digitalWrite(colonPin, HIGH);
   } else {
    digitalWrite(colonPin, LOW);
   }
   
    digitalWrite(digitPins[currentDigit], HIGH);                    // Turn display on for new digit
   
    currentDigit++;                                                 // Cycle to next digit
    if (currentDigit >= 4) currentDigit = 0;
   }
}
// ---------------------------------------------------- Count one second routine & assign digits

void timer() {
  unsigned long now = millis();
  if (now - previousMillis >= timerInterval) {
    previousMillis = now;
    counterSeconds++;

    if (counterSeconds >= 60) {
      counterSeconds = 0;
      counterMinutes++;
    }

    if (counterMinutes >= 60) {
      counterMinutes = 0;
    }
                                                                            // convert counter values
    digits[2] = (counterSeconds / 10) % 10;                                 // divide counter by 10, divide by 10, give remainder
    digits[3] = (counterSeconds % 10);                                      // divide by 10, give remainder
    
    digits[0] = (counterMinutes / 10) % 10;                                 // divide counter by 10, divide by 10, give remainder
    digits[1] = (counterMinutes % 10);                                      // divide by 10, give remainder
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
  multiplex();
  blinkColon();
  
    resetButtonState = !digitalRead(resetPin);
      if (resetButtonState) {
         resetHandler();
  }    

    minuteButtonState = !digitalRead(minutePin);
      if (minuteButtonState) {
        minuteButtonHandler();
      }
  
  timer();

}
