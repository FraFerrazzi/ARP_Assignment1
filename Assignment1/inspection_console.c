#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h> 
#include <fcntl.h> 
#include <signal.h>
#include <sys/stat.h> 
#include <sys/types.h> 

#define PAUSE 10000

int main()
{
	// Open a file pointer named "logfileInspectionCons.txt" for writing (w+)
	FILE *fp; 
	fp = fopen("./logfile/logfileInspectionCons.txt", "w");
	if(fp == NULL)
   	{
    	printf("Error opening the logfileMaster!");   
    	exit(1);             
   	}

	char* data = "The data to be logged...";
	fputs(data, fp);

	fclose(fp);
	// initialize file descriptors for pipes
	int fd_motor_x;	
	int fd_motor_z;
	int fd_motor_x_to_insp;
	int fd_motor_z_to_insp;
	int fd_insp;

	// initialize the temporary file
	char* f_motor_x = "/tmp/f_motor_x";
	char* f_motor_z = "/tmp/f_motor_z";
	char* f_motor_x_to_insp = "/tmp/f_motor_x_to_insp";
	char* f_motor_z_to_insp = "/tmp/f_motor_z_to_insp";
	char* f_insp = "/tmp/f_insp"; 

	// get motor x PID for signal handling
	int pidmotorx;
	fd_motor_x_to_insp = open(f_motor_x_to_insp, O_RDONLY);
	if (fd_motor_x_to_insp < 0) {
        perror("fd_motor_x_to_insp");
        return -1;
    }
 	read(fd_motor_x_to_insp, &pidmotorx, sizeof(pidmotorx));
 	close(fd_motor_x_to_insp);
 	printf("pid motor x: %d\n", pidmotorx);
 	fflush(stdout);

	// get motor z PID for signal handling
	int pidmotorz;
	fd_motor_z_to_insp = open(f_motor_z_to_insp, O_RDONLY);
	if (fd_motor_z_to_insp < 0) {
        perror("fd_motor_z_to_insp");
        return -1;
    }
 	read(fd_motor_z_to_insp, &pidmotorz, sizeof(pidmotorz));
 	close(fd_motor_z_to_insp);
 	printf("pid motor z: %d\n", pidmotorz);
 	fflush(stdout);

	// recieve watchdog's PID for signal handling
	int pidwatchdog;
	fd_insp = open(f_insp, O_RDONLY);
	if (fd_insp < 0) {
        perror("fd_insp");
        return -1;
    }
 	read(fd_insp, &pidwatchdog, sizeof(pidwatchdog));
 	close(fd_insp);
 	printf("pid watchdog: %d\n", pidwatchdog);
 	fflush(stdout);
	
	//defining select function's variables
	char comm_inspect;
	int n;
	fd_set fd_in;

	fd_motor_x = open(f_motor_x, O_RDONLY);
	if (fd_motor_x < 0) {
        perror("fd_motor_x");
        return -1;
    }
	fd_motor_z = open(f_motor_z, O_RDONLY);
	if (fd_motor_z < 0) {
        perror("fd_motor_z");
        return -1;
    }
	float pos_x;
	float pos_z;

	for(;;)
	{
		FD_ZERO(&fd_in);
		FD_SET(STDIN_FILENO, &fd_in);
		struct timeval timeout;
		timeout.tv_sec = 0;
		timeout.tv_usec = 0;
		select(STDIN_FILENO+1, &fd_in, NULL, NULL, &timeout);

		if (FD_ISSET(STDIN_FILENO, &fd_in))
		{
			kill(pidwatchdog, SIGUSR1);
			n = read(STDIN_FILENO, &comm_inspect, sizeof(comm_inspect));
			switch(comm_inspect)
			{
				case 'r':
				kill(pidmotorx, SIGINT);
				kill(pidmotorz, SIGINT);
				break;

				case 'e':
				kill(pidmotorx, SIGTERM);
				kill(pidmotorz, SIGTERM);
				break;
				
				default:
				break;
			}
				
		}

		fd_set fds;
		int maxfd;
		int res;
		char bufdispx[20];
		char bufdispz[20];
		struct timeval time;

		FD_ZERO(&fds); // clear FD for the select
		FD_SET(fd_motor_x, &fds);
		FD_SET(fd_motor_z, &fds);
		time.tv_sec = 0;
		time.tv_usec = 0;

		maxfd = fd_motor_x > fd_motor_z ? fd_motor_x : fd_motor_z;

		select(maxfd+1, &fds, NULL, NULL, &time);

		if (FD_ISSET(fd_motor_x, &fds)) // read from motor x file descriptor
		{
			res = read(fd_motor_x, bufdispx, sizeof(bufdispx));
			pos_x = atof(bufdispx);
		}

		if (FD_ISSET(fd_motor_z, &fds))
		{
			res = read(fd_motor_z, bufdispz, sizeof(bufdispz));
			pos_z = atof(bufdispz);
		}
		printf("Motor X position: %f;        Motor Z position: %f;\n", pos_x, pos_z);
		usleep(PAUSE);
		//system("clear");
	}
	
	close(fd_motor_x);
	close(fd_motor_z);
	unlink(f_motor_x);
	unlink(f_motor_z);
	return 0;
}
