#include <avr/io.h>

void i2c_start(void);
void i2c_stop(void);
void i2c_writebit(uint8_t val);
void i2c_writebyte(uint8_t byte);
void init_i2c(void);
