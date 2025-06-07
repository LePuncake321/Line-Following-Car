/**************************************************************
*
* File:: SensorController.h
*
* Description::  Interface of the necessary structs and data 
*   to utilize the sensors of our car (IR, Echo and RGB) and
*   provide readable data to be used for the functions of the
*   car, namely following the line of a course, seeing
*   the color of the line, and avoiding an obstacle
*
**************************************************************/

#ifndef INITIALIZE_H
#define INITIALIZE_H

#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <pigpio.h>
#include "MotorDriver.h"
#include "TCS34725.h"
#include <time.h>

#define BUTTON_PIN 1

#define IR_PIN_1 5
#define IR_PIN_2 6
#define IR_PIN_3 13
#define IR_PIN_4 19
#define IR_PIN_5 26
#define IR_PIN_LEFT 12
#define IR_PIN_RIGHT 7

#define ECHO_PIN_1 16  
#define ECHO_PIN_2 20
#define ECHO_PIN_3 21
#define TRIGGER_PIN_1 27
#define TRIGGER_PIN_2 22
#define TRIGGER_PIN_3 15

struct ir_struct {
    int pin;
    int value;
    pthread_t thread;
};

struct echo_struct {
    int echo_pin;
    int trigger_pin;
    float value;
    pthread_t thread;
};

extern struct ir_struct ir_1, ir_2, ir_3, ir_4, ir_5, ir_left, ir_right;
extern struct echo_struct echo_1, echo_2, echo_3;
extern RGB rgb;

int initialize(void);
int terminate(void);

#endif