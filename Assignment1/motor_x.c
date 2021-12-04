#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h> 
#include <fcntl.h> 
#include <signal.h>
#include <sys/stat.h> 
#include <sys/types.h> 
#include <stdbool.h>
#include <time.h>

// mesures in meters
#define HOME 0
#define END 25 
#define STEP 0.05
// time in microseconds
#define PAUSE 150000

//initialize the temporary files
char* f_motor_x = "/tmp/f_motor_x";
char* f_comm_x = "/tmp/f_comm_x";
char* f_motor_x_to_insp = "/tmp/f_motor_x_to_insp";
// initialize file descriptor for pipe
int fd_motor_x;	
int fd_comm_x;
int fd_motor_x_to_insp;

double position_x = 0;
char choice_x;
char choice_x_bef;
char send[20];

FILE *fp;
time_t rawtime;
struct tm * timeinfo;

// function that handles the RESET SIGNAL when the user gives the command in the inspection console
// this is done to react immediatly to the user command and sets the choice_x to 'r' 
// it's made in order to make the signal handler as atomic as possible
void sig_handler_reset(int signo)
{
	if (signo == SIGINT)
	{
		time ( &rawtime );
  		timeinfo = localtime ( &rawtime );
		fprintf(fp, "%sRESET HANDLING STARTED (Motor X)\n", asctime (timeinfo));
		fflush(fp);
		choice_x = 'r';
	}
}	

// function that handles the STOP SIGNAL when user gives the command in the inspection console
// when the signal comes, the signal is handled by this function which stops immidiatly motor x
// until a new stdandard input comes
void sig_handler_stop(int signo)
{
	if (signo == SIGTERM)
	{
		time ( &rawtime );
  		timeinfo = localtime ( &rawtime );
		fprintf(fp, "%sEMERGENCY STOP!!! (Motor X)\n\n", asctime (timeinfo));
		fflush(fp);
		position_x = position_x;
		choice_x = 'q';
		choice_x_bef = 'q';
	}
}

int main()
{
	// Open a file pointer named "logfileMotorX.txt" for writing (w+)
    fp = fopen("./logfile/logfileMotorX.txt", "w");
	if(fp == NULL)
   	{
    	printf("Error opening the logfileMaster!");   
    	exit(1);             
   	}
			
	// get the PID of the motor x and send it to the watchdog	
	int pid = getpid();
	fd_motor_x = open(f_motor_x, O_WRONLY);
	if (fd_motor_x < 0) 
	{
		fprintf(fp, "Error opening motor x pipe");
		fflush(fp);
        perror("f_motor_x");
        return -1;
    }
 	write(fd_motor_x, &pid, sizeof(pid));
 	close(fd_motor_x);
	time ( &rawtime );
  	timeinfo = localtime ( &rawtime );
	fprintf(fp, "%sMotor X PID is: %d\n\n", asctime (timeinfo), pid);
	fflush(fp);
	
	// sending motor x PID to inspection console for signal handling
	fd_motor_x_to_insp = open(f_motor_x_to_insp, O_WRONLY);
	if (fd_motor_x_to_insp < 0) 
	{
		fprintf(fp, "Error opening motor x to inspection pipe");
		fflush(fp);
        perror("f_motor_x_to_insp");
        return -1;
    }
 	write(fd_motor_x_to_insp, &pid, sizeof(pid));
 	close(fd_motor_x_to_insp);

	time ( &rawtime );
  	timeinfo = localtime ( &rawtime );
	// printing in the log file that pipes used by motor x for sending and receive PIDS are open
	fprintf(fp, "%sAll pipes used by Motor X for sending and receiving PIDS are correctly open\n\n", asctime (timeinfo));
	fflush(fp);
	 
	// SIGNAL HANDLING
	// for reset signal
	struct sigaction sa_reset, sa_stop;
	memset(&sa_reset, 0, sizeof(sa_reset)); //set sa to zero using the memset()
	sa_reset.sa_handler = &sig_handler_reset;
	sa_reset.sa_flags = SA_RESTART;
	sigaction(SIGINT, &sa_reset, NULL); 
	// for stop signal
	memset(&sa_stop, 0, sizeof(sa_stop)); //set sa to zero using the memset()
	sa_stop.sa_handler = &sig_handler_stop;
	sa_stop.sa_flags = SA_RESTART;
	sigaction(SIGTERM, &sa_stop, NULL);
	
	int retval, command;
	fd_comm_x = open(f_comm_x, O_RDONLY);
	if (fd_comm_x < 0) 
	{
		fprintf(fp, "Error opening command x pipe");
		fflush(fp);
        perror("f_comm_x");
        return -1;
    }
	fd_motor_x = open(f_motor_x, O_WRONLY);
	if (fd_motor_x < 0) 
	{
		fprintf(fp, "Error opening motor x pipe");
		fflush(fp);
        perror("f_motor_x");
        return -1;
    }

	time ( &rawtime );
  	timeinfo = localtime ( &rawtime );
	// printing in the log file that pipes used by motor x for sending positions and receive instructions are open
	fprintf(fp, "%sAll pipes used by Motor X for receiving instructions and sending positions are correctly open\n\n", asctime (timeinfo));
	fflush(fp);
	
	fd_set set;
	struct timeval time_s; 
	bool print = false;

	for (;;)
	{
		FD_ZERO(&set); // clears set
		FD_SET(fd_comm_x, &set); // adds the file descriptor fd_comm_x to set
		time_s.tv_sec = 0;
		time_s.tv_usec = 0;
		// select() allows a program to monitor multiple file descriptors,
        // waiting until one or more of the file descriptors become "ready"
        // for some class of I/O operation 
		// A file descriptor is considered ready if it is possible to perform a
        // corresponding I/O operation without blocking.
		// On success, select() returns the number of file descriptors contained
		// in the three returned descriptor sets. On error returns -1
		retval = select(fd_comm_x+1, &set, NULL, NULL, &time_s);
		// handle select() error
		if (retval == -1)
		{
			time ( &rawtime );
  			timeinfo = localtime ( &rawtime );
			fprintf(fp, "%sError in select() function", asctime (timeinfo));
			fflush(fp);
			perror("select()");
			return -1;
		}
		// FD_ISSET() is used to see if a file descriptor is present in the set
		// returns non zero if a fd is still present in the set
		else if (FD_ISSET(fd_comm_x, &set))
		{
			// motor x reads the user's choice from command console
			command = read(fd_comm_x, &choice_x, sizeof(choice_x));	
		}
		// decide if the state of the motor will be printed in log file
		// if the command at the previous loop is different from the new loop, then
		// it means that the user gave a command and it should be printed in che log file
		if (choice_x != choice_x_bef)
		{
			print = true;
		}
		else
		{
			print = false;
		}
		// switch handles the decision
		// if command did't change keeps doing what the last command was
		/// if command changes, motor reads it and switcher handles the new command
		switch (choice_x)
	 	{	
	 	case 'a':	// case in which the motor moves left 		
			if (position_x <= HOME)  // if position less than home (lower bound), set position to home
			{
				position_x = HOME;
				sprintf(send, "%f", position_x);
				write(fd_motor_x, &send, sizeof(send));
				if (print)
				{
					time ( &rawtime );
  					timeinfo = localtime ( &rawtime );
					fprintf(fp, "%sMotor x is in HOME position\n\n", asctime (timeinfo));
					fflush(fp);
				}
				choice_x_bef = 'a';
			}
			else   // if position is not home, move towards home
			{  
				position_x = position_x - STEP;
				float err = (((float)rand()/(float)RAND_MAX)*0.025)-0.0125;
				position_x -= err;
				sprintf(send, "%f", position_x);
				write(fd_motor_x, &send, sizeof(send));
				if (print)
				{
					time ( &rawtime );
  					timeinfo = localtime ( &rawtime );
					fprintf(fp, "%sMotor x is in moving left\n\n", asctime (timeinfo));
					fflush(fp);
				}
				choice_x_bef = 'a';
			}
			break;
				
		case 'd': // case in which the motor moves right
			if (position_x >= END) // if position more than end (higher bound), set position to end
			{
				position_x = END;
				sprintf(send, "%f", position_x);
				write(fd_motor_x, &send, sizeof(send));	
				if (print)
				{
					time ( &rawtime );
  					timeinfo = localtime ( &rawtime );
					fprintf(fp, "%sMotor x is in END position\n\n", asctime (timeinfo));
					fflush(fp);
				}
				choice_x_bef = 'd';
			}
			else // if position is not end, move towards end
			{	
				position_x = position_x+STEP;
				float err = (((float)rand()/(float)RAND_MAX)*0.025)-0.0125;
				position_x -= err;
				sprintf(send, "%f", position_x);
				write(fd_motor_x, &send, sizeof(send));
				if (print)
				{
					time ( &rawtime );
  					timeinfo = localtime ( &rawtime );
					fprintf(fp, "%sMotor x is in moving right\n\n", asctime (timeinfo));
					fflush(fp);
				}
				choice_x_bef = 'd';
			}
			break;
				
		case 'q': // case in which the motor stops 
			position_x = position_x;
			sprintf(send, "%f", position_x);
			write(fd_motor_x, &send, sizeof(send));
			if (print)
			{
				time ( &rawtime );
  				timeinfo = localtime ( &rawtime );
				fprintf(fp, "%sMotor x has been stopped!\n\n", asctime (timeinfo));
				fflush(fp);
			}
			choice_x_bef = 'q';
			break;

		case 'r': // case in which the motor reset
		// this is done in order to make the signal handler as atomic as possible
			if (position_x >= HOME)
			{
				position_x = position_x-STEP;
				float err= (((float)rand()/(float)RAND_MAX)*0.025)-0.0125;
				position_x -= err;
				sprintf(send, "%f", position_x);
				write(fd_motor_x, &send, sizeof(send));
				if (print)
				{
					fprintf(fp, "Motor x is RESETTING!\n\n");
					fflush(fp);
				}
				choice_x_bef = 'r';
			}
			else
			{
				position_x = HOME;
				sprintf(send, "%f", position_x);
				write(fd_motor_x, &send, sizeof(send));
				if (print)
				{
					fprintf(fp, "Motor x is in HOME position!\n\n");
					fflush(fp);
				}
				choice_x_bef = 'r';
			}
			break; 

		default: // if any command comes which is not w,s or z is handled by this
			break;			 
	 	}
		// little pause 
		usleep(PAUSE);
	}
	
 	close(fd_motor_x);
 	close(fd_comm_x);
	unlink(f_motor_x);
	return 0;
	
}
