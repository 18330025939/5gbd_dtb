#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include "iic.h"

int i2c_open(I2CDevice *dev)
{
    dev->fd = open(dev->dev_path, O_RDWR);
    if (dev->fd < 0) {
        return -1;
    }

    if (ioctl(dev->fd, I2C_SLAVE, dev->slave_addr) < 0) {
        close(dev->fd);
        return -1;
    }

    return 0;
}

int i2c_read_byte(I2CDevice* dev, uint8_t reg_addr, uint8_t *value)
{
    struct i2c_msg msgs[2] = {
        {
            .addr = dev->slave_addr,
            .flags = 0,
            .len = 1,
            .buf = &reg_addr
        },
        {
            .addr = dev->slave_addr,
            .flags = I2C_M_RD,
            .len = 1,
            .buf = value
        }
    };

    struct i2c_rdwr_ioctl_data data = { msgs, 2 };

    return ioctl(dev->fd, I2C_RDWR, &data);
}

int i2c_read_block(I2CDevice *dev, uint8_t reg_addr, uint8_t *buf, size_t len)
{
    struct i2c_msg msgs[2] = {
        {
            .addr = dev->slave_addr,
            .flags = 0,
            .len = 1,
            .buf = &reg_addr
        },
        {
            .addr = dev->slave_addr,
            .flags = I2C_M_RD,
            .len = len,
            .buf = buf
        }
    };
    struct i2c_rdwr_ioctl_data data = { msgs, 2 };

    return ioctl(dev->fd, I2C_RDWR, &data);
}

int i2c_write_byte(I2CDevice* dev, uint8_t reg_addr, uint8_t value)
{
    uint8_t buf[] = {reg_addr, value};

    struct i2c_msg msg = {
        .addr = dev->slave_addr,
        .flags = 0,
        .len = 2,
        .buf = buf
    };

    struct i2c_rdwr_ioctl_data data = { &msg, 1 };

    return ioctl(dev->fd, I2C_RDWR, &data);
}

int i2c_write_block(I2CDevice *dev, uint8_t reg_addr, const uint8_t *buf, size_t len)
{
    uint8_t *tmp_buf = malloc(len + 1);
    if (!tmp_buf) {
        return -1;
    }
    
    tmp_buf[0] = reg_addr;
    memcpy(tmp_buf + 1, buf, len);

    struct i2c_msg msg = {
        .addr = dev->slave_addr,
        .flags = 0,
        .len = len + 1,
        .buf = tmp_buf
    };

    struct i2c_rdwr_ioctl_data data = { &msg, 1 };
    int ret = ioctl(dev->fd, I2C_RDWR, &data);
    free(tmp_buf);

    return ret;
}

void i2c_close(I2CDevice *dev)
{
    if (dev->fd > 0) {
        close(dev->fd);
    }
}

struct I2CDeviceOps i2c_ops = {
    .open = i2c_open,
    .read_byte = i2c_read_byte,
    .read_block = i2c_read_block,
    .write_byte = i2c_write_byte,
    .write_block = i2c_write_block,
    .close = i2c_close
};

I2CDevice* i2c_device_create(const char *path, uint8_t addr)
{
    I2CDevice *i2c = malloc(sizeof(I2CDevice));
    memset(i2c, 0, sizeof(I2CDevice));

    i2c->ops = &i2c_ops;
    strncpy(i2c->dev_path, path, sizeof(i2c->dev_path) - 1);
    i2c->slave_addr = addr;

    return i2c;
}