#include <fcntl.h>
#include <stdio.h>
#include  <sys/mman.h>
#include "lms2012.h"

int main()
{
	//DEFINE VARIABLES
	int file;
	UI *pLeds;
	int i;
	//The first byte determines the color or pattern as specified in  bytecodes.h -> LEDPATTERN
	//The second byte (LED number) is not currently being used by the device driver, but is needed
	char led_command[2]= { 0, 0}; 

	//INITIALIZE DEVICE
	//Open the device file
	if((file = open(UI_DEVICE_NAME, O_RDWR | O_SYNC)) == -1)
	{
		printf("Failed to open device\n");
		return -1; 
	}
	printf("Device ready\n");

	//DO SOMETHING WITH THE DEVICE
	//Send LED Command
	while(1)
	{
		for(i = 0; i<LEDPATTERNS; i++)
		{
//			printf("**********************************************************************LED Pattern = %d\n", i);
			//The kernel driver will subtract a '0' offset before using the Color/Pattern instruction (see d_ui.c)
			led_command[0] = '0' + i;
			write(file, led_command, 2);
			sleep(2);
		}
	}

	//CLOSE DEVICE
	printf("Closing device\n");
	close(file);
	return 0;
}
