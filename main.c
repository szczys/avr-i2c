#define F_CPU 1000000L
#include <avr/io.h>
#include <util/delay.h>
#include "font5x8.h"

void Delay_ms(int cnt);
void i2c_start(void);
void i2c_stop(void);
void i2c_writebit(uint8_t val);
void i2c_writebyte(uint8_t byte);
void init_i2c(void);


#define I2C_PORT  PORTB
#define I2C_DDR   DDRB
#define SCK   1<<PB6
#define SDA   1<<PB7

#define I2C_SDA_LO    I2C_DDR |= SDA
#define I2C_SCK_LO    I2C_DDR |= SCK
#define I2C_SDA_HI    I2C_DDR &= ~(SDA)
#define I2C_SCK_HI    I2C_DDR &= ~(SCK)
#define I2C_WAIT      _delay_us(1)

#define UP  (1<<PC2)
#define DN  (1<<PC0)
#define LT  (1<<PC1)
#define RT  (1<<PC4)
#define SW  (1<<PC3)
#define BUTDDR  DDRC
#define BUTPORT PORTC
#define BUTPIN  PINC
#define BUTMASK (UP | DN | LT | RT | SW)

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
}

void i2c_start(void) {
  I2C_SCK_HI;
  I2C_SDA_HI;
  I2C_SDA_LO;
  I2C_WAIT;
  I2C_SCK_LO;
  I2C_WAIT;
}

void i2c_stop(void) {
  I2C_SDA_LO;
  I2C_WAIT;
  I2C_SCK_HI;
  I2C_WAIT;
  I2C_SDA_HI;
  I2C_WAIT;
}

void i2c_writebit(uint8_t val) {
  if (val) { I2C_SDA_HI; }
  else { I2C_SDA_LO; }
  I2C_WAIT;
  I2C_SCK_HI;
  I2C_WAIT;
  I2C_SCK_LO;
}

void i2c_writebyte(uint8_t byte) {
  for (uint8_t i=0; i<8; i++) {
    i2c_writebit(byte & 0x80);
    byte <<= 1;
  }

  //Make sure you read the ACK
  I2C_SDA_HI;
  I2C_WAIT;
  I2C_SCK_HI;
  I2C_WAIT;
  //Read should be done on this line. Not using it yet so not doing read.
  I2C_SCK_LO;
  I2C_WAIT;

}

void i2c_cmd(uint8_t cmd) {
  i2c_start();
  i2c_writebyte(0x78);
  i2c_writebyte(0x80);
  i2c_writebyte(cmd);
  i2c_stop();
}

void init_i2c(void) {
  I2C_DDR &= ~(SCK | SDA);
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

  uint8_t inputs = (~BUTPIN) & BUTMASK;
  uint8_t lastbut = inputs;
  while(1)
  {
    inputs = (~BUTPIN) & BUTMASK;
    if (lastbut != inputs) {
      lastbut = inputs;
      uint8_t poordebounce = 0;
      //Set column start and end address
      i2c_cmd(0x21);
      i2c_cmd(0x00);
      i2c_cmd(0xFF);
      if (inputs & SW) {
        PORTB |= (1<<PB0);
        fill_screen(0x00);
        oled_puts("Stumpy");
        ++poordebounce;
      }
      if (inputs & LT) {
        PORTB |= (1<<PB0);
        fill_screen(0x00);
        oled_puts("Asa");
        ++poordebounce;
      }
      if (inputs & UP) {
        PORTB |= (1<<PB0);
        fill_screen(0x00);
        oled_puts("Ione");
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
        fill_screen(0x00);
        oled_puts("Buddha");
        ++poordebounce;
      }
      if (poordebounce) {
        //Delay so we don't read multiple presses
        Delay_ms(80);
        PORTB &= ~(1<<PB0);
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
