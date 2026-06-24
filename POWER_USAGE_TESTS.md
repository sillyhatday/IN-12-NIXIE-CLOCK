# Standby Power Use

>[!IMPORTANT]
>This is a work in progress

>[!NOTE]
>Standby refers to my codes standby mode, running only code needed to track time. <br>
>Sleep idle refers to the AVR hardware sleep.
>LOD watchdog uses power when enabled. This will protect the EEPROM data.

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

## 2MHz Clock

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

ATMEGA328P @2MHz (Using CLKPR /8):

| Standby | Sleep Idle | Analogue Comp Disabled | ADC Disabled | SPI, Timer0 & ADC Clock Disabled | LOD 1.8v Enable | Current @ 2.5v | Power Reduction |
| ------- | ---------- | ---------------------- | ------------ | -------------------------------- | --------------- | -------------- | --------------- |
|   YES   |    NO      |          NO            |      NO      |                NO                |        NO       |     -------    | --------------- |
|   YES   |    YES     |          NO            |      NO      |                NO                |        NO       |     -------    | --------------- |
|   YES   |    YES     |          YES           |      NO      |                NO                |        NO       |     -------    | --------------- |
|   YES   |    YES     |          YES           |      YES     |                NO                |        NO       |     -------    | --------------- |
|   YES   |    YES     |          YES           |      YES     |                YES               |        NO       |     -------    | --------------- |
|   YES   |    YES     |          YES           |      YES     |                YES               |        YES      |     4.071mA    | --------------- |
|   YES   |    YES     |          YES           |      YES     |                NO                |        YES      |     4.057mA    | --------------- |

## Normal Operation

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
