/**************************************************************
*
* File:: main.c
*
* Description:: ... 
*
**************************************************************/

#include "main.h"

#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <bits/time.h>
#include <stdio.h>
#include <math.h>

// IR Handler

int weights[] = {200, 16, 8, 0, -8, -16, -200};
float previous_error = 0;
float integral = 0;

// Echo Handler

int isObjectDetectionStarted = 0;
struct timespec objectDetectionStart;
struct timespec objectDetectionEnd;
float echo_prev_err = 0;
float echo_gral = 0;

// RGB Handler

uint32_t RGB888 = 0;

int isColorDetectionStarted = 0;
struct timespec colorDetectionStart;
struct timespec colorDetectionEnd;

// File Handler

// const char *filename = "red.json";
// FILE *file;

// -------------------- Handlers --------------------

int ir_handler(void) {
    int ir_values[] = { ir_left.value, ir_1.value, ir_2.value, ir_3.value, ir_4.value, ir_5.value, ir_right.value};
    int sum = 0;
    int active = 0;

    for (int i = 0; i < 7; i++) {
        if (!ir_values[i]) {
            sum += weights[i];
            active++;
        }
    }

	if (active == 0 || active == 7) return 0;

    float error = (float) sum / active;
    integral += error;

	float derivative = error - previous_error;
    float correction = KP * error + KI * integral + KD * derivative;

    previous_error = error;

    int left_speed = BASE_SPEED + correction;
    int right_speed = BASE_SPEED - correction;

    if (left_speed > 100) left_speed = 100;
    if (left_speed < 0) left_speed = 0;
    if (right_speed > 100) right_speed = 100;
    if (right_speed < 0) right_speed = 0;

    set_motor_speed(left_speed, right_speed);

	return 0;
}

/**
 * @brief Executes a basic avoidance maneuver to sidestep an obstacle and rejoin the line.
 */
void avoid_obstacle() {
	printf("----- Avoiding Obstacle -----\n");
    set_motor_speed(0, 0);
    usleep(200000);  // short pause

    // Step 1: turn left ~45°
	set_motor_speed(0, 100);
    usleep(900000);

    // Step 2: drive forward
    set_motor_speed(100, 100);
	usleep(1200000);

    // Step 3: turn back right ~45°
	set_motor_speed(100, 0);
    usleep(650000);

	// Step 4: drive forward
	set_motor_speed(100, 100);
	usleep(1500000);

	// Step 5: turn back left ~45°
	set_motor_speed(100, 0);
	usleep(650000);

    // Step 6: drive forward again briefly
    set_motor_speed(100, 100);
	usleep(200000);
}

/**
 * @brief Detects and avoids obstacles in front of the car.
 *
 * @details This function detects if an object is in front of the car. 
 *          If an object is detected, the car will wait for a period of 
 *          `DETECT_WAIT_SEC` seconds. If the object persists, the car will 
 *          execute an avoidance maneuver. If the object is gone after the wait,
 *          the car will continue its normal movement.
 *
 * @return 0 if the obstacle detection and avoidance (if needed) was successful.
 * @return -1 if an error occurred during detection or avoidance.
 */
int echo_handler(void) {

	// Skip if the distance is greater than DISTANCE_TO_AVOID
	if (echo_2.value > DISTANCE_TO_AVOID || echo_2.value == 0) {
		if (isObjectDetectionStarted) printf("----- No Obstacle Detected ---\n");
		isObjectDetectionStarted = 0;
		return 0;
	}

	// Gets the start and end time of the object detection
	if (!isObjectDetectionStarted) {
		printf("----- Obstacle Detected -----\n");
		isObjectDetectionStarted = 1;
		clock_gettime(CLOCK_MONOTONIC, &objectDetectionStart);
	} else {
		clock_gettime(CLOCK_MONOTONIC, &objectDetectionEnd);		
	}

	double elapsed = (objectDetectionEnd.tv_sec - objectDetectionStart.tv_sec) +
                 (objectDetectionEnd.tv_nsec - objectDetectionStart.tv_nsec) / 1e9;

	printf("Elapsed = %d\n", (int) elapsed);

	// If an object is detected for DETECT_WAIT_SEC seconds, avoid it
	if (objectDetectionEnd.tv_nsec - objectDetectionStart.tv_nsec >= OBSTACLE_WAIT_SEC * 1e9) {
		printf("----- Obstacle Detected -----\n");
		// avoid_obstaclev3();
		avoid_obstacle();
		//avoid_obstaclev2(1); // 1 = go left, 0 = go right
	}

	return 0;
}

// --------------------- RGB Handler --------------------

int rgb_handler(void) {

	uint32_t colors = TCS34725_GetRGB888(rgb);

	int r = (colors >> 16) & 0xFF;
	int g = (colors >> 8) & 0xFF;
	int b = colors & 0xFF;

	printf("RGB = \t%d %d %d\n", r - 190, g, b);

	if (!(r - 190 >= (g + b) / 2)) {
		if (isColorDetectionStarted) printf("\n----- RGB Detect Failed -----\n\n");
		isColorDetectionStarted = 0;
		return 0;
	}
	
	if (!isColorDetectionStarted) {
		printf("\n ----- RGB Detect Starting ----- \n\n");
		isColorDetectionStarted = 1;
		clock_gettime(CLOCK_MONOTONIC, &colorDetectionStart);
	} else {
		clock_gettime(CLOCK_MONOTONIC, &colorDetectionEnd);
	}

	double elapsed = (colorDetectionEnd.tv_sec - colorDetectionStart.tv_sec) +
                 (colorDetectionEnd.tv_nsec - colorDetectionStart.tv_nsec) / 1e9;

	if (elapsed >= RGB_WAIT_SEC) {
		printf("\n----- RGB Detect Success -----\n\n");
		set_motor_speed(0, 0);
		usleep(3000000);
		isColorDetectionStarted = 0;
		set_motor_speed(100, 100);
	}
		
	return 0;
}

// -------------------- Main --------------------

void sigint_handler(int sig) {
    printf("\nSIGINT caught — cleaning up...\n");
    terminate();
    exit(0);
}

int main(void) {
	if (initialize()) {
		printf("ERROR: Sensor Controller failed to initalize\n");
		return -1;
	}

	signal(SIGINT, sigint_handler);

	printf("Waiting for button press...\n");	
	while (!gpioRead(BUTTON_PIN));

	set_motor_speed(100, 100);

	while (1) {
		printf("IR = \t%d %d %d %d %d %d %d\n", ir_left.value, ir_1.value, ir_2.value, ir_3.value, ir_4.value, ir_5.value, ir_right.value);
		ir_handler();

		printf("ECHO = \t%f %f %f\n", echo_1.value, echo_2.value, echo_3.value);
		echo_handler();

		// rgb_handler();
    }

	terminate();

    return 0;
}
