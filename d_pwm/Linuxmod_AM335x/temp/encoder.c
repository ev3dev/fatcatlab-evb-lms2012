#include <fcntl.h>
#include <stdio.h>
#include <sys/mman.h>
#include "lms2012.h"
    
// Motor power 0..100
const int MOTOR_SPEED= 10;
// The motor operations use a single bit (or a combination of them)
// to determine which motor(s) will be used
// A = 0x01, B = 0x02, C = 0x04, D = 0x08
// AC = 0x05
const char MOTOR_PORT_A = 0x01;
const char MOTOR_PORT_D = 0x08;
const int MAX_READINGS = 10000;



    
int main()
{
		    
	MOTORDATA *pMotorData;
	char motor_command[4];
	int motor_file;
	int encoder_file;
	int i;
		    
	//Open the device file asscoiated to the motor controlers
	if((motor_file = open(PWM_DEVICE_NAME, O_WRONLY)) == -1)
	{
		printf("Failed to open device\n");
		return -1; //Failed to open device
	}
		    
	//Open the device file asscoiated to the motor encoders
	if((encoder_file = open(MOTOR_DEVICE_NAME, O_RDWR | O_SYNC)) == -1)
		return -1; //Failed to open device
		    
	pMotorData = (MOTORDATA*)mmap(0, sizeof(MOTORDATA)*vmOUTPUTS, PROT_READ | PROT_WRITE, MAP_FILE | MAP_SHARED, encoder_file, 0);
	
	if (pMotorData == MAP_FAILED)
	{
		printf("Map failed\n");
		return -1;
	}
		    
	// All motor operations use the first command byte to indicate the type of operation
	// and the second one to indicate the motor(s) port(s)
	motor_command[0] = opOUTPUT_SPEED;
	motor_command[1] = MOTOR_PORT_D;
	motor_command[2] = MOTOR_SPEED;
		    
	write(motor_file,motor_command,3);
		    
	// Start the motor
	motor_command[0] = opOUTPUT_START;
	motor_command[1] = MOTOR_PORT_D;
		    
	write(motor_file,motor_command,2);
		    
	// Read encoders while running the motor
	for(i = 1; i < MAX_READINGS; i++)
	{
		printf("Spd/Cnt/Snr: D=%d/%d/%d\n", pMotorData[3].Speed,pMotorData[3].TachoCounts,pMotorData[3].TachoSensor);
	}	    

	// Stop the motor
	motor_command[0] = opOUTPUT_STOP;
		    
	write(motor_file,motor_command,2);
		    
	// Close device files
	close(encoder_file);
		    
	close(motor_file);
		    
	return 0;
    
}


