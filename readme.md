AVR ATmega328p Driving 128x32 OLED (SSD1306) via I2C
====================================================

This is a proof of concept for driving an SSD1306 oled display

Library can be found here: http://en.radzio.dxp.pl/ks0108/

Pinout:
------------------------------------------------

128x32 OLED (i2c bit bang option)
GND --> GND
VCC --> 3.3V
SCK --> PB6
SDA --> PB7

StatusLED
PB0 --> LED --> 1k --> GND

5-Way Switch
1 --> PC4
2 --> PC0
3 --> PC1
4 --> GND
5 --> PC2
6 --> PC3

Remarks:
-------------------------------------
The bit-bang version of this i2c SHOULD NEVER BE USED. It's a quick and dirty implementation made just to ensure I understood how the protocol works. It works, but has not been optimized and doesn't have strict timing and doesn't return ACK/NACK for any use.
