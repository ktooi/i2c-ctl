#ifndef I2CCTL_H
#define I2CCTL_H
#include <fcntl.h>
#include <sys/types.h>

struct i2c_slave;
typedef struct i2c_slave I2CSlave;

I2CSlave* gen_i2c_slave(char *i2c_dev_name, char *dev_name, char slave_id, unsigned int retry_num, unsigned int retry_interval);

int destroy_i2c_slave(I2CSlave *i2c_slave);

int call_i2c_slave(ssize_t (*pFunc)(int handle, void* buf, size_t n), int fd, void *data, unsigned size, int retry, int usec);

int write_i2c_slave(I2CSlave *i2c_slave, const void* data, int size);

int read_i2c_slave(I2CSlave *i2c_slave, void* buf, int size);

int init_i2c_slave(I2CSlave *i2c_slave);

int term_i2c_slave(I2CSlave *i2c_slave);
#endif
