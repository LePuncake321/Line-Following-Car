/**************************************************************
*
* File:: MotorDriver.h
*
* Description:: High-level interface that abstracts how to
*   modify the speed, direction and state of the motor through
*   the Motor HAT; Additionally, this is where the 
*   Proportional–integral–derivative Algorithm is interfaced
*
**************************************************************/

#ifndef _MOTORDRIVER_H
#define _MOTORDRIVER_H

#include "PCA9685.h"
#include "main.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>

// GPIO config for motor wires going into A slots
#define PWMA PCA_CHANNEL_2
#define AIN1 PCA_CHANNEL_1
#define AIN2 PCA_CHANNEL_0

// GPIO config for motor wires going into B slots
#define PWMB PCA_CHANNEL_5
#define BIN1 PCA_CHANNEL_4
#define BIN2 PCA_CHANNEL_3

// Definition of values for the direction of the motors
enum Direction {
    FORWARD = 1,
    BACKWARD = 2
};

int initialize_motors(void);
int terminate_motors(void);

// Set the direction of the motor based on enum DIRECTION and speed
void set_left_motor(int speed, enum Direction dir);
void set_right_motor(int speed, enum Direction dir); 

// Change speed of both motors (Always moving forwards)
void set_motor_speed(int left, int right);

#endif
