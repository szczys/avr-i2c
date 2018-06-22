#define F_CPU 8000000L

#include <avr/io.h>
#include <avr/interrupt.h>
#include "i2c-hardware.h"
#include <util/delay.h>

#define I2C_PORT  PORTB
#define I2C_DDR   DDRB
#define SCK   1<<PB6
#define SDA   1<<PB7

#define I2C_SDA_LO    I2C_DDR |= SDA
#define I2C_SCK_LO    I2C_DDR |= SCK
#define I2C_SDA_HI    I2C_DDR &= ~(SDA)
#define I2C_SCK_HI    I2C_DDR &= ~(SCK)
#define I2C_WAIT      _delay_us(5)

void i2c_start(void) {
  TWCR = (1<<TWINT) | (1<<TWSTA)| (1<<TWEN);
  while (!(TWCR & (1<<TWINT)));
}

void i2c_stop(void) {
  TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWSTO);
}

void i2c_writebyte(uint8_t byte) {
  TWDR = byte;
  TWCR = (1<<TWINT) | (1<<TWEN);
  while (!(TWCR & (1<<TWINT)));
}

void i2c_cmd(uint8_t cmd) {
  i2c_start();
  i2c_writebyte(0x78);
  i2c_writebyte(0x80);
  i2c_writebyte(cmd);
  i2c_stop();
}

void init_i2c(void) {
  //I2C_DDR &= ~(SCK | SDA);
  sei();
}
