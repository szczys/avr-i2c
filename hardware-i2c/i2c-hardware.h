#include <avr/io.h>

void i2c_start(void);
void i2c_stop(void);
void i2c_writebyte(uint8_t byte);
void i2c_cmd(uint8_t cmd);
void init_i2c(void);
