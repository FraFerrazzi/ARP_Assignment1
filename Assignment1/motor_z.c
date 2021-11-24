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

double position_z = 0;
char choice_z;

void sig_handler_reset(int signo)
{
	if (signo==SIGINT)
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
			while (position_z>=HOME)
			{
				float err= (((float)rand()/(float)RAND_MAX)*0.001)-0.0005; // cambia nel tuo
			 	position_z = position_z - STEP + err;
				printf("Resetting posiztion: remaining %f m\n", position_z);
				fflush(stdout);
				sleep(0.5); // vedere se non sbarella
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
	if (signo1 == SIGTERM)
	{
		position_z = position_z;
		printf("Emergency STOP!!\n position = %f\n", position_z);
		choice_z = 'z';
	}
}

int main()
{
	// signal handling
	signal(SIGHUP, sig_handler_reset);
	signal(SIGTERM, sig_handler_stop);

	// initialize file descriptor for pipe
	int fd_motor_z;	
	int fd_comm_z;
	int fd_motor_z_to_insp;
	// initialize the temporary file
	char* f_motor_z = "/tmp/f_motor_z";
	char* f_comm_z = "/tmp/f_comm_z";
	char* f_motor_z_to_insp = "/tmp/f_motor_z_to_insp";

	// get the PID of the motor z and position_z it to the watchdog
	int pid_m_z = getpid(); 	
 	printf("motor z says: my pid is  %d\n", pid_m_z);
 	fflush(stdout);
	fd_motor_z = open(f_motor_z, O_WRONLY);
	if (fd_motor_z < 0) {
        perror(f_motor_z);
        return -1;
    }
 	write(fd_motor_z, &pid_m_z, sizeof(pid_m_z));
 	close(fd_motor_z);

	// position_z motor z PID to inspection for signal handling
	fd_motor_z_to_insp = open(f_motor_z_to_insp, O_WRONLY);
	if (fd_motor_z_to_insp < 0) {
        perror(f_motor_z_to_insp);
        return -1;
    }
 	write(fd_motor_z_to_insp, &pid_m_z, sizeof(pid_m_z));
 	close(fd_motor_z_to_insp);

	// open pipes used in the select loop
	int retval;
	fd_comm_z = open(f_comm_z, O_RDONLY);
	if (fd_comm_z < 0) {
        perror(f_comm_z);
        return -1;
	fd_motor_z = open(f_motor_z, O_WRONLY);
	if (fd_motor_x < 0) {
        perror(f_motor_x);
        return -1;

	fd_set set; // in caso da mettere nel loop

	for (;;)
	{	
		struct timeval time;
		
		FD_ZERO(&set);
		FD_SET(fd_comm_z, &set);
		time.tv_sec = 0;
		time.tv_usec = 0;

		retval = select(fd_comm_z+1, &set, NULL, NULL, &time);

		if (retval == -1)
			perror("select()");
			return -1;

		else if (FD_ISSET(fd_comm_z, &set))
		{
			read(fd_comm_z, &choice_z, sizeof(choice_z));
			
			switch (choice_z)
		 	{	
		 	case 'w':	 		
				if  (position_z<=HOME){
				 	position_z=HOME;
					printf("Cannot move up: position =%f m\n", position_z);
					write(fd_motor_z, &position_z, sizeof(position_z));	
				}
				else{
					float err= (((float)rand()/(float)RAND_MAX)*0.01)-0.005;
				 	position_z = position_z - STEP err;
					printf("the position is %f m\n", position_z);
					fflush(stdout);
					write(fd_motor_z, &position_z, sizeof(position_z));
				}
				break;
				
			case 's':
				if  (position_z>=END){
				 	position_z=END;
					printf("Cannot move right: position =%f m\n", position_z);
					fflush(stdout);
					write(fd_motor_z, &position_z, sizeof(position_z));
				}
				else{	
					float err= (((float)rand()/(float)RAND_MAX)*0.01)-0.005;
				 	position_z = position_z + STEP -  err;
					printf("the position is %f m\n", position_z);
					fflush(stdout);
					write(fd_motor_z, &position_z, sizeof(position_z));
				}
				
				break;
				
			case 'z':
			 	position_z = position_z;
				printf("motor stopped, the position is %f m\n", position_z);
				fflush(stdout);
				write(fd_motor_z, &position_z, sizeof(position_z));
				break;
				
			default:
				break;			 
		 	}
		}
	}
	
 	close(fd_motor_z);
 	close(fd_comm_z);
	unlink(f_comm_z);
	unlink(f_motor_z);
	unlink(f_motor_z_to_insp);
	return 0;
	
}