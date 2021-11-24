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
	if (signo==SIGINT)
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
				float err= (((float)rand()/(float)RAND_MAX)*0.001)-0.0005; // cambia sul tuo
				position_x =position_x - STEP + err;
				printf("Resetting position: remaining %f m\n", position_x);
				fflush(stdout);
				sleep(0.5); // bisogna testare questo sleep che potrebbe mandare tutto a puttane 
			}
			position_x=HOME;
			printf("position = %f m\n", position_x);
			fflush(stdout);
			choice_x = 'q';
		}
	}	
}

void sig_handler_stop(int signo)
{
	if (signo1 == SIGTERM)
	{
		position_x = position_x;
		printf("Emergency STOP!!\n position = %f\n", position_x);
		choice_x = 'q';
	}
}

int main()
{
	// handling signals
	signal(SIGHUP, sig_handler_reset);
	signal(SIGTERM, sig_handler_stop);

	// initialize file descriptor for pipe
	int fd_motor_x;	
	int fd_comm_x;
	int fd_motor_x_to_insp;
	// initialize the temporary file
	char* f_motor_x = "/tmp/f_motor_x";
	char* f_comm_x = "/tmp/f_comm_x";
	char* f_motor_x_to_insp = "/tmp/f_motor_x_to_insp";

	// get the PID of the motor x and send it to the watchdog
	int pid_m_x = getpid();	 	
 	printf("motor x says: my pid is  %d\n", pid);
 	fflush(stdout);
	fd_motor_x = open(f_motor_x, O_WRONLY);
	if (fd_motor_x < 0) {
        perror(f_motor_x);
        return -1;
    }
 	write(fd_motor_x, &pid_m_x, sizeof(pid_m_x));
 	close(fd_motor_x);
	
	// send motor x PID to inspection for signal handling
	fd_motor_x_to_insp = open(f_motor_x_to_insp, O_WRONLY);
	if (fd_motor_x_to_insp < 0) {
        perror(f_motor_x_to_insp);
        return -1;
    }
 	write(fd_motor_x_to_insp, &pid_m_x, sizeof(pid_m_x));
 	close(fd_motor_x_to_insp);

	// open pipes used in the select loop
	int retval;
	fd_comm_x = open(f_comm_x, O_RDONLY);
	if (fd_comm_x < 0) {
        perror(f_comm_x);
        return -1;
	fd_motor_x = open(f_motor_x, O_WRONLY);
	if (fd_motor_x < 0) {
        perror(f_motor_x);
        return -1;

	fd_set set; // dentro o fuori loop

	for (;;)
	{	
		struct timeval time;

		FD_ZERO(&set);
		FD_SET(fd_comm_x, &set);
		time.tv_sec = 0;
		time.tv_usec = 0;

		retval = select(fd_comm_x+1, &set, NULL, NULL, &time);

		if (retval == -1)
			perror("select()");
			return -1;

		else if (FD_ISSET(fd_comm_x, &set))
		{
			read(fd_comm_x, &choice_x, sizeof(choice_x));
			switch (choice_x)
		 	{	
		 	case 'a':	 		
				if (position_x<=HOME){
					position_x=HOME;
					printf("Cannot move left: position =%f m\n", position_x);
					fflush(stdout);
					write(fd_motor_x, &position_x, sizeof(position_x));
					}
				else{
					float err= (((float)rand()/(float)RAND_MAX)*0.01)-0.005;
					position_x = position_x - STEP + err;
					printf("the position is %f m\n", position_x);
					fflush(stdout);
					write(fd_motor_x, &position_x, sizeof(position_x));
				}
				break;
				
			case 'd':
				if (position_x>=END){
					position_x=END;
					printf("Cannot move right: position =%f m\n", position_x);
					fflush(stdout);
					write(fd_motor_x, &position_x, sizeof(posizion_x));	
				}
				else{	
					float err= (((float)rand()/(float)RAND_MAX)*0.01)-0.005;
					position_x+= position_x + STEP - err;
					printf("the position is %f m\n", position_x);
					fflush(stdout);
					write(fd_motor_x, &position_x, sizeof(position_x));
				}
				break;
				
			case 'q':
				position_x=position_x;
				printf("motor stopped, the position is %f m\n", position_x);
				fflush(stdout);
				write(fd_motor_x, &position_x, sizeof(position_x));
				break;
				
			default:
				break;			 
		 	}
		}
	}
	
 	close(fd_motor_x);
 	close(fd_comm_x);
	unlink(f_comm_x);
	unlink(f_motor_x);
	unlink(f_motor_x_to_insp);
	return 0;
	
}