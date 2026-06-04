// -------------------------------------------------------------------------------------------- //
//                                     ATmega8 clock firmware                                   //
//                                   Test software version 2.1                                  //
//                              Used in strip board protoype for ISP                            //
// -------------------------------------------------------------------------------------------- //

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#include <EEPROM.h>
#include <avr/io.h>
#include <string.h>

// ************************************* Pin Assignments ************************************** //

//  AVR  PIN  Description
//  PD0 = 2 = Hours Tens Cathode
//  PD1 = 3  = Hours Units Cathode
//  PD2 = 4  = Mins Tens Cathode
//  PD3 = 5  = Mins Units Cathode
//  PD4 = 6  = Sec Tens Cathode
//  PD5 = 11 = Sec Units Cathode
//  PD6 = 12 = Colon Seperator
//  PD7 = 13 = NC

//  PB0 = 14 = BCD A
//  PB1 = 15 = BCD B
//  PB2 = 16 = BCD C
//  PB3 = 17 = BCD D
//  PB4 = 18 = NC
//  PB5 = 19 = NC
//  PB6 = 9  = Crystal In
//  PB7 = 10 = Crystal In

//  PC0 = 23 = Nixie Run Time Button
//  PC1 = 24 = Hour Increment Button
//  PC2 = 25 = Minute Increment Button
//  PC3 = 26 = 180v On/Off
//  PC4 = 27 = Hour Mode Button
//  PC5 = 28 = Power Monitor Input
//  PC6 = 1  = Reset Input

// ************************************** Pin Assignments ************************************* //

const uint8_t BCD_PINS[4] =          {0, 1, 2, 3};                  // PORTB Outputs
const uint8_t DIGIT_PINS[6] =  {0, 1, 2, 3, 4, 5};                  // PORTD Outputs
const uint8_t COLON_PIN =                       6;                  // PD6
const uint8_t STATUS_PIN =                      0;                  // PC0 For showing EEPROM time data
const uint8_t HOUR_PIN =                        1;                  // PC1 Hour increment
const uint8_t MINUTE_PIN =                      2;                  // PC2 Minute increment
//const uint8_t HV_POWER =                        3;                  // PC3 Turns on/off high voltage
const uint8_t MODE_PIN =                        4;                  // PC4 12/24 hour mode
//const uint8_t PWR_PIN =                         5;                  // PC5 monitor standby switch

// *************************************** Enumerations *************************************** //

enum DisplayMode : uint8_t {
    TIME_MODE,
    HOUR_MODE,
    RUN_TIME_MODE,
    POISON_MODE
};
// ************************************** EEPROM Adresses ************************************* //

const uint8_t RUN_TIME_HOURS =                  4;                  // EEPROM address for nixie tube run time, 4 bytes 0 - 3
const uint8_t EEPROM_INITIALISED =             20;                  // EEPROM address to check if EEPROM is initialised

// ****************************************** Timing ****************************************** //

const uint8_t DEBOUNCE_DELAY =                150;                  // Time after button press to ignore inputs
const uint16_t POISON_DWELL =                  50;
const uint8_t POISON_REPEAT =                   4;

// *************************************** Character Data ************************************* //

const uint8_t BLANK =                          10;                  // Used for clear reference to CD4511 invalid data
const uint8_t BCD_CODES[11] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0F};

// **************************************** Display Modes ************************************ //

volatile DisplayMode requestedMode =    TIME_MODE;
DisplayMode currentMode =               TIME_MODE;

// ****************************************** Bit Masks *************************************** //

const uint8_t CATHODES_OFF =           0b11000000;

// **************************************** Button States ************************************* //

uint8_t minuteButtonState =                 false;
uint8_t prevMinuteButtonState =             false;
uint8_t hourButtonState =                   false;
uint8_t prevHourButtonState =               false;
uint8_t modeButtonState =                   false;
uint8_t prevModeButtonState =               false;
uint8_t statusButtonState =                 false;
uint8_t prevStatusButtonState =             false;

// **************************************** Button Debounce *********************************** //

uint32_t minuteButtonPrevPush =                 0;                             // Time when minute button was pressed last
uint32_t hourButtonPrevPush =                   0;                             // Time when hour button was pressed last
uint32_t modeButtonPrevPush =                   0;
uint32_t statusButtonPrevPush =                 0;
uint32_t currentButtonReferenceTime =           0;

// **************************************** Internal Flags ************************************ //

volatile uint8_t runTimer =                 false;
uint8_t colonState =                        false;
uint8_t multiplexFlag =                     false;
uint8_t twelveHourModeFlag =                false;
uint8_t modeSelectBlock =                   false;
uint8_t eepromWriteReady =                  false;
uint8_t poisonRunFlag =                     false;

// *************************************** EEPROM Variables *********************************** //

uint32_t eepromData =                           0;

// ******************************************* Counters *************************************** //

volatile uint8_t currentDigit =                 0;
volatile uint32_t milliSecondCounter =          0;
uint8_t systemTimer =                           0;
uint8_t counterSeconds =                        0;
uint8_t counterMinutes =                        0;
uint8_t counterHours =                          0;
uint8_t counterHoursCopy =                      0;
uint8_t poisonCycleDigit =                      0;
uint32_t poisonCycleLast =                      0;
uint8_t poisonCycleCount =                      0;

// **************************************** Display Buffers *********************************** //

uint8_t multiplexBuffer[6] =   {0, 0, 0, 0, 0, 0};
uint8_t timerBuffer[6] =       {0, 0, 0, 0, 0, 0};
uint8_t runTimeBuffer[6] =     {0, 0, 0, 0, 0, 0};
uint8_t hourModeBuffer[6] =    {0, 0, 0, 0, 0, 0};
uint8_t poisonBuffer[6] =      {0, 0, 0, 0, 0, 0};

// =========================================== Functions ====================================== //

void setup(void);
void loop(void);
void buttonHandler(uint8_t avrPin, uint8_t &buttonState, uint8_t &prevButtonState, uint32_t currentReferenceTime, uint32_t &buttonPrevPush);
void modeSelect(uint8_t logic);
void minuteButtonHandler(void);
void hourButtonHandler(void);
void displayBuffer(uint8_t displayMode);
void multiplex(void);
void eepromUpdate(void);
void twelveHourModeConversion(void);
void escapeToTimeMode(void);
void hourModeText(void);
void totalRunTimeText(void);
void cathodePoison(void);
void timer(void);
void timerBufferUpdate(void);

// ============================================= Setup ======================================== //

void setup() {

  set_sleep_mode(SLEEP_MODE_IDLE);
  // BCD Pins
  DDRB = 0b00001111;
  PORTB = 0b00110000;

  // Cathode Pins
  DDRD = 0b01111111;
  PORTD = 0b10000000;

  // Buttons/Feedback Pins
  DDRC = 0b00001000;                                              // Output on PC3
  PORTC = 0b00010111;                                             // No pullup on PC5

  cli();
  // Timer1 CTC - 16MHz
  TCCR1A = 0;
  TCCR1B = (1 << WGM12)|(1 << CS12);
  OCR1A = 62499;
  TIMSK |= (1 << OCIE1A);
/*
  // Timer1 CTC - 1MHz
  TCCR1A = 0;
  TCCR1B = (1 << WGM12)|(1 << CS12);
  OCR1A = 3906;                                             // wrong
  TIMSK |= (1 << OCIE1A);
*/  
  // Timer2 CTC - 16MHz
  TCCR2 |= (1 << WGM21)|(1 << CS22);
  TCNT2 = 0;
  OCR2 = 249;
  TIMSK |= (1 << OCIE2);

/*  // Timer2 CTC - 1MHz
  cli();
  TCCR2 |= (1 << WGM21)|(1 << CS21);                              // CTC Mode - prescaler 8
  TCNT2 = 0;
  OCR2 = 62;                                                      // 500ms for colon flash
  TIMSK |= (1 << OCIE2);
  sei();
*/
  byte nixieStorageInit = EEPROM.read(EEPROM_INITIALISED);        // Read initialised data from EEPROM

  if (nixieStorageInit != 0xAA) {                                 // Check if EEPROM was previously initialised.
    EEPROM.update(EEPROM_INITIALISED, 0xAA);                      // Address 4 is out the way of the previous data
    unsigned long zero = 0;                                       // EEPROM.write will only write one byte not all 4 as below
    EEPROM.put(RUN_TIME_HOURS, zero);
  }
  sei();
}
// ============================================ 1Hz Timer ===================================== //

ISR(TIMER1_COMPA_vect) {
  timer();
}
// ========================================== Multiplex Timer ================================== //

ISR(TIMER2_COMP_vect) {
  milliSecondCounter++;
  multiplex();
}
// =============================================== Loop ======================================== //

void loop() {

  currentButtonReferenceTime = millis();               // Log time for comparing with last time button pressed

  if (currentMode == HOUR_MODE) {
    hourModeText();
  }

  if (currentMode == RUN_TIME_MODE) {
    totalRunTimeText();
  }
  
  if (eepromWriteReady == true) {
    eepromUpdate();
    eepromWriteReady = false;
    poisonRunFlag = true;
    modeSelect(POISON_MODE);
  }

  if (poisonRunFlag == true) {
    cathodePoison();
  }

  displayBuffer(currentMode);
  
  buttonHandler(HOUR_PIN,   hourButtonState,    prevHourButtonState,    currentButtonReferenceTime, hourButtonPrevPush);
  buttonHandler(MINUTE_PIN, minuteButtonState,  prevMinuteButtonState,  currentButtonReferenceTime, minuteButtonPrevPush);  
  buttonHandler(MODE_PIN,   modeButtonState,    prevModeButtonState,    currentButtonReferenceTime, modeButtonPrevPush);
  buttonHandler(STATUS_PIN, statusButtonState,  prevStatusButtonState,  currentButtonReferenceTime, statusButtonPrevPush);
}
// ======================================== Button Handler ==================================== //

void buttonHandler(uint8_t avrPin, uint8_t &buttonState, uint8_t &prevButtonState,
                   uint32_t currentReferenceTime, uint32_t &buttonPrevPush) {

  buttonState = !(PINC & (1 << avrPin));

  if (buttonState != prevButtonState) {

    if (currentReferenceTime - buttonPrevPush > DEBOUNCE_DELAY) {

      if (buttonState == true) {   // only trigger on press, not release
        switch(avrPin) {
          case MINUTE_PIN:
            minuteButtonHandler();
            break;

          case HOUR_PIN:
            hourButtonHandler();
            break;

          case MODE_PIN:
            requestedMode = HOUR_MODE;
            modeSelect(requestedMode);
            break;

          case STATUS_PIN:
            requestedMode = RUN_TIME_MODE;
            modeSelect(requestedMode);
            break;
        }
      }

      buttonPrevPush = currentReferenceTime;
    }
  }

  prevButtonState = buttonState;
}
// ========================================== Mode Select ===================================== //
/* Put stuff here that you want to run once when the new mode is activated. Repeated code goes
 *  in its own function.
 */

void modeSelect(DisplayMode requestedMode) {

  switch (requestedMode) {

    case TIME_MODE:
      currentMode = requestedMode;
      break;
      
    case HOUR_MODE:
      if (modeSelectBlock) {
        break;
      }
      currentMode = requestedMode;
      twelveHourModeFlag = !twelveHourModeFlag;
      modeSelectBlock = true;
      systemTimer = 0;
      break;
      
    case RUN_TIME_MODE:
      if (modeSelectBlock) {
        break;
      }
      currentMode = requestedMode;
      modeSelectBlock = true;
      systemTimer = 0;
      EEPROM.get(RUN_TIME_HOURS, eepromData);
      break;
      
    case POISON_MODE:
      if (modeSelectBlock) {
        break;
      }
      currentMode = requestedMode;
      cathodePoison();
      break;
  }
}
// ========================================= Display Buffer =================================== //

void displayBuffer(DisplayMode currentMode) {
  
  switch (currentMode) {

    case TIME_MODE:
      cli();
      memcpy(multiplexBuffer, timerBuffer, 6);    // Copy 6 bytes from timer to multi
      sei();
      break;
      
    case HOUR_MODE:
      cli();
      memcpy(multiplexBuffer, hourModeBuffer, 6); // Look at using pointers instead of copying mem
      sei();
      break;
      
    case RUN_TIME_MODE:
      cli();
      memcpy(multiplexBuffer, runTimeBuffer, 6);
      sei();
      break;
      
    case POISON_MODE:
      cli();
      memcpy(multiplexBuffer, poisonBuffer, 6);
      sei();
      break;
  }
}
// ========================================= EEPROM Update ==================================== //

void eepromUpdate() {
  
    EEPROM.get(RUN_TIME_HOURS, eepromData);
    eepromData++;
    EEPROM.update(RUN_TIME_HOURS, eepromData);
}
// ===================================== Minute button handler ================================ //

void minuteButtonHandler() {
 
    counterMinutes = (counterMinutes + 1) % 60;
    timerBufferUpdate();
}
// ====================================== Hour button handler ================================= //

void hourButtonHandler() {

    counterHours = (counterHours + 1) % 24;
    counterHoursCopy = counterHours;
    
    if (twelveHourModeFlag) {
      twelveHourModeConversion();
    }
    timerBufferUpdate();
}
// ====================================== Escape To Time Mode ================================= //

void escapeToTimeMode() {

    modeSelectBlock = false;
    requestedMode = TIME_MODE;
    modeSelect(requestedMode);
}
// ========================================= Run Time Mode ==================================== //

void totalRunTimeText() {

  if (systemTimer < 5) {                                // eg 123456 hours run
    runTimeBuffer[0] = (eepromData / 100000) % 10;      // 1
    runTimeBuffer[1] = (eepromData / 10000) % 10;       // 2
    runTimeBuffer[2] = (eepromData / 1000) % 10;        // 3
    runTimeBuffer[3] = (eepromData / 100) % 10;         // 4
    runTimeBuffer[4] = (eepromData / 10) % 10;          // 5
    runTimeBuffer[5] = eepromData % 10;                 // 6
  } else {
  escapeToTimeMode();
  }
}
// ========================================= Hour Mode Text =================================== //

void hourModeText() {

  if (systemTimer < 2) {
    if (twelveHourModeFlag){
      hourModeBuffer[0] = BLANK;
      hourModeBuffer[1] = BLANK;
      hourModeBuffer[2] = 1;
      hourModeBuffer[3] = 2;
      hourModeBuffer[4] = BLANK;
      hourModeBuffer[5] = BLANK;
    } else {
      hourModeBuffer[0] = BLANK;
      hourModeBuffer[1] = BLANK;
      hourModeBuffer[2] = 2;
      hourModeBuffer[3] = 4;
      hourModeBuffer[4] = BLANK;
      hourModeBuffer[5] = BLANK;
    }
  } else {
  escapeToTimeMode();
  }
}
// ======================================= Cathode Poisoning ================================== //

void cathodePoison() {

  if ((milliSecondCounter - poisonCycleLast) >= POISON_DWELL) {
    for (byte i = 0; i < 6; i++) {
      poisonBuffer[i] = poisonCycleDigit;
    }
    poisonCycleDigit++;
      
    if (poisonCycleDigit > 9) {
      poisonCycleDigit = 0;
      poisonCycleCount++;
    }

    if (poisonCycleCount >= POISON_REPEAT) {
      poisonCycleCount = 0;
      poisonCycleDigit = 0;
      poisonRunFlag = false;                 // Mode request stops this displaying, not setting
      escapeToTimeMode();                    // flag to false keeps this running in background!
    }
    poisonCycleLast = milliSecondCounter;
  }
}
// =========================================== Multiplex ====================================== //

void multiplex()
{
  uint8_t num = 0;
  uint8_t bcd = 0;

  PORTD &= CATHODES_OFF;                       // Turn off all cathodes DONT CLOBBER COLON
  _delay_us(10);
  PORTB = BCD_CODES[BLANK];
  num = multiplexBuffer[currentDigit];         // Find number to display in digit array   
  bcd = BCD_CODES[num];                        // Find character data for current number

  PORTB = bcd;                                 // Set outputs to character data, !!bit mask due to XTAL inputs
  _delay_us(10);
  PORTD |= (1 << (5 - currentDigit));
  currentDigit++;
  
  if (currentDigit >= 6) {                     // If next digit is 6 or more, wrap to 0
    currentDigit = 0;
  }
}
// ======================================= 12 Hour Conversion ================================= //

void twelveHourModeConversion() {

  if (counterHours > 12) {                     // Convert 24 hour mode into 12 hour mode data
    counterHoursCopy -= 12;
  } else if (counterHours == 0) {
    counterHoursCopy = 12;
  }
}
// ============================================= Timer ======================================== //

void timer() {

  counterSeconds++;

  if (counterSeconds >= 60) {
    counterSeconds = 0;
    counterMinutes++;
  }

  if (counterMinutes >= 60) {
    counterMinutes = 0;
    counterHours++;
    eepromWriteReady = true;
  }

  if (counterHours >= 24) {
    counterHours = 0;
  }

  counterHoursCopy = counterHours;

  if (twelveHourModeFlag) {
    twelveHourModeConversion();
  }

  timerBufferUpdate();

  if (currentMode != TIME_MODE) {
    systemTimer++;
  }
}
// ========================================= Timer Buffer Update =============================== //

void timerBufferUpdate() {

  timerBuffer[0] = (counterHoursCopy / 10) % 10;             // Find tens
  timerBuffer[1] = (counterHoursCopy % 10);                  // Find units
  timerBuffer[2] = (counterMinutes / 10) % 10;
  timerBuffer[3] = (counterMinutes % 10);
  timerBuffer[4] = (counterSeconds / 10) % 10;
  timerBuffer[5] = (counterSeconds % 10);
}
