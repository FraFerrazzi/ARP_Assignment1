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
#define END 25
#define STEP 0.001

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
char send[20];

void sig_handler_reset(int signo)
{
	if (signo == SIGINT)
	{
	 	position_z = position_z;
		printf("!!!!RESET SIGNAL!!!!");
		if  (position_z == HOME)
		{
			printf("already at home: posiztion = %f\n", position_z);
			fflush(stdout);
		}
		else
		{
			while (position_z >= HOME)
			{
			 	position_z = position_z - STEP;
				float err = (((float)rand()/(float)RAND_MAX)*0.0005)-0.00025;
			 	position_z -= err;
				printf("Resetting position: remaining %f m\n", position_z);
				fflush(stdout);
				sprintf(send, "%f", position_z);
				write(fd_motor_z, &send, sizeof(send));
				sleep(1);
			}
		 	position_z = HOME;
			printf("posiztion = %f m\n", position_z);
			fflush(stdout);
			choice_z = 'z';
		}
	}	
}

void sig_handler_stop(int signo)
{
	if (signo == SIGTERM)
	{
		position_z = position_z;
		printf("Emergency STOP!!\n position = %f\n", position_z);
		choice_z = 'z';
	}
}

int main()
{
	// get the PID of the motor z and send it to the watchdog
	int pid = getpid();
 	printf("motor z says: my pid is  %d\n", pid);
 	fflush(stdout);
	fd_motor_z = open(f_motor_z, O_WRONLY);
 	write(fd_motor_z, &pid, sizeof(pid));
 	close(fd_motor_z);
	//unlink(f_motor_z);

	fd_motor_z_to_insp = open(f_motor_z_to_insp, O_WRONLY);
	if (fd_motor_z_to_insp < 0) 
	{
        perror("f_motor_z_to_insp");
        return -1;
    }
 	write(fd_motor_z_to_insp, &pid, sizeof(pid));
 	close(fd_motor_z_to_insp);

	//REACTION TO THE SIGNALS
	//for reset signal
	struct sigaction sa_reset, sa_stop;
	memset(&sa_reset, 0, sizeof(sa_reset));//set sa to zero using the memset()
	sa_reset.sa_handler = &sig_handler_reset;
	sa_reset.sa_flags = SA_RESTART;
	sigaction(SIGINT, &sa_reset, NULL); 

	//for stop signal
	memset(&sa_stop, 0, sizeof(sa_stop)); //set sa to zero using the memset()
	sa_stop.sa_handler = &sig_handler_stop;
	sa_stop.sa_flags = SA_RESTART;
	sigaction(SIGTERM, &sa_stop, NULL);


	int retval, command;
	fd_comm_z = open(f_comm_z, O_RDONLY);
	fd_motor_z = open(f_motor_z, O_WRONLY);
	
	fd_set set;
	struct timeval time;
		
	for (;;)
	{
				
		FD_ZERO(&set);
		FD_SET(fd_comm_z, &set);
		time.tv_sec = 1;
		time.tv_usec = 0;

		retval = select(fd_comm_z+1, &set, NULL, NULL, &time);

		if (retval == -1)
			perror("select()");

		else if (FD_ISSET(fd_comm_z, &set))
		{
			command = read(fd_comm_z, &choice_z, sizeof(choice_z));
			
			switch (choice_z)
		 	{	
		 	case 'w':	 		
				if  (position_z <= HOME)
				{
				 	position_z = HOME;
					printf("Cannot move up: position =%f m\n", position_z);
					fflush(stdout);
					sprintf(send, "%f", position_z);
					write(fd_motor_z, &send, sizeof(send));	
				}
				else{
				 	position_z = position_z - STEP;
					float err = (((float)rand()/(float)RAND_MAX)*0.0005)-0.00025;
				 	position_z -= err;
					printf("the position is %f m\n", position_z);
					fflush(stdout);
					sprintf(send, "%f", position_z);
					write(fd_motor_z, &send, sizeof(send));
				}
				break;
				
			case 's':
				if  (position_z >= END){
				 	position_z = END;
					printf("Cannot move right: position =%f m\n", position_z);
					fflush(stdout);
					sprintf(send, "%f", position_z);
					write(fd_motor_z, &send, sizeof(send));
				}
				else{	
				 	position_z = position_z + STEP;
					float err = (((float)rand()/(float)RAND_MAX)*0.0005)-0.00025;
				 	position_z -= err;
					printf("the position is %f m\n", position_z);
					fflush(stdout);
					sprintf(send, "%f", position_z);
					write(fd_motor_z, &send, sizeof(send));
				}
				
				break;
				
			case 'z':
			 	position_z = position_z;
				printf("motor stopped, the position is %f m\n", position_z);
				fflush(stdout);
				sprintf(send, "%f", position_z);
				write(fd_motor_z, &send, sizeof(send));
				break;
				
			default:
				break;			 
		 	}
		}
		sleep(1);
	}
	
 	close(fd_motor_z);
 	close(fd_comm_z);
	unlink(f_motor_z);
	return 0;
	
}
