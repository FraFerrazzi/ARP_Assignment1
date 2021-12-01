#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h> 
#include <fcntl.h> 
#include <signal.h>
#include <sys/stat.h> 
#include <sys/types.h> 
#include <stdbool.h>

#define HOME 0
#define END 30
#define STEP 0.01
#define PAUSE 10000

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
char send[20];

void sig_handler_reset(int signo)
{
	if (signo == SIGINT)
	{
		printf("!!!!RESET SIGNAL!!!!");
		while(position_x >= HOME || signo == SIGTERM)
		{
			position_x = position_x-STEP;
			float err= (((float)rand()/(float)RAND_MAX)*0.005)-0.0025;
			position_x -= err;
			printf("Resetting position: remaining %f m\n", position_x);
			fflush(stdout);
			sprintf(send, "%f", position_x);
			write(fd_motor_x, &send, sizeof(send));
			usleep(PAUSE);
		}
		if (position_x <= HOME)
		{
			position_x = HOME;
			printf("position = %f m\n", position_x);
			fflush(stdout);
			sprintf(send, "%f", position_x);
			write(fd_motor_x, &send, sizeof(send));
			usleep(PAUSE);
		}
		else
		{
			position_x = position_x;
			printf("position = %f m\n", position_x);
			fflush(stdout);
			sprintf(send, "%f", position_x);
			write(fd_motor_x, &send, sizeof(send));
			usleep(PAUSE);
		}
		choice_x = 'q';
	}
}	


void sig_handler_stop(int signo)
{
	if (signo == SIGTERM)
	{
		position_x = position_x;
		printf("Emergency STOP!!\n position = %f\n", position_x);
		choice_x = 'q';
	}
}

int main()
{
	// Open a file pointer named "logfileMotorX.txt" for writing (w+)
	FILE *fp;
    fp = fopen("./logfile/logfileMotorX.txt", "w");
	if(fp == NULL)
   	{
    	printf("Error opening the logfileMaster!");   
    	exit(1);             
   	}
			
	// get the PID of the motor x and send it to the watchdog	
	int pid = getpid();
 	printf("motor x says: my pid is  %d\n", pid);
 	fflush(stdout);
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

	// printing in the log file that pipes used by motor x for sending and receive PIDS are open
	fprintf(fp, "All pipes used by Motor X for sending and receiving PIDS are correctly open\n");
	fflush(fp);
	 
	//REACTION TO THE SIGNALS
	//for reset signal
	struct sigaction sa_reset, sa_stop;
	memset(&sa_reset, 0, sizeof(sa_reset)); //set sa to zero using the memset()
	sa_reset.sa_handler = &sig_handler_reset;
	sa_reset.sa_flags = SA_RESTART;
	sigaction(SIGINT, &sa_reset, NULL); 
	//for stop signal
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

	// printing in the log file that pipes used by motor x for sending positions and receive instructions are open
	fprintf(fp, "All pipes used by Motor X for receiving instructions and sending positions are correctly open\n");
	fflush(fp);
	
	fd_set set;

	for (;;)
	{
		
		struct timeval time; 
		FD_ZERO(&set); // clears set
		FD_SET(fd_comm_x, &set); // adds the file descriptor fd_comm_x to set
		time.tv_sec = 1;
		time.tv_usec = 0;
		// select() allows a program to monitor multiple file descriptors,
        // waiting until one or more of the file descriptors become "ready"
        // for some class of I/O operation 
		// A file descriptor is considered ready if it is possible to perform a
        // corresponding I/O operation without blocking.
		// On success, select() returns the number of file descriptors contained
		// in the three returned descriptor sets. On error returns -1
		retval = select(fd_comm_x+1, &set, NULL, NULL, &time);
		// handle select() error
		if (retval == -1){
			fprintf(fp, "Error opening command x pipe");
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
			
			switch (choice_x)
		 	{	
		 	case 'a':	// case in which the motor moves left 		
				if (position_x <= HOME)  // if position less than home (lower bound), set position to home
				{
					position_x = HOME;
					printf("Cannot move left: position =%f m\n", position_x);
					fflush(stdout);
					sprintf(send, "%f", position_x);
					write(fd_motor_x, &send, sizeof(send));
				}
				else   // if position is not home, move towards home
				{  
					position_x = position_x - STEP;
					float err = (((float)rand()/(float)RAND_MAX)*0.005)-0.0025;
					position_x -= err;
					printf("the position is %f m\n", position_x);
					fflush(stdout);
					sprintf(send, "%f", position_x);
					write(fd_motor_x, &send, sizeof(send));
				}
				break;
				
			case 'd': // case in which the motor moves right
				if (position_x >= END) // if position more than end (higher bound), set position to end
				{
					position_x = END;
					printf("Cannot move right: position =%f m\n", position_x);
					fflush(stdout);
					sprintf(send, "%f", position_x);
					write(fd_motor_x, &send, sizeof(send));	
				}
				else // if position is not end, move towards end
				{	
					position_x = position_x+STEP;
					float err = (((float)rand()/(float)RAND_MAX)*0.005)-0.0025;
					position_x -= err;
					printf("the position is %f m\n", position_x);
					fflush(stdout);
					sprintf(send, "%f", position_x);
					write(fd_motor_x, &send, sizeof(send));
				}
				
				break;
				
			case 'q': // case in which the motor stops 
				position_x = position_x;
				printf("motor stopped, the position is %f m\n", position_x);
				fflush(stdout);
				sprintf(send, "%f", position_x);
				write(fd_motor_x, &send, sizeof(send));
				break;
				
			default:
				break;			 
		 	}
		}
		usleep(PAUSE);
	}
	
 	close(fd_motor_x);
 	close(fd_comm_x);
	unlink(f_motor_x);
	return 0;
	
}
