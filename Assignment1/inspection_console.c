#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h> 
#include <fcntl.h> 
#include <signal.h>
#include <sys/stat.h> 
#include <sys/types.h> 

#define PAUSE 0.6

int main()
{
	// initialize file descriptors for pipes
	int fd_insp;	
	int fd_motor_x;	
	int fd_motor_z;
	int fd_motor_x_to_insp;
	int fd_motor_z_to_insp;
	// initialize the temporary file
	char* f_insp = "/tmp/f_insp";
	char* f_motor_x = "/tmp/f_motor_x";
	char* f_motor_z = "/tmp/f_motor_z";
	char* f_motor_x_to_insp = "/tmp/f_motor_x_to_insp";
	char* f_motor_z_to_insp = "/tmp/f_motor_z_to_insp";
	
	// get the PID of the inspection console and send it to the watchdog
	int pid = getpid();		
 	printf("inspection console says: my pid is  %d\n", pid);
 	fflush(stdout);	
 	fd_insp = open(f_insp, O_WRONLY);
	if (fd_insp < 0) {
        perror(f_insp);
        return -1;
    }
 	write(fd_insp, &pid, sizeof(pid));
 	close(fd_insp);

	// initialize variables
	int pidmotorx;
	int pidmotorz;

	// get motor x and motor z PIDS for signal handling
 	fd_motor_x_to_insp = open(f_motor_x_to_insp, O_RDONLY);
	if (fd_motor_x_to_insp < 0) {
        perror(fd_motor_x_to_insp);
        return -1;
    }
 	read(fd_motor_x_to_insp, &pidmotorx, sizeof(pidmotorx));
 	close(fd_motor_x_to_insp);
 	printf("pid motor x: %s\n", pidmotorx);
 	fflush(stdout);

	fd_motor_z_to_insp = open(f_motor_z_to_insp, O_RDONLY);
	if (fd_motor_z_to_insp < 0) {
        perror(fd_motor_z_to_insp);
        return -1;
    }
 	read(fd_motor_z_to_insp, &pidmotorz, sizeof(pidmotorz));
 	close(fd_motor_z_to_insp);
 	printf("pid motor z: %s\n", pidmotorz);
 	fflush(stdout);

	// open pipes used for the select for motors
	fd_motor_x = open(f_motor_x, O_RDONLY);
		if (fd_motor_x < 0) {
        perror(f_motor_x);
        return -1;
    }
	fd_motor_z = open(f_motor_z, O_RDONLY);
		if (fd_motor_z < 0) {
        perror(f_motor_z);
        return -1;
    }
	int retval;
	float pos_x;
	float pos_z;
	fd_set fds;

	//defining select function's variables for signals handling
	char comm_inspect[2];
	int retval_s;
	fd_set fd_in;
 	
	for(;;)
	{
		FD_ZERO(&fd_in);
		FD_SET(STDIN_FILENO, &fd_in);
		struct timeval timeout;
		timeout.tv_sec = 0;
		timeout.tv_usec = 0;

		retval_s = select(STDIN_FILENO+1, &fd_in, NULL, NULL, &timeout);

		if (retval_s == -1)
			perror("select()");
			return -1;

		if (FD_ISSET(STDIN_FILENO, &fd_in))
		{
			read(STDIN_FILENO, &comm_inspect, sizeof(comm_inspect));
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

		int maxfd;
		int res;
		char bufdispx[20];
		char bufdispz[20];
		struct timeval time;

		//sigset_t emptyset, blockset;

		//sigemptyset(&blockset);
		//sigaddset(&blockset, SIGINT);
		//sigprocmask(SIG_BLOCK, &blockset, NULL);

		//sa.sa_handler = handler;
		//sa.sa_flags = 0;
		//sigemptyset(&sa.sa_mask);
		//sigaction(SIGINT, &sa, NULL);

		FD_ZERO(&fds); // clear FD for the select
		FD_SET(fd_motor_x, &fds);
		FD_SET(fd_motor_z, &fds);
		time.tv_sec = 0;
		time.tv_usec = 0;

		maxfd = fd_motor_x > fd_motor_z ? fd_motor_x : fd_motor_z;

		//sigemptyset(&emptyset);
		retval = select(maxfd+1, &fds, NULL, NULL, &time);

		if (retval == -1)
			perror("select()");
			return -1;

		if (FD_ISSET(fd_motor_x, &fds)) // read from motor x file descriptor
		{
			res = read(fd_motor_x, bufdispx, sizeof(bufdispx));
			pos_x = atoi(bufdispx);
		}

		if (FD_ISSET(fd_motor_z, &fds))
		{
			res = read(fd_motor_z, bufdispz, sizeof(bufdispz));
			pos_z = atoi(bufdispz);
		}
		printf("Motor X position: %f;        Motor Z position: %f;\n", pos_x, pos_z);
		sleep(PAUSE);
	}
	
	close(fd_motor_x);
	close(fd_motor_z);
	unlink(f_motor_x);
	unlink(f_motor_z);
	unlink(f_insp);
	unlink(f_motor_z_to_insp);
	unlink(f_motor_x_to_insp);
	return 0;
}