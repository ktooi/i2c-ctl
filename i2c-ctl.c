#include "i2c-ctl.h"
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <string.h>

#if MODULE
  #include <linux/kernel.h>
  #include <linux/module.h>
  #include <linux/errno.h>
  #include <linux/fs.h>
#else
  #include <stdio.h>
  #include <stdlib.h>
  //ユーザランドでも動くようにするための、関数・定数の再定義
  #define printk(...) fprintf(stderr, __VA_ARGS__)
  #define KERN_INFO ""
  #define KERN_NOTICE ""
  #define KERN_WARNING ""
  #define KERN_ERR ""
#endif

#define DEV_NAME_MAX_LENGTH     64
#define I2C_DEV_NAME_MAX_LENGTH 64

struct i2c_slave {
  char dev_name[DEV_NAME_MAX_LENGTH];
  char i2c_dev_name[I2C_DEV_NAME_MAX_LENGTH];
  char slave_id;
  unsigned int retry_num;
  unsigned int retry_interval;
  int fd;
};

I2CSlave* gen_i2c_slave(char *i2c_dev_name, char *dev_name, char slave_id, unsigned int retry_num, unsigned int retry_interval) {

  I2CSlave *i2c_slave;
  if ((i2c_slave = malloc(sizeof(I2CSlave))) == NULL) {
    printk(KERN_WARNING "%s : Can not allocate on memory.\n", dev_name);
    return (I2CSlave *)NULL;
  }
  strncpy(i2c_slave->i2c_dev_name, i2c_dev_name, I2C_DEV_NAME_MAX_LENGTH);
  strncpy(i2c_slave->dev_name, dev_name, DEV_NAME_MAX_LENGTH);
  i2c_slave->slave_id = slave_id;
  i2c_slave->retry_num = retry_num;
  i2c_slave->retry_interval = retry_interval;
  i2c_slave->fd = -1;
}

int destroy_i2c_slave(I2CSlave *i2c_slave) {

  if (i2c_slave->fd != -1) {
    if (term_i2c_slave(i2c_slave) == -1) {
      printk(KERN_WARNING "%s : Failed destroy I2C Slave object.\n", i2c_slave->dev_name);
      return -1;
    }
  }
  free(i2c_slave);
  return 0;
}

// write 関数は引数の型が微妙に異なる為、
// コンパイル時に incompatible pointer type の警告が出る。
ssize_t my_write(int handle, void* buf, size_t n) {

  return write(handle, (const void*) buf, n);
}

int call_i2c_slave(ssize_t (*pFunc)(int handle, void* buf, size_t n), int fd, void *data, unsigned size, int retry, int usec) {

  int count = 0;
  // pFunc の処理を実行。
  // 処理に失敗した場合は、指定回数リトライする。
  // リトライの間隔は引数の usec で指定されたμ秒。
  while ((*pFunc)(fd, data, size) == -1) {
    if (retry < ++count) {
      return -1;
    }
    usleep(usec);
  }
  return 0;
}

int write_i2c_slave(I2CSlave *i2c_slave, const void* data, int size) {

  if (call_i2c_slave(my_write, i2c_slave->fd, (void*) data, size, i2c_slave->retry_num, i2c_slave->retry_interval) == -1) {
    printk(KERN_WARNING "%s : Failed write the data to I2C Slave device.\n", i2c_slave->dev_name);
    return -1;
  }
  return 0;
}

int read_i2c_slave(I2CSlave *i2c_slave, void* buf, int size) {

  if (call_i2c_slave(read, i2c_slave->fd, buf, size, i2c_slave->retry_num, i2c_slave->retry_interval) == -1) {
    printk(KERN_WARNING "%s : Failed read the data from I2C Slave device.\n", i2c_slave->dev_name);
    return -1;
  }
  return 0;
}

int init_i2c_slave(I2CSlave *i2c_slave) {

  if ((i2c_slave->fd = open(i2c_slave->i2c_dev_name, O_RDWR)) < 0) {
    printk(KERN_WARNING "%s : Failed open I2C Slave device.\n", i2c_slave->dev_name);
    return -1;
  }
  if (ioctl(i2c_slave->fd, I2C_SLAVE, i2c_slave->slave_id) < 0) {
    printk(KERN_WARNING "%s : Failed ioctl I2C Slave device.\n", i2c_slave->dev_name);
    return -1;
  }
  return 0;
}

int term_i2c_slave(I2CSlave *i2c_slave) {

  if (close(i2c_slave->fd) == -1) {
    printk(KERN_WARNING "%s : Failed close I2C Slave device.\n", i2c_slave->dev_name);
    return -1;
  }
  i2c_slave->fd = -1;
  return 0;
}
