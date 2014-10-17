#include <stdio.h>
#include <fcntl.h>
#include "lms2012.h"

// Motor power 0..100
const int MOTOR_SPEED = 25;
// The motor operations use a single bit (or a combination of them) 
// to determine which motor(s) will be used
// A = 0x01, B = 0x02, C = 0x04, D = 0x08
// AC = 0x05
const char MOTOR_PORT = 0x04; 

int main()
{
	//DEFINE VARIABLES
	char motor_command[3];
	int motor_file;

	//INITIALIZE DEVICE
	//Open the device file asscoiated to the motor controlers
	if((motor_file = open(PWM_DEVICE_NAME, O_WRONLY)) == -1)
		return -1; //Failed to open device
	
	//ACTUATE MOTORS
	// All motor operations use the first command byte to indicate the type of operation
	// and the second one to indicate the motor(s) port(s)
	motor_command[0] = opOUTPUT_SPEED;
	motor_command[1] = MOTOR_PORT; 
	motor_command[2] = MOTOR_SPEED;
	write(motor_file,motor_command,3);
	// Start the motor
	motor_command[0] = opOUTPUT_START;
	write(motor_file,motor_command,2);
	// Run the motor for a couple of seconds
	printf("speed = %d\n", motor_command[2]);
	sleep(10);

	// Stops the motor
	motor_command[0] = opOUTPUT_STOP;
	write(motor_file,motor_command,2);

	//CLOSE DEVICE
	close(motor_file);
	return 0;
}
