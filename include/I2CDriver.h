#ifndef _I2C_DRIVER_H_
#define _I2C_DRIVER_H_

#include <stdint.h>

#define MAX_BUSES 2

typedef struct {
    int bus_index;      // Index of the I2C bus (e.g., 0 for /dev/i2c-1, 1 for /dev/i2c-2)
    int fd;             // File descriptor for the I2C bus
    uint8_t address;    // I2C address of the device
} I2C_Bus;

int I2C_OpenDevice(int bus_index, uint8_t address);
void I2C_CloseDevice(int bus_index);
void I2C_CloseAllDevices();

void I2C_WriteByte(int bus_index, uint8_t reg, uint8_t value);
void I2C_WriteWord(int bus_index, uint8_t reg, uint16_t value);

uint8_t I2C_ReadByte(int bus_index, uint8_t reg);
uint16_t I2C_ReadWord(int bus_index, uint8_t reg);

#endif // _I2C_DRIVER_H_