#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h> 
#include <fcntl.h> 
#include <signal.h>
#include <sys/stat.h> 
#include <sys/types.h> 

#define PAUSE 1

int main()
{
	// initialize file descriptors for pipes
	//int fd_insp;	
	int fd_motor_x;	
	int fd_motor_z;
	int fd_motor_x_to_insp;
	int fd_motor_z_to_insp;

	// initialize the temporary file
	//char* f_insp = "/tmp/f_insp";
	char* f_motor_x = "/tmp/f_motor_x";
	char* f_motor_z = "/tmp/f_motor_z";
	char* f_motor_x_to_insp = "/tmp/f_motor_x_to_insp";
	char* f_motor_z_to_insp = "/tmp/f_motor_z_to_insp";

	// get motor x and motor z PIDS for signal handling
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
	// get motor z PID
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
	
	printf("dofiwnpfmo");
	//defining select function's variables
	char comm_inspect;
	int n;
	fd_set fd_in;

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
		//printf("command is %c\n", comm_inspect[0]);	

		fd_motor_x = open(f_motor_x, O_RDONLY);
		fd_motor_z = open(f_motor_z, O_RDONLY);
		float pos_x;
		float pos_z;


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

		system("clear");

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
		sleep(PAUSE);
	}
	
	close(fd_motor_x);
	close(fd_motor_z);
	unlink(f_motor_x);
	unlink(f_motor_z);
	//unlink(f_insp);
	return 0;
}
