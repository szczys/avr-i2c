#define F_CPU 8000000L
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include "font5x8.h"
#include "i2c-hardware.h"

void Delay_ms(int cnt);

#define UP  (1<<PC2)
#define DN  (1<<PC0)
#define LT  (1<<PC1)
#define RT  (1<<PD4)
#define SW  (1<<PC3)
#define BUTDDR  DDRC
#define BUTPORT PORTC
#define BUTPIN  PINC
#define BUTMASK (UP | DN | LT | SW)
#define BUTREAD ((~BUTPIN) & BUTMASK) | ((~PIND) & RT)

const uint8_t PROGMEM wavetable[128] = {
  0x10,0x10,0x11,0x12,0x13,0x13,0x14,0x15,
  0x15,0x16,0x17,0x17,0x18,0x19,0x19,0x1a,
  0x1a,0x1b,0x1b,0x1c,0x1c,0x1d,0x1d,0x1e,
  0x1e,0x1e,0x1e,0x1f,0x1f,0x1f,0x1f,0x1f,
  0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1e,0x1e,
  0x1e,0x1e,0x1d,0x1d,0x1c,0x1c,0x1b,0x1b,
  0x1a,0x1a,0x19,0x19,0x18,0x17,0x17,0x16,
  0x15,0x15,0x14,0x13,0x13,0x12,0x11,0x10,
  0x10,0xf,0xe,0xd,0xc,0xc,0xb,0xa,
  0xa,0x9,0x8,0x8,0x7,0x6,0x6,0x5,
  0x5,0x4,0x4,0x3,0x3,0x2,0x2,0x1,
  0x1,0x1,0x1,0x0,0x0,0x0,0x0,0x0,
  0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x1,
  0x1,0x1,0x2,0x2,0x3,0x3,0x4,0x4,
  0x5,0x5,0x6,0x6,0x7,0x8,0x8,0x9,
  0xa,0xa,0xb,0xc,0xc,0xd,0xe,0xf,
};

const uint16_t galaga[16] = {
	0b0000000100000000,
	0b0000000100000000,
	0b0000000100000000,
	0b0000001110000000,
	0b0000001110000000,
	0b0001001110010000,
	0b0001001110010000,
	0b0001011111010000,
	0b1001111011110010,
	0b1001110001110010,
	0b1001110101110010,
	0b1011111111111010,
	0b1111111111111110,
	0b1110111111101110,
	0b1100110101100110,
	0b1000000100000010
};

uint8_t frameBuffer[512];

void clearBuffer(void) {
  for (uint16_t i=0; i<512; i++) {
    frameBuffer[i] = 0x00;
  }
}

void sineToBuffer(uint8_t offset) {
  clearBuffer();
  for (uint8_t i=0; i<128; i++) {
    uint8_t idx = (i+offset)%128;
    uint8_t data = pgm_read_byte(&(wavetable[idx]));
    frameBuffer[i+((data/8)*128)] = 1<<(data % 8);
  }
}

void showBuffer(void) {
  //Set column start and end address
  i2c_cmd(0x21);
  i2c_cmd(0x00);
  i2c_cmd(0xFF);
  //Set page start and end address
  i2c_cmd(0x22);
  i2c_cmd(0x00);
  i2c_cmd(0x03);

  i2c_start();
  i2c_writebyte(0x78);
  i2c_writebyte(0x40);
  for (uint16_t i=0; i<512; i++) {
    i2c_writebyte(frameBuffer[i]);
  }
  i2c_stop();    
}

void bitXY(uint8_t x, uint8_t y, uint16_t val) {
  //Set or clear a bit in the frameBuffer
  uint16_t idx = ((y/8)*128)+x;
  uint8_t bitmask = (1<<(y%8));
  if (val) frameBuffer[idx] |= bitmask;
  else frameBuffer[idx] &= ~bitmask;
}

void putGalaga(uint8_t x, uint8_t y) {
  for (int8_t col=0; col<16; col++) {
    for (uint8_t row=0; row<16; row++) {
      bitXY(x+col,row+y,galaga[15-col] & (1<<row));
    }
  }
}

void Delay_ms(int cnt) {
	while (cnt-->0) {
		_delay_ms(1);
	}
}

void init_io(void) {
  //Set up status LED
  DDRB |= (1<<PB0);
  PORTB &= ~(1<<PB0);
  
  //Set up button input
  BUTDDR &= ~BUTMASK;
  BUTPORT |= BUTMASK;
  //Hack in one pin from a different register
  //Because PC4 and PC5 are used by hardware I2C and
  //PC6 is the reset pin  
  DDRD &= ~RT;
  PORTD |= RT;
}

void init_oled(void) {
  //Initialization for a 128x32 OLED (SSD1306)

  //Set MUX Ratio
  i2c_cmd(0xA8);
  i2c_cmd(0x1F);  //32rows-1 = 0x1F

  //Set Display Offset
  i2c_cmd(0xD3);
  i2c_cmd(0x00);

  //Set Display Start Line
  i2c_cmd(0x40);  //start line 0

  //Set Segment re-map
  i2c_cmd(0xA0);

  //Set COM Output Scan Direction
  i2c_cmd(0xC0);

  //Set COM Pins hardware configuration
  i2c_cmd(0xDA);
  i2c_cmd(0x02); 

  //Set Contrast Control
  i2c_cmd(0x81);
  i2c_cmd(0x7F);

  //Disable Entire Display On
  i2c_cmd(0xA4);

  //Set Normal Display
  i2c_cmd(0xA6);

  //Set horizontal addressing mode
  i2c_cmd(0x20);
  i2c_cmd(0x00);

  //Set column start and end address
  i2c_cmd(0x21);
  i2c_cmd(0x00);
  i2c_cmd(0xFF);

  //Set page start and end address
  i2c_cmd(0x22);
  i2c_cmd(0x00);
  i2c_cmd(0x03);

  //Set Osc Frequency
  i2c_cmd(0xD5);
  i2c_cmd(0x80); 

  //Enable charge pump regulator
  i2c_cmd(0x8D);
  i2c_cmd(0x14); 

  //Display On
  i2c_cmd(0xAF); 
}

void fill_screen(uint8_t pattern) {
  i2c_start();
  i2c_writebyte(0x78);
  i2c_writebyte(0x40);
  for (uint16_t col=0; col<(128*32); col++) {
    i2c_writebyte(pattern);
  }
  i2c_stop();
}

void oled_putc(uint8_t letter) {
  i2c_start();
  i2c_writebyte(0x78);
  i2c_writebyte(0x40);
  for (uint8_t i=0; i<5; i++) {
    i2c_writebyte(font5x8[letter-32][i]);
  }
  i2c_writebyte(0x00);
  i2c_stop();
}

void oled_puts(char * string) {
  while (*string > 0) {
    oled_putc(*string);
    ++string;
  }
}

int main(void)
{
  Delay_ms(1);
  init_i2c();
  init_oled();
  init_io();
  fill_screen(0x00);
  oled_puts("Press any button...");

  uint8_t inputs = BUTREAD;
  uint8_t lastbut = inputs;

  while(1)
  {
    inputs = BUTREAD;
    if (lastbut != inputs) {
      lastbut = inputs;
      uint8_t poordebounce = 0;
      //Set column start and end address
      i2c_cmd(0x21);
      i2c_cmd(0x00);
      i2c_cmd(0xFF);
      //Set page start and end address
      i2c_cmd(0x22);
      i2c_cmd(0x00);
      i2c_cmd(0x03);
      if (inputs & SW) {
        PORTB |= (1<<PB0);
        //fill_screen(0x00);
        //oled_puts("Stumpy");
        for (uint8_t i=0; i<112; i++) {
          clearBuffer();
          putGalaga(i,8);
          showBuffer();
          Delay_ms(5);
        }
        ++poordebounce;
      }
      if (inputs & LT) {
        PORTB |= (1<<PB0);
        fill_screen(0x00);
        uint8_t counter = 10;
        while(counter > 0) {
          oled_puts("Asa ");
          counter = counter - 1;
          oled_putc(counter+'0');
        }
        //i2c_cmd(0x2F);
        ++poordebounce;
      }
      if (inputs & UP) {
        PORTB |= (1<<PB0);
        fill_screen(0x00);
        oled_puts("Light shared pin");
        PORTC &= ~DN; //Make sure this is never a high voltage when pin set as an output        
        DDRC |= DN;        
        
        Delay_ms(100);
       
        ++poordebounce;
      }
      if (inputs & DN) {
        PORTB |= (1<<PB0);
        fill_screen(0x00);
        oled_puts("Merrick");
        ++poordebounce;
      }
      if (inputs & RT) {
        PORTB |= (1<<PB0);
        clearBuffer();
        while(1) {
          for (uint8_t i=0; i<128; i++) {
            sineToBuffer(i);
            showBuffer();
          }
          break;
        }
        ++poordebounce;
      }
      if (poordebounce) {
        //Delay so we don't read multiple presses
        Delay_ms(40);
        PORTB &= ~(1<<PB0);
        DDRC &= ~DN;
        PORTC |= DN;
      }
    }
    
    /*
    Delay_ms(1000);
    //i2c_cmd(0xAE);
    PINB |= 1<<PB0;
    Delay_ms(1000);
    //i2c_cmd(0xAF);
    PINB |= 1<<PB0;
    */
  }
}
