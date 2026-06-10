# Power Usage Measurements

### Work In Progress

Preface:
(Standby refers to my code running in standby mode, running only code needed to track time. No display driving, no mode switching, display buffers idle, timer 2 disabled).

(Sleep idle refers to the AVR hardware. There are different sleep modes, each with differing levels of internal hardware shutdown. Idle is the lowest power mode that can be used to sleep and keep track of time with timer 1).

(LOD watchdog uses power when enabled. Trade off is worth it for clean low battery cutoff. MCU held in reset and EEPROM protected).

## Standby Testing

### 16MHz Clock

ATMEGA8A @16MHz:

| Standby | Sleep Idle | Analogue Comp Disabled | ADC Disabled | SPI, Timer0 & ADC Clock Disabled | LOD 1.8v Enable | Current @ 2.5v | Power Reduction |
| ------- | ---------- | ---------------------- | ------------ | -------------------------------- | --------------- | -------------- | --------------- |
|   YES   |    NO      |          NO            |      NO      |                NO                |        N/A      |     -------    | --------------- |
|   YES   |    YES     |          NO            |      NO      |                NO                |        N/A      |     6.774mA    | --------------- |
|   YES   |    YES     |          YES           |      NO      |                NO                |        N/A      |     -------    | --------------- |
|   YES   |    YES     |          YES           |      YES     |                NO                |        N/A      |     -------    | --------------- |
|   YES   |    YES     |          YES           |      YES     |                YES               |        N/A      |     -------    | --------------- |
|   YES   |    YES     |          YES           |      YES     |                YES               |        N/A      |     -------    | --------------- |
|   YES   |    YES     |          YES           |      YES     |                NO                |        N/A      |     -------    | --------------- |

ATMEGA328P @16MHz:

| Standby | Sleep Idle | Analogue Comp Disabled | ADC Disabled | SPI, Timer0 & ADC Clock Disabled | LOD 1.8v Enable | Current @ 2.5v | Power Reduction |
| ------- | ---------- | ---------------------- | ------------ | -------------------------------- | --------------- | -------------- | --------------- |
|   YES   |    NO      |          NO            |      NO      |                NO                |        NO       |     7.288mA    |        0%       |
|   YES   |    YES     |          NO            |      NO      |                NO                |        NO       |     5.678mA    |      28.35%     |
|   YES   |    YES     |          YES           |      NO      |                NO                |        NO       |     5.639mA    |      29.24%     |
|   YES   |    YES     |          YES           |      YES     |                NO                |        NO       |     4.949mA    |      47.26%     |
|   YES   |    YES     |          YES           |      YES     |                YES               |        NO       |     5.126mA    |      42.17%     |
|   YES   |    YES     |          YES           |      YES     |                YES               |        YES      |     -------    |      -------    |
|   YES   |    YES     |          YES           |      YES     |                NO                |        YES      |     5.539mA    |      31.57%     |

## 4MHz Clock

ATMEGA8A @4MHz:

| Standby | Sleep Idle | Analogue Comp Disabled | ADC Disabled | SPI, Timer0 & ADC Clock Disabled | LOD 1.8v Enable | Current @ 2.5v | Power Reduction |
| ------- | ---------- | ---------------------- | ------------ | -------------------------------- | --------------- | -------------- | --------------- |
|   YES   |    NO      |          NO            |      NO      |                NO                |        N/A      |     -------    | --------------- |
|   YES   |    YES     |          NO            |      NO      |                NO                |        N/A      |     -------    | --------------- |
|   YES   |    YES     |          YES           |      NO      |                NO                |        N/A      |     -------    | --------------- |
|   YES   |    YES     |          YES           |      YES     |                NO                |        N/A      |     -------    | --------------- |
|   YES   |    YES     |          YES           |      YES     |                YES               |        N/A      |     -------    | --------------- |
|   YES   |    YES     |          YES           |      YES     |                YES               |        N/A      |     -------    | --------------- |
|   YES   |    YES     |          YES           |      YES     |                NO                |        N/A      |     -------    | --------------- |

ATMEGA328P @4MHz:

| Standby | Sleep Idle | Analogue Comp Disabled | ADC Disabled | SPI, Timer0 & ADC Clock Disabled | LOD 1.8v Enable | Current @ 2.5v | Power Reduction |
| ------- | ---------- | ---------------------- | ------------ | -------------------------------- | --------------- | -------------- | --------------- |
|   YES   |    NO      |          NO            |      NO      |                NO                |        NO       |     -------    | --------------- |
|   YES   |    YES     |          NO            |      NO      |                NO                |        NO       |     -------    | --------------- |
|   YES   |    YES     |          YES           |      NO      |                NO                |        NO       |     -------    | --------------- |
|   YES   |    YES     |          YES           |      YES     |                NO                |        NO       |     -------    | --------------- |
|   YES   |    YES     |          YES           |      YES     |                YES               |        NO       |     -------    | --------------- |
|   YES   |    YES     |          YES           |      YES     |                YES               |        YES      |     -------    | --------------- |
|   YES   |    YES     |          YES           |      YES     |                NO                |        YES      |     -------    | --------------- |

## 2MHz Clock (Using CLKPR /8)

ATMEGA8A @2MHz:

| Standby | Sleep Idle | Analogue Comp Disabled | ADC Disabled | SPI, Timer0 & ADC Clock Disabled | LOD 1.8v Enable | Current @ 2.5v | Power Reduction |
| ------- | ---------- | ---------------------- | ------------ | -------------------------------- | --------------- | -------------- | --------------- |
|   YES   |    NO      |          NO            |      NO      |                NO                |        N/A      |     -------    | --------------- |
|   YES   |    YES     |          NO            |      NO      |                NO                |        N/A      |     -------    | --------------- |
|   YES   |    YES     |          YES           |      NO      |                NO                |        N/A      |     -------    | --------------- |
|   YES   |    YES     |          YES           |      YES     |                NO                |        N/A      |     -------    | --------------- |
|   YES   |    YES     |          YES           |      YES     |                YES               |        N/A      |     -------    | --------------- |
|   YES   |    YES     |          YES           |      YES     |                YES               |        N/A      |     -------    | --------------- |
|   YES   |    YES     |          YES           |      YES     |                NO                |        N/A      |     -------    | --------------- |

ATMEGA328P @2MHz:

| Standby | Sleep Idle | Analogue Comp Disabled | ADC Disabled | SPI, Timer0 & ADC Clock Disabled | LOD 1.8v Enable | Current @ 2.5v | Power Reduction |
| ------- | ---------- | ---------------------- | ------------ | -------------------------------- | --------------- | -------------- | --------------- |
|   YES   |    NO      |          NO            |      NO      |                NO                |        NO       |     -------    | --------------- |
|   YES   |    YES     |          NO            |      NO      |                NO                |        NO       |     -------    | --------------- |
|   YES   |    YES     |          YES           |      NO      |                NO                |        NO       |     -------    | --------------- |
|   YES   |    YES     |          YES           |      YES     |                NO                |        NO       |     -------    | --------------- |
|   YES   |    YES     |          YES           |      YES     |                YES               |        NO       |     -------    | --------------- |
|   YES   |    YES     |          YES           |      YES     |                YES               |        YES      |     4.071mA    | --------------- |
|   YES   |    YES     |          YES           |      YES     |                NO                |        YES      |     4.057mA    | --------------- |

## Normal Operation Testing

### ATMEGA8A:

| Clock Speed | Input Current | Input Voltage | Input Power |
| ----------- | ------------- | ------------- | ----------- |
|   @16MHz    |       -       |       -       |      -      |
|   @4MHz     |       -       |       -       |      -      |
|   @1MHz     |       -       |       -       |      -      |

### ATMEGA328P:

| Clock Speed | Input Current | Input Voltage | Input Power |
| ----------- | ------------- | ------------- | ----------- |
|   @16MHz    |       -       |       -       |      -      |
|   @4MHz     |       -       |       -       |      -      |
|   @1MHz     |       -       |       -       |      -      |
