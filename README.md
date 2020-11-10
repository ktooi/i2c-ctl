# i2c-ctl

このプロジェクトは、 Linux 上でC言語で I2C 通信を行うためのライブラリです。

## Example

```c
#include "i2c-ctl.h"
#define I2C_ID 0x01

int main(void) {
  char write_data[3], read_buf[8];
  I2CSlave *i2c_dev;

  i2c_dev = gen_i2c_slave("/dev/i2c-1", "my_i2c_name", I2C_ID, 1, 3000);
  if (init_i2c_slave(i2c_dev) < 0) {
    exit 1;  # ERROR
  }
  write_data[0] = 0x00;
  write_data[1] = 0x01;
  write_data[2] = 0x02;
  if (write_i2c_slave(i2c_slave, write_data, 3) < 0) {
    exit 1;  # ERROR
  }
  if (read_i2c_slave(i2c_slave, read_buf, 8) < 0) {
    exit 1;  # ERROR
  }
  if (destroy_i2c_slave(i2c_slave) < 0) {
    exit 1;  # ERROR
  }
}
```
