/**************************************************************
*
* File:: SensorController.c
*
* Description:: Implementation of the GPIO interactions with
*	the sensors and the conversion of their soutputs into
* 	understandable values that can be used elsewhere for
*	important functions of the car
*
**************************************************************/

#include "SensorController.h"

struct ir_struct ir_1, ir_2, ir_3, ir_4, ir_5, ir_left, ir_right;
struct echo_struct echo_1, echo_2, echo_3;
RGB rgb;

pthread_t pthread_IRs;
pthread_t pthread_ECHOs;
pthread_t pthread_RGB;

// -------------------- Constants --------------------

int initialize_pins(void) {
	ir_1.pin = IR_PIN_1;
	ir_2.pin = IR_PIN_2;
	ir_3.pin = IR_PIN_3;
	ir_4.pin = IR_PIN_4;
	ir_5.pin = IR_PIN_5;
	ir_left.pin = IR_PIN_LEFT;
	ir_right.pin = IR_PIN_RIGHT;

	echo_1.echo_pin = ECHO_PIN_1;
	echo_2.echo_pin = ECHO_PIN_2;
	echo_3.echo_pin = ECHO_PIN_3;

	echo_1.trigger_pin = TRIGGER_PIN_1;
	echo_2.trigger_pin = TRIGGER_PIN_2;
	echo_3.trigger_pin = TRIGGER_PIN_3;

	return 0;
}

// -------------------- Initialize and Terminate GPIO --------------------

int initialize_gpio(void) {

	// Initialize PIGPIO
	if (gpioInitialise() < 0) {
        printf("ERROR: Failed to initialize GPIO\n");
        return 1;
    }

	// Init Button
	if (gpioSetMode(BUTTON_PIN, PI_INPUT)) {
		printf("Error: Failed to set BUTTON_PIN as input \n");
		return -1;
	}
	gpioSetPullUpDown(BUTTON_PIN, PI_PUD_DOWN);

	// Init IR
	if (gpioSetMode(IR_PIN_1, PI_INPUT) ||
		gpioSetMode(IR_PIN_2, PI_INPUT) ||
		gpioSetMode(IR_PIN_3, PI_INPUT) ||
		gpioSetMode(IR_PIN_4, PI_INPUT) ||
		gpioSetMode(IR_PIN_5, PI_INPUT) ||
		gpioSetMode(IR_PIN_LEFT, PI_INPUT) ||
		gpioSetMode(IR_PIN_RIGHT, PI_INPUT)) {
		printf("Error: Failed to set IR pins as input\n");
		return -1;
	}

	// Init Echo
	if (gpioSetMode(ECHO_PIN_1, PI_INPUT) ||
		gpioSetMode(ECHO_PIN_2, PI_INPUT) ||
		gpioSetMode(ECHO_PIN_3, PI_INPUT)) {
		printf("Error: Failed to set ECHO pins as input\n");
		return -1;
	}
	
	if (gpioSetMode(TRIGGER_PIN_1, PI_OUTPUT) ||
		gpioSetMode(TRIGGER_PIN_2, PI_OUTPUT) ||
		gpioSetMode(TRIGGER_PIN_3, PI_OUTPUT)) {
		printf("Error: Failed to set TRIGGER pins as output\n");
		return -1;
	}

	if (gpioWrite(TRIGGER_PIN_1, 0) ||
		gpioWrite(TRIGGER_PIN_2, 0) ||
		gpioWrite(TRIGGER_PIN_3, 0)) {
		printf("Error: Failed to set TRIGGER pins to low\n");
		return -1;
	}

	return 0;
}

int terminate_gpio(void) {
	gpioTerminate();
	return 0;
}

// -------------------- Initialize and Terminate I2C --------------------

int initialize_rgb(void) {
	if (TCS34725_Init()) {
		printf("ERROR: TCS34725 Failed to initalize\n");
		return -1;
	}
	return 0;
}

int terminate_rgb(void) {
	I2C_CloseDevice(2);
	return 0;
}

// -------------------- Threads --------------------

void *ir_thread(void *arg) {
	while (1) {
		ir_1.value = !gpioRead(ir_1.pin);
		ir_2.value = !gpioRead(ir_2.pin);
		ir_3.value = !gpioRead(ir_3.pin);
		ir_4.value = !gpioRead(ir_4.pin);
		ir_5.value = !gpioRead(ir_5.pin);
		ir_left.value = gpioRead(ir_left.pin);
		ir_right.value = gpioRead(ir_right.pin);
	}
}

void echo_read(struct echo_struct *echo) {
	struct timespec start, end;

	gpioWrite(echo->trigger_pin, 1);
	gpioDelay(10);
	gpioWrite(echo->trigger_pin, 0);
	
	while (gpioRead(echo->echo_pin) == 0)
		clock_gettime(CLOCK_MONOTONIC, &start);
	
	while (gpioRead(echo->echo_pin) ==  1)
		clock_gettime(CLOCK_MONOTONIC, &end);

	double elapsed_time = (end.tv_sec - start.tv_sec) * 1e6 + (end.tv_nsec - start.tv_nsec) / 1e3;
	echo->value = (elapsed_time * 0.0340) / 2;

	if (echo->value <= 0 ){
		echo->value = 0;
	}
}

void *echo_thread(void *arg) {
	while (1) {
		echo_read(&echo_1);
		echo_read(&echo_2);
		echo_read(&echo_3);
		usleep(1000);
	}
}

void *rgb_thread(void *arg) {
	while (1) {
		rgb = TCS34725_Get_RGBData();
		usleep(1000);
	}
}

// -------------------- Initialize and Terminate Threads --------------------

int initialize_threads(void) {
	
	// Init IR
	if (pthread_create(&pthread_IRs, NULL, ir_thread, NULL)) {
		printf("ERROR: Failed to create IR threads\n");
		return -1;
	}

	// Init ECHO
	if (pthread_create(&pthread_ECHOs, NULL, echo_thread, NULL)) {
		printf("ERROR: Failed to create ECHO thread\n");
		return -1;
	}

	// Init RGB
	if (pthread_create(&pthread_RGB, NULL, rgb_thread, NULL)) {
		printf("ERROR: Failed to create RGB thread\n");
		return -1;
	}

	return 0;
}

int terminate_threads(void) {
	
	// Terminate IR
	if (pthread_cancel(pthread_IRs)) {
		printf("ERROR: Failed to terminate IR threads\n");
		return -1;
	}
	
	// Terminate ECHO
	if (pthread_cancel(pthread_ECHOs)) {
		printf("ERROR: Failed to terminate ECHO threads\n");
		return -1;
	}

	// Terminate RGB
	if (pthread_cancel(pthread_RGB)) {
		printf("ERROR: Failed to terminate RGB threads\n");
		return -1;
	}

	return 0;
}

// -------------------- Initialize and Terminate --------------------

int initialize(void) {
	
	printf("\n----- Initializing... -----\n\n");

	printf("\t(1/5)\tInitializing pins: ");
	if (initialize_pins()) {
		printf("\t\t❌\n");
		printf("ERROR: Initializing pins failed in initialize\n");
		return -1;
	}
	printf("✅\n");

	printf("\t(2/5)\tInitializing gpio: ");
	if (initialize_gpio()) {
		printf("\t\t❌\n");
		printf("ERROR: Initializing gpio failed in initialize\n");
		return -1;
	}
	printf("✅\n");
	
	printf("\t(3/5)\tInitializing rgb: ");
	if (initialize_rgb()) {
		printf("\t\t❌\n");
		printf("ERROR: Initializing rgb failed in initialize\n");
		return -1;
	}
	printf("✅\n");

	printf("\t(4/5)\tInitializing threads: ");
	if (initialize_threads()) {
		printf("\t\t❌\n");
		printf("ERROR: Initializing threads failed in initialize\n");
		return -1;
	}
	printf("✅\n");
	
	printf("\t(5/5)\tInitializing motors: ");
	if (initialize_motors()) {
		printf("\t\t❌\n");
		printf("ERROR: Initializing motors failed in initialize\n");
		return -1;
	}
	printf("✅\n");

	printf("\n----- Initialized Successfully -----\n\n");

	return 0;
}

int terminate(void) {
    printf("\n----- Terminating... -----\n\n");

    printf("\t(1/4)\tTerminating motors: ");
    if (terminate_motors()) {
        printf("\t\t❌\n");
        printf("ERROR: Terminating motors failed in terminate\n");
		return -1;
    }
    printf("✅\n");
	
	printf("\t(2/4)\tTerminating threads: ");
	if (terminate_threads()) {
		printf("\t\t❌\n");
		printf("ERROR: Terminating threads failed in terminate\n");
		return -1;
	}
	printf("✅\n");

    printf("\t(3/4)\tTerminating rgb: ");
    if (terminate_rgb()) {
        printf("\t\t❌\n");
        printf("ERROR: Terminating rgb failed in terminate\n");
		return -1;
    }
    printf("✅\n");

    printf("\t(4/4)\tTerminating gpio: ");
    if (terminate_gpio()) {
        printf("\t\t❌\n");
        printf("ERROR: Terminating gpio failed in terminate\n");
		return -1;
    }
    printf("✅\n");

    printf("\n----- Terminated Successfully -----\n\n");

	return 0;
}
