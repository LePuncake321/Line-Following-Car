/**************************************************************
*
* File:: main.h
*
* Description:: ... 
*
**************************************************************/

#ifndef _MAIN_H_
#define _MAIN_H_

#include "SensorController.h"
#include "MotorDriver.h"
#include <time.h>

// IR Handler

#define KP 10.0
#define KI 0.0
#define KD 20.0
#define BASE_SPEED 100

// Echo Handler

#define DISTANCE_TO_AVOID 50

// RGB Handler

#define OBSTACLE_WAIT_SEC 0.15
#define RGB_WAIT_SEC .1

#endif