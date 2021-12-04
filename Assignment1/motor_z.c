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
#define END 15 
#define STEP 0.05
// time in microseconds
#define PAUSE 150000

// initialize file descriptor for pipe
int fd_motor_z;	
int fd_comm_z;
int fd_motor_z_to_insp;

// initialize the temporary file
char* f_motor_z = "/tmp/f_motor_z";
char* f_comm_z = "/tmp/f_comm_z";
char* f_motor_z_to_insp = "/tmp/f_motor_z_to_insp";

double position_z = 0;
char choice_z;
char choice_z_bef;
char send[20];

FILE *fp;
time_t rawtime;
struct tm * timeinfo;

// function that handles the RESET SIGNAL when the user gives the command in the inspection console
// this is done to react immediatly to the user command and sets the choice_z to 'r' 
// it's made in order to make the signal handler as atomic as possible
void sig_handler_reset(int signo)
{
	if (signo == SIGINT)
	{
		time ( &rawtime );
  		timeinfo = localtime ( &rawtime );
	 	fprintf(fp, "%sRESET HANDLING STARTED (Motor Z)\n", asctime (timeinfo));
		fflush(fp);
		choice_z = 'r';
	}
}	

// function that handles the STOP SIGNAL when user gives the command in the inspection console
// when the signal comes, the signal is handled by this function which stops immidiatly motor z
// until a new stdandard input comes
void sig_handler_stop(int signo)
{
	if (signo == SIGTERM)
	{
		time ( &rawtime );
  		timeinfo = localtime ( &rawtime );
		fprintf(fp, "%sEMERGENCY STOP!!! (Motor Z)\n\n", asctime (timeinfo));
		fflush(fp);
		position_z = position_z;
		choice_z = 'z';
		choice_z_bef = 'z';
	}
}

int main()
{
	// Open a file pointer named "logfileMotorZ.txt" for writing (w+)
    fp = fopen("./logfile/logfileMotorZ.txt", "w");
	if(fp == NULL)
   	{
    	printf("Error opening the logfileMaster!");   
    	exit(1);             
   	}

	// get the PID of the motor z and send it to the watchdog
	int pid = getpid();
	fd_motor_z = open(f_motor_z, O_WRONLY);
	if (fd_motor_z < 0) 
	{
		fprintf(fp, "Error opening motor z pipe");
		fflush(fp);
        perror("f_motor_z");
        return -1;
    }
 	write(fd_motor_z, &pid, sizeof(pid));
 	close(fd_motor_z);
	time ( &rawtime );
  	timeinfo = localtime ( &rawtime );
	fprintf(fp, "%sMotor Z PID is: %d\n\n", asctime (timeinfo), pid);
	fflush(fp);
	
	// sending motor z PID to inspection console for signal handling
	fd_motor_z_to_insp = open(f_motor_z_to_insp, O_WRONLY);
	if (fd_motor_z_to_insp < 0) 
	{
		fprintf(fp, "Error opening motor z to inspection pipe");
		fflush(fp);
        perror("f_motor_z_to_insp");
        return -1;
    }
 	write(fd_motor_z_to_insp, &pid, sizeof(pid));
 	close(fd_motor_z_to_insp);

	time ( &rawtime );
  	timeinfo = localtime ( &rawtime );
	// printing in the log file that pipes used by motor z for sending and receive PIDS are open
	fprintf(fp, "%sAll pipes used by Motor Z for sending and receiving PIDS are correctly open\n\n", asctime (timeinfo));
	fflush(fp);

	// SIGNAL HANDLING
	// for reset signal
	struct sigaction sa_reset, sa_stop;
	memset(&sa_reset, 0, sizeof(sa_reset));//set sa to zero using the memset()
	sa_reset.sa_handler = &sig_handler_reset;
	sa_reset.sa_flags = SA_RESTART;
	sigaction(SIGINT, &sa_reset, NULL); 

	// for stop signal
	memset(&sa_stop, 0, sizeof(sa_stop)); //set sa to zero using the memset()
	sa_stop.sa_handler = &sig_handler_stop;
	sa_stop.sa_flags = SA_RESTART;
	sigaction(SIGTERM, &sa_stop, NULL);


	int retval, command;
	fd_comm_z = open(f_comm_z, O_RDONLY);
	if (fd_comm_z < 0) 
	{
		fprintf(fp, "Error opening command z pipe");
		fflush(fp);
        perror("f_comm_z");
        return -1;
    }
	fd_motor_z = open(f_motor_z, O_WRONLY);
	if (fd_motor_z < 0) 
	{
		fprintf(fp, "Error opening motor z pipe");
		fflush(fp);
        perror("f_motor_z");
        return -1;
    }

	time ( &rawtime );
  	timeinfo = localtime ( &rawtime );
	// printing in the log file that pipes used by motor z for sending positions and receive instructions are open
	fprintf(fp, "%sAll pipes used by Motor Z for receiving instructions and sending positions are correctly open\n\n", asctime (timeinfo));
	fflush(fp);
	
	fd_set set;
	struct timeval time_s;
	bool print = false;
		
	for (;;)
	{		
		FD_ZERO(&set);  // clears set
		FD_SET(fd_comm_z, &set);  // adds the file descriptor fd_comm_x to set
		time_s.tv_sec = 0;
		time_s.tv_usec = 0;
		// select() allows a program to monitor multiple file descriptors,
        // waiting until one or more of the file descriptors become "ready"
        // for some class of I/O operation 
		// A file descriptor is considered ready if it is possible to perform a
        // corresponding I/O operation without blocking.
		// On success, select() returns the number of file descriptors contained
		// in the three returned descriptor sets. On error returns -1
		retval = select(fd_comm_z+1, &set, NULL, NULL, &time_s);
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
		else if (FD_ISSET(fd_comm_z, &set))
		{
			// motor z reads the user's choice from command console
			command = read(fd_comm_z, &choice_z, sizeof(choice_z));
		}
		// decide if the state of the motor will be printed in log file
		// if the command at the previous loop is different from the new loop, then
		// it means that the user gave a command and it should be printed in che log file
		if (choice_z != choice_z_bef)
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
		switch (choice_z)
		{	
	 	case 'w': // case in which the motor moves up	
			if  (position_z <= HOME) // if position less than home (lower bound), set position to home
			{
			 	position_z = HOME;
				sprintf(send, "%f", position_z);
				write(fd_motor_z, &send, sizeof(send));
				if (print)
				{
					time ( &rawtime );
  					timeinfo = localtime ( &rawtime );
					fprintf(fp, "%sMotor z is in HOME position\n\n", asctime (timeinfo));
					fflush(fp);	
				}
				choice_z_bef = 'w';
			}
			else  // if position is not home, move towards home
			{
				position_z = position_z - STEP;
				float err = (((float)rand()/(float)RAND_MAX)*0.025)-0.0125;
			 	position_z -= err;
				sprintf(send, "%f", position_z);
				write(fd_motor_z, &send, sizeof(send));
				if (print)
				{
					time ( &rawtime );
  					timeinfo = localtime ( &rawtime );
					fprintf(fp, "%sMotor z is moving up\n\n", asctime (timeinfo));
					fflush(fp);	
				}
				choice_z_bef = 'w';
			}
			break;
				
		case 's': // case in which the motor moves right
			if  (position_z >= END) // if position more than end (higher bound), set position to end
			{
				position_z = END;
				sprintf(send, "%f", position_z);
				write(fd_motor_z, &send, sizeof(send));
				if (print)
				{
					time ( &rawtime );
  					timeinfo = localtime ( &rawtime );
					fprintf(fp, "%sMotor z is in END position\n\n", asctime (timeinfo));
					fflush(fp);
				}
				choice_z_bef = 's';
			}
			else // if position is not end, move towards end
			{	
			 	position_z = position_z + STEP;
				float err = (((float)rand()/(float)RAND_MAX)*0.025)-0.0125;
			 	position_z -= err;
				sprintf(send, "%f", position_z);
				write(fd_motor_z, &send, sizeof(send));
				if (print)
				{
					time ( &rawtime );
  					timeinfo = localtime ( &rawtime );
					fprintf(fp, "%sMotor z is moving down\n\n", asctime (timeinfo));
					fflush(fp);
				}
				choice_z_bef = 's';
			}
			break;
				
		case 'z': // case in which the motor stops 
			position_z = position_z;
			sprintf(send, "%f", position_z);
			write(fd_motor_z, &send, sizeof(send));
			if (print)
			{
				time ( &rawtime );
  				timeinfo = localtime ( &rawtime );
				fprintf(fp, "%sMotor z has been stopped!\n\n", asctime (timeinfo));
				fflush(fp);
			}
			choice_z_bef = 'z';
			break;

		case 'r': // case in which the motor reset
		// this is done in order to make the signal handler as atomic as possible
			if (position_z >= HOME)
			{
				position_z = position_z-STEP;
				float err= (((float)rand()/(float)RAND_MAX)*0.025)-0.0125;
				position_z -= err;
				sprintf(send, "%f", position_z);
				write(fd_motor_z, &send, sizeof(send));
				if (print)
				{
					fprintf(fp, "Motor z is RESETTING!\n\n");
					fflush(fp);
				}
				choice_z_bef = 'r';
			}
			else
			{
				position_z = HOME;
				sprintf(send, "%f", position_z);
				write(fd_motor_z, &send, sizeof(send));
				if (print)
				{
					fprintf(fp, "Motor z arrived to HOME POSITION!\n\n");
					fflush(fp);
				}
				choice_z_bef = 'r';
			}
			break; 
				
		default: // if any command comes which is not w,s or z is handled by this
			break;			 
		}
		usleep(PAUSE);
	}
	
 	close(fd_motor_z);
 	close(fd_comm_z);
	unlink(f_motor_z);
	return 0;
	
}
