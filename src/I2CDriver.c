#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdint.h>

#define MAX_BUSES 2     // Number of supported I2C buses (expandable)

typedef struct {
    int bus_index;      // Index of the I2C bus (e.g., 0 for /dev/i2c-1, 1 for /dev/i2c-2)
    int fd;             // File descriptor for the I2C bus
    uint8_t address;    // I2C address of the device
} I2C_Bus;

I2C_Bus i2c_buses[MAX_BUSES];

// --------------------- Initialize and Terminate --------------------

int I2C_OpenDevice(int bus_index, uint8_t address) {
    if (bus_index > MAX_BUSES || bus_index <= 0) {
        printf("ERROR: Invalid bus index (attempted %d).\n", bus_index);
        return -1;
    }

    char i2c_device[15];
    sprintf(i2c_device, "/dev/i2c-%d", bus_index);

    int fd = open(i2c_device, O_RDWR);
    if (fd < 0) {
        perror("ERROR: Failed to open I2C device.\n");
        return -1;
    }

    if (ioctl(fd, I2C_SLAVE, address) < 0) {
        perror("ERROR: Failed to access I2C device.\n");
        close(fd);
        return -1;
    }

    i2c_buses[bus_index - 1].bus_index = bus_index;
    i2c_buses[bus_index - 1].address = address;
    i2c_buses[bus_index - 1].fd = fd;

    return 0;  // Device initialized successfully
}

void I2C_CloseDevice(int bus_index) {
    if (bus_index > MAX_BUSES || bus_index < 0) {
        printf("ERROR: Invalid bus index.\n");
        return;
    }

    close(i2c_buses[bus_index - 1].fd);
    i2c_buses[bus_index - 1].fd = -1;
}

void I2C_CloseAllDevices() {
    for (int i = 0; i < MAX_BUSES; i++) {
        if (i2c_buses[i].fd != -1) {
            close(i2c_buses[i].fd);
            i2c_buses[i].fd = -1;
        }
    }
}

// -------------------- Write --------------------

void I2C_WriteByte(int bus_index, uint8_t reg, uint8_t value) {
    if (bus_index > MAX_BUSES || bus_index < 0) {
        printf("ERROR: I2C_WriteByte failed.\n");
        printf("ERROR: Invalid bus index.\n");
        return;
    }

    uint8_t buffer[2] = {reg, value};
    if (write(i2c_buses[bus_index - 1].fd, buffer, 2) != 2) {
        printf("ERROR: Failed to write to device on bus %d\n", bus_index);
    }
}

void I2C_WriteWord(int bus_index, uint8_t reg, uint16_t value) {
    if (bus_index > MAX_BUSES || bus_index < 0) {
        printf("ERROR: I2C_WriteWord failed.\n");
        printf("ERROR: Invalid bus index.\n");
        return;
    }

    uint8_t buffer[3] = {reg, (value >> 8), (value & 0xFF)};
    if (write(i2c_buses[bus_index - 1].fd, buffer, 3) != 3) {
        printf("ERROR: Failed to write to device on bus %d\n", bus_index);
    }
}

// -------------------- Read --------------------

uint8_t I2C_ReadByte(int bus_index, uint8_t reg) {
    if (bus_index > MAX_BUSES || bus_index < 0) {
        printf("ERROR: I2C_ReadByte failed.\n");
        printf("ERROR: Invalid bus index.\n");
        return 1;
    }

    write(i2c_buses[bus_index - 1].fd, &reg, 1);
    uint8_t buffer[1];
    if (read(i2c_buses[bus_index - 1].fd, buffer, 1) != 1) {
        printf("ERROR: Failed to read from device on bus %d\n", bus_index);
        return 0;
    }
    return buffer[0];
}

// Function to read a word from a specific device
uint16_t I2C_ReadWord(int bus_index, uint8_t reg) {
    if (bus_index > MAX_BUSES || bus_index < 0) {
        printf("ERROR: I2C_ReadWord failed.\n");
        printf("ERROR: Invalid bus index.\n");
        return 1;
    }
    
    write(i2c_buses[bus_index - 1].fd, &reg, 1);
    uint8_t buffer[2];
    if (read(i2c_buses[bus_index - 1].fd, buffer, 2) != 2) {
        printf("ERROR: Failed to read from device on bus %d\n", bus_index);
        return 0;
    }
    return ((buffer[0] << 8) | buffer[1]);
}