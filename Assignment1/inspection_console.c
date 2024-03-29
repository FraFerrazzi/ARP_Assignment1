#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h> 
#include <fcntl.h> 
#include <signal.h>
#include <sys/stat.h> 
#include <sys/types.h> 
#include <time.h>

void print_ui();

#define PAUSE 150000

int main()
{
	time_t rawtime;
	struct tm * timeinfo;
	// Open a file pointer named "logfileInspectionCons.txt" for writing (w+)
	FILE *fp; 
	fp = fopen("./logfile/logfileInspectionCons.txt", "w");
	if(fp == NULL)
   	{
    	printf("Error opening the logfileMaster!");   
    	exit(1);             
   	}

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
	if (fd_motor_x_to_insp < 0) 
	{
		fprintf(fp, "Error opening motor x to inspection pipe");
		fflush(fp);
        perror("fd_motor_x_to_insp");
        return -1;
    }
 	read(fd_motor_x_to_insp, &pidmotorx, sizeof(pidmotorx));
 	close(fd_motor_x_to_insp);

	// get motor z PID for signal handling
	int pidmotorz;
	fd_motor_z_to_insp = open(f_motor_z_to_insp, O_RDONLY);
	if (fd_motor_z_to_insp < 0) 
	{
		fprintf(fp, "Error opening motor z to inspection pipe");
		fflush(fp);
        perror("fd_motor_z_to_insp");
        return -1;
    }
 	read(fd_motor_z_to_insp, &pidmotorz, sizeof(pidmotorz));
 	close(fd_motor_z_to_insp);
	
	// get watchdog's PID for signal handling
	int pidwatchdog;
	fd_insp = open(f_insp, O_RDONLY);
	if (fd_insp < 0) 
	{
		fprintf(fp, "Error opening inspection pipe");
		fflush(fp);
        perror("fd_insp");
        return -1;
    }
 	read(fd_insp, &pidwatchdog, sizeof(pidwatchdog));
 	close(fd_insp);
	
	time ( &rawtime );
  	timeinfo = localtime ( &rawtime );
	// printing PIDS in the log file
	fprintf(fp, "%sMotor X PID is: %d\n", asctime (timeinfo), pidmotorx);
	fprintf(fp, "Motor Z PID is: %d\n", pidmotorz);
	fprintf(fp, "Watchdog PID is: %d\n\n", pidwatchdog);
	fflush(fp);

	time ( &rawtime );
  	timeinfo = localtime ( &rawtime );
	// printing in the log file that pipes used by inspection console for sending and receive PIDS are open
	fprintf(fp, "%sAll pipes used by Inspection Console for sending and receiving PIDS are correctly open\n\n", asctime (timeinfo));
	fflush(fp);

	// open pipes to communicate with motors
	fd_motor_x = open(f_motor_x, O_RDONLY);
	if (fd_motor_x < 0) 
	{
		fprintf(fp, "Error opening motor x pipe");
		fflush(fp);
        perror("fd_motor_x");
        return -1;
    }
	fd_motor_z = open(f_motor_z, O_RDONLY);
	if (fd_motor_z < 0) 
	{
		fprintf(fp, "Error opening motor z pipe");
		fflush(fp);
        perror("fd_motor_z");
        return -1;
    }

	time ( &rawtime );
  	timeinfo = localtime ( &rawtime );
	// printing in the log file that pipes used by inspection console for receiving positions are open
	fprintf(fp, "%sAll pipes used by inspection console for receiving positions are correctly open\n\n", asctime (timeinfo));
	fflush(fp);

	// initialize variables
	char comm_inspect;
	int count;
	int retval_s;
	int retval;
	fd_set fd_in;
	fd_set fds;
	float pos_x = 0;
	float pos_z = 0;
	char bufdispx[20];
	char bufdispz[20];
	struct timeval timeout;
	struct timeval time_s;

	for(;;)
	{
		FD_ZERO(&fd_in); // clear fd_in for the select
		FD_SET(STDIN_FILENO, &fd_in);
		timeout.tv_sec = 0;
		timeout.tv_usec = 0;
		// select() that handles standard inputs from inspection console
		retval_s = select(STDIN_FILENO+1, &fd_in, NULL, NULL, &timeout);
		// handle select() error
		if (retval_s == -1)
		{
			time ( &rawtime );
  			timeinfo = localtime ( &rawtime );
			fprintf(fp, "%sError in select() function for input from inspection", asctime (timeinfo));
			fflush(fp);
			perror("select()");
			return -1;
		}

		if (FD_ISSET(STDIN_FILENO, &fd_in)) // returns non zero if a fd is still present in the set
		{
			// if user types something in the inspection console, reset watchdog timer
			kill(pidwatchdog, SIGUSR1);
			time ( &rawtime );
  			timeinfo = localtime ( &rawtime );
			fprintf(fp, "%sUser typed something in the inspection console: SIGNAL IS SEND TO WATCHDOG TO RESET COUNTER\n\n", asctime (timeinfo));
			fflush(fp);
			read(STDIN_FILENO, &comm_inspect, sizeof(comm_inspect));
			switch(comm_inspect)
			{
				case 'r': // Reset case
				time ( &rawtime );
  				timeinfo = localtime ( &rawtime );
				fprintf(fp, "\n%sRESET SIGNAL IS SENT TO MOTORS\n\n", asctime (timeinfo));
				fflush(fp);
				kill(pidmotorx, SIGINT); // send signal to motor x
				kill(pidmotorz, SIGINT); // send signal to motor z
				break;

				case 'e': // Emergency stop case
				time ( &rawtime );
  				timeinfo = localtime ( &rawtime );
				fprintf(fp, "\n%sEMERGENCY STOP SIGNAL IS SENT TO MOTORS\n\n", asctime (timeinfo));
				fflush(fp);
				kill(pidmotorx, SIGTERM); // send signal to motor x
				kill(pidmotorz, SIGTERM); // send signal to motor z
				break;
				
				default:
				break;
			}
				
		}

		int maxfd;
		count ++;

		FD_ZERO(&fds); // clear fds for the select
		FD_SET(fd_motor_x, &fds);
		FD_SET(fd_motor_z, &fds);
		time_s.tv_sec = 0;
		time_s.tv_usec = 0;

		maxfd = fd_motor_x > fd_motor_z ? fd_motor_x : fd_motor_z;

		// select() that handles positions sent from motor x and motor z
		retval = select(maxfd+1, &fds, NULL, NULL, &time_s);
		// handle select() error
		if (retval == -1)
		{
			time ( &rawtime );
  			timeinfo = localtime ( &rawtime );
			fprintf(fp, "%sError in select() function for receiving position from motors\n\n", asctime (timeinfo));
			fflush(fp);
			perror("select()");
			return -1;
		}

		if (FD_ISSET(fd_motor_x, &fds)) // if not zero, read from motor x buffer
		{
			read(fd_motor_x, bufdispx, sizeof(bufdispx));
			pos_x = atof(bufdispx);
		}

		if (FD_ISSET(fd_motor_z, &fds)) // if not zero, read from motor z buffer
		{
			read(fd_motor_z, bufdispz, sizeof(bufdispz));
			pos_z = atof(bufdispz);
		}
		// call the function for printing commands that user can use
		print_ui();
		printf("Motor X position: %f;        Motor Z position: %f;\n", pos_x, pos_z); // print positions in the inspection console
		fflush(stdout);
		// printing the updated position of motors in the log file just once every seven times 
		if (count%7 == 0)
		{
			time ( &rawtime );
  			timeinfo = localtime ( &rawtime );
			fprintf(fp, "%sMotor X position: %f;        Motor Z position: %f;\n\n", asctime (timeinfo), pos_x, pos_z); // print positions in the inspection console file log
			fflush(fp);
			count = 0;
		}
		usleep(PAUSE);
		printf("\e[1;1H\e[2J");
	}
	
	close(fd_motor_x);
	close(fd_motor_z);
	unlink(f_motor_x);
	unlink(f_motor_z);
	return 0;
}


// function that displays the possible commands that user can use
// if user decides to use anything else, the switcher will handle the exception
void print_ui()
{
	printf("R = RESET motors");
	printf("        E = EMERGENCY STOP\n\n\n");
	fflush(stdout);
}