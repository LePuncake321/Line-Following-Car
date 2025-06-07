/**************************************************************
*
* File:: MotorDriver.h
*
* Description:: Implementation of the high-level abstraction
*   for modifying the speed, state and direction of motors
*   through the Motor Driver HAT; The  Proportional–integral– ...
*   ... derivative Algorithm will use the method here to make
*   sure the car stays on a line that is 3 sensors wide
*
**************************************************************/

#include "MotorDriver.h"

// -------------------- Motor Initialization and Termination --------------------

int initialize_motors(void) {
    if (PCA9685_Init(0x40)) {
        printf("ERROR: PCA9685 Failed to initialize\n");
        return -1;
    }

    // Init left motor
    PCA9685_SetLevel(AIN1, 0);
    PCA9685_SetLevel(AIN2, 0);
    PCA9685_SetPwmDutyCycle(PWMA, 0);

    // Init right motor
    PCA9685_SetLevel(BIN1, 0);
    PCA9685_SetLevel(BIN2, 0);
    PCA9685_SetPwmDutyCycle(PWMB, 0);

    return 0;
}

int terminate_motors(void) {

    // Stop both motors
    PCA9685_SetPwmDutyCycle(PWMA, 0);
    PCA9685_SetLevel(AIN1, 0);
    PCA9685_SetLevel(AIN2, 0);

    PCA9685_SetPwmDutyCycle(PWMB, 0);
    PCA9685_SetLevel(BIN1, 0);
    PCA9685_SetLevel(BIN2, 0);

    PCA9685_SetPWMFreq(0);

    // Close PCA9685
    I2C_CloseDevice(1);

    return 0;
}

// -------------------- Motor Setter Methods --------------------

void set_left_motor(int speed, enum Direction dir) {
    speed = (speed < 0) ? 0 : (speed > 100) ? 100 : speed;
    PCA9685_SetPwmDutyCycle(PWMA, speed);

    if (dir == FORWARD) {
        PCA9685_SetLevel(AIN2, 0);
        PCA9685_SetLevel(AIN1, 1);
    } else {
        PCA9685_SetLevel(AIN1, 0);
        PCA9685_SetLevel(AIN2, 1);
    }
}

void set_right_motor(int speed, enum Direction dir) {
    speed = (speed < 0) ? 0 : (speed > 100) ? 100 : speed;
    PCA9685_SetPwmDutyCycle(PWMB, speed);

    if (dir == FORWARD) {
        PCA9685_SetLevel(BIN1, 1);
        PCA9685_SetLevel(BIN2, 0);
    } else {
        PCA9685_SetLevel(BIN1, 0);
        PCA9685_SetLevel(BIN2, 1);
    }
}

void set_motor_speed(int left, int right) {
    // printf("Motor = Left = %d, Right = %d\n", left, right);
    set_left_motor(left, BACKWARD);
    set_right_motor(right * 0.59765625, FORWARD);
}
