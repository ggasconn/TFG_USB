#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>

#include "pitches.h"

const int melody[] = {
    NOTE_C4, NOTE_C4, 
    NOTE_D4, NOTE_C4, NOTE_F4,
    NOTE_E4, NOTE_C4, NOTE_C4, 
    NOTE_D4, NOTE_C4, NOTE_G4,
    NOTE_F4, NOTE_C4, NOTE_C4,

    NOTE_C5, NOTE_A4, NOTE_F4, 
    NOTE_E4, NOTE_D4, NOTE_AS4, NOTE_AS4,
    NOTE_A4, NOTE_F4, NOTE_G4,
    NOTE_F4
};

const int durations[] = {
    4, 8, 
    4, 4, 4,
    2, 4, 8, 
    4, 4, 4,
    2, 4, 8,
    
    4, 4, 4, 
    4, 4, 4, 8,
    4, 4, 4,
    2
};
    
int size = sizeof(durations) / sizeof(int);

int openBlinkstick(char *path) {
	int fd;

	if ((fd = open(path, O_RDWR)) < 0) {
		printf("ERROR! Couldn't open device %s\n", path);
		return -1;
	}
	
	return fd;	 
}

int main(int argc, char** argv) {
    int fd = openBlinkstick("/dev/usb/pwnedDevice0");
    char buf[200];

    for (int note = 0; note < size; note++) {
        int duration = 1000 / durations[note];
        
        sprintf(buf, "buzzer %d %d", melody[note], 0); // Write note to buffer
        write(fd, &buf[0], strlen(buf));       

        int pauseBetweenNotes = duration * 2.0;
        usleep(pauseBetweenNotes*1000);

        sprintf(buf, "buzzer %d %d", 0, 1); // Stop buzzer
        write(fd, &buf[0], strlen(buf));       
    }

    close(fd);

    return 0;
}