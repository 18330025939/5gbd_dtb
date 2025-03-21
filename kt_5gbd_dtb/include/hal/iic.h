#ifndef __IIC_H
#define __IIC_H

#include <stdint.h>

typedef struct st_I2CDevice I2CDevice;

struct I2CDeviceOps {
    int (*open)(I2CDevice* dev);
    int (*read_byte)(I2CDevice* dev, uint8_t reg_addr, uint8_t *value);
    int (*read_block)(I2CDevice* dev, uint8_t reg_addr, uint8_t *buf, size_t len);
    int (*write_byte)(I2CDevice* dev, uint8_t reg_addr, uint8_t value);
    int (*write_block)(I2CDevice* dev, uint8_t reg_addr, const uint8_t *data, size_t len);
    void (*close)(I2CDevice* dev);
} ;


struct st_I2CDevice{
    uint8_t slave_addr;
    uint32_t  speed;
    int fd;
    char dev_path[64];
    struct I2CDeviceOps *ops;
} ;


I2CDevice* i2c_device_create(const char *path, uint8_t addr);
#endif