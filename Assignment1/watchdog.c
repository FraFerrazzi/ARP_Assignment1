#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h> 
#include <fcntl.h> 
#include <signal.h>
#include <sys/stat.h> 
#include <sys/types.h> 
#include <time.h>

#define N 60

time_t last_sig;

void sig_hendler(int signo)
{
	if (signo==SIGUSR1)
	{
		last_sig=time(NULL); // keeps track of an event
	}	
}

int main(int argc, char * argv[])
{
	// getting watchdog PID
	int wd_pid=getpid();
	printf("wahtchdog says: my pid is %d\n", wd_pid);
	fflush(stdout);

	// importing command process PID and display it on watchdog konsole
	int fd_comm;
	int pidcomm;		
 	char* f_comm = "/tmp/f_comm";
  	fd_comm = open(f_comm, O_RDONLY);
 	read(fd_comm, &pidcomm, sizeof(wd_pid));
 	close(fd_comm);
 	printf("pid comm: %d\n", pidcomm);
 	fflush(stdout);
	
	// importing inspection process PID and display it on watchdog konsole
	int fd_insp;	
	int pidinsp;	
 	char* f_insp = "/tmp/f_insp";
 	fd_insp = open(f_insp, O_RDONLY);
 	read(fd_insp, &pidinsp, sizeof(pidinsp));
 	close(fd_insp);	
 	printf("pid inspection: %d\n", pidinsp);
	fflush(stdout);

	// importing motor x PID and display it on watchdog konsole
	int fd_motor_x;	
	int pidmotorx;	
 	char* f_motor_x = "/tmp/f_motor_x"; 	
 	fd_motor_x = open(f_motor_x, O_RDONLY);
 	read(fd_motor_x, &pidmotorx, sizeof(pidmotorx));
 	close(fd_motor_x);
 	printf("pid motor x: %d\n", pidmotorx);
 	fflush(stdout);
 	
 	// importing motor z PID and display it on watchdog konsole
	int fd_motor_z;
	int pidmotorz;		
 	char* f_motor_z = "/tmp/f_motor_z";
 	fd_motor_z = open(f_motor_z, O_RDONLY);
 	read(fd_motor_z, &pidmotorz, sizeof(pidmotorz));
 	close(fd_motor_z);
 	printf("pid motor z: %d\n", pidmotorz);
 	fflush(stdout);

	// sending watchdog PID to command process for signal handling
	fd_comm = open(f_comm, O_WRONLY);
	write(fd_comm, &wd_pid, sizeof(wd_pid));<
	close(fd_comm);

	signal(SIGUSR1, sig_hendler);
	last_sig = time(NULL);

	while(1)
	{
		sleep(1); // perchè lo sleep?
		fflush(stdout);
		if ( difftime(time(NULL),last_sig) > N)
			{
				printf("nothing has happened...\n");
				printf("RESET THE PROCESSES\n");
			
				kill(pidmotorx, SIGINT);
				kill(pidmotorz, SIGINT);				
			} 
			else
			{
				printf("time reset\n");
			}
			
				
	}
	unlink(f_comm);
	unlink(f_insp);
	unlink(f_motor_x);
	unlink(f_motor_z);
	return 0;
}