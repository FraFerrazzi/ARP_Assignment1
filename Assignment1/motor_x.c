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
#define END 50
#define STEP 0.001

double position_x = 0;
char choice_x;

void sig_handler_reset(int signo)
{
	if (signo == SIGINT)
	{
		position_x=position_x;
		printf("!!!!RESET SIGNAL!!!!");
		if (position_x == HOME)
		{
			printf("already at home: position = %f\n", position_x);
			fflush(stdout);
		}
		else
		{
			while(position_x >= HOME)
			{
				position_x = position_x-STEP;
				float err= (((float)rand()/(float)RAND_MAX)*0.0005)-0.00025;
				position_x -= err;
				printf("Resetting position: remaining %f m\n", position_x);
				fflush(stdout);
				sleep(0.5);
			}
			position_x = HOME;
			printf("position = %f m\n", position_x);
			fflush(stdout);
			choice_x = 'q';
		}
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
	//initialize the temporary files
	char* f_motor_x = "/tmp/f_motor_x";
	char* f_comm_x = "/tmp/f_comm_x";
	char* f_motor_x_to_insp = "/tmp/f_motor_x_to_insp";
	// initialize file descriptor for pipe
	int fd_motor_x;	
	int fd_comm_x;
	int fd_motor_x_to_insp;
			
	// get the PID of the motor x and send it to the watchdog	
	int pid = getpid();
 	printf("motor x says: my pid is  %d\n", pid);
 	fflush(stdout);
	fd_motor_x = open(f_motor_x, O_WRONLY);
 	write(fd_motor_x, &pid, sizeof(pid));
 	close(fd_motor_x);
	//unlink(f_motor_x);
	printf("jisdhfiuw");
	fflush(stdout);

	 //sending motor x pid to inspection console for signal handling

	fd_motor_x_to_insp = open(f_motor_x_to_insp, O_WRONLY);
	if (fd_motor_x_to_insp < 0) 
	{
        perror("f_motor_x_to_insp");
        return -1;
    }
 	write(fd_motor_x_to_insp, &pid, sizeof(pid));
 	close(fd_motor_x_to_insp);
	 
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
	fd_motor_x = open(f_motor_x, O_WRONLY);
	char send[20];
	fd_set set;

	for (;;)
	{
		
		struct timeval time;
		FD_ZERO(&set);
		FD_SET(fd_comm_x, &set);
		time.tv_sec = 1;
		time.tv_usec = 0;
		retval = select(fd_comm_x+1, &set, NULL, NULL, &time);

		if (retval == -1)
			perror("select()");

		else if (FD_ISSET(fd_comm_x, &set))
		{
			command = read(fd_comm_x, &choice_x, sizeof(choice_x));
			
			switch (choice_x)
		 	{	
		 	case 'a':	 		
				if (position_x <= HOME){
					position_x = HOME;
					printf("Cannot move left: position =%f m\n", position_x);
					fflush(stdout);
					sprintf(send, "%f", position_x);
					write(fd_motor_x, &send, sizeof(send));
					
					}
				else{
					position_x = position_x - STEP;
					float err = (((float)rand()/(float)RAND_MAX)*0.0005)-0.00025;
					position_x -= err;
					printf("the position is %f m\n", position_x);
					fflush(stdout);
					sprintf(send, "%f", position_x);
					write(fd_motor_x, &send, sizeof(send));
				}
				break;
				
			case 'd':
				if (position_x >= END){
					position_x = END;
					printf("Cannot move right: position =%f m\n", position_x);
					fflush(stdout);
					sprintf(send, "%f", position_x);
					write(fd_motor_x, &send, sizeof(send));	
				}
				else{	
					position_x = position_x+STEP;
					float err = (((float)rand()/(float)RAND_MAX)*0.0005)-0.00025;
					position_x -= err;
					printf("the position is %f m\n", position_x);
					fflush(stdout);
					sprintf(send, "%f", position_x);
					write(fd_motor_x, &send, sizeof(send));
				}
				
				break;
				
			case 'q':
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
	}
	
 	close(fd_motor_x);
 	close(fd_comm_x);
	unlink(f_motor_x);
	return 0;
	
}
