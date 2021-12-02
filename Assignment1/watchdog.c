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
FILE *fp;

// signal handler in order to let watchdog know if a command arrived
// on the command console
void sig_handler(int signo)
{
	if (signo==SIGUSR1)
	{
		last_sig=time(NULL); // keeps track of an event
	}	
}

int main(int argc, char * argv[])
{
	// Open a file pointer named "logfileWatchdog.txt" for writing (w+)
    fp = fopen("./logfile/logfileWatchdog.txt", "w");
	if(fp == NULL)
   	{
    	printf("Error opening the logfileMaster!");   
    	exit(1);             
   	}
	// printing in the log file that watchdog is forked by master
	fprintf(fp, "WATCHDOG IS FORKED BY MASTER\n\n");
	fflush(fp);

	// initialize file descriptor for pipe
	int fd_comm;	
	int fd_motor_x;	
	int fd_motor_z;	
	int fd_insp;

	// initialize the temporary file
	char* f_comm = "/tmp/f_comm";
	char* f_motor_x = "/tmp/f_motor_x"; 
	char* f_motor_z = "/tmp/f_motor_z";
	char* f_insp = "/tmp/f_insp"; 

	// getting watchdog PID
	int wd_pid=getpid();
	printf("watchdog says: my pid is %d\n", wd_pid);
	fflush(stdout);
	// printing in the log file that watchdog got its PID
	fprintf(fp, "Watchdog pid is: %d\n", wd_pid);
	fflush(fp);
	
	// importing motor x pid and display it on watchdog konsole	
	int pidmotorx;
 	fd_motor_x = open(f_motor_x, O_RDONLY);
	if (fd_motor_x < 0) 
	{
		fprintf(fp, "Error opening motor x pipe");
		fflush(fp);
        perror("f_motor_x");
        return -1;
    }
 	read(fd_motor_x, &pidmotorx, sizeof(pidmotorx));
 	close(fd_motor_x);

 	// importing motor z pid and display it on watchdog konsole
    int pidmotorz;
	fd_motor_z = open(f_motor_z, O_RDONLY);
	if (fd_motor_z < 0) 
	{
		fprintf(fp, "Error opening motor z pipe");
		fflush(fp);
        perror("f_motor_z");
        return -1;
    }
 	read(fd_motor_z, &pidmotorz, sizeof(pidmotorz));
 	close(fd_motor_z);

	// sending watchdog PID to command process for signal handling
	fd_comm = open(f_comm, O_WRONLY);
	if (fd_comm < 0) 
	{
		fprintf(fp, "Error opening command pipe");
		fflush(fp);
        perror("f_comm");
        return -1;
    }
	write(fd_comm, &wd_pid, sizeof(wd_pid));
	close(fd_comm);

	// sending watchdog PID to command process for signal handling
	fd_insp = open(f_insp, O_WRONLY);
	if (fd_insp < 0) 
	{
		fprintf(fp, "Error opening inspection pipe");
		fflush(fp);
        perror("f_insp");
        return -1;
    }
	write(fd_insp, &wd_pid, sizeof(wd_pid));
	close(fd_insp);

	// printing all PIDS in log file
	fprintf(fp, "PID Motor X: %d\n", pidmotorx);
	fprintf(fp, "PID Motor Z: %d\n", pidmotorz);
 	fflush(fp);
	// printing in the log file all pipes used by watchdog are open
	fprintf(fp, "All pipes used by Watchdog are correctly open\n");
	fflush(fp);
	
	// SIGNAL HANDLING
	struct sigaction sa;
	// set sa to zero using the memset()
	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = &sig_handler;
	sa.sa_flags = SA_RESTART;
	sigaction(SIGUSR1, &sa, NULL); 
	last_sig = time(NULL);

	int counter = 0;

	// infinite loop
	// if user doesn't give any command, the counter increases
	// when counter reaches 60, watchdog resets the motor
	// if user give a command, reset the counter
	while(1)
	{
		sleep(1);
		fflush(stdout);
		if ( difftime(time(NULL),last_sig) > N)
		{
			// printing in the log file that nothing has happend for N seconds and send signals to motors
			fprintf(fp, "\nNothing has happened for %d seconds, Watchdog resets all the processes!\n", N);
			fflush(fp);

			kill(pidmotorx, SIGINT);
			fprintf(fp, "Send the RESET signal to motor x\n");
			fflush(fp);
			kill(pidmotorz, SIGINT);
			fprintf(fp, "Send the RESET signal to motor z\n");
			fflush(fp);				
		} 
		else if ( difftime(time(NULL),last_sig) == 0)
		{
			counter = 0;
			// printing in log file that user gave a command
			fprintf(fp, "\nUser gave a command, counter is setted to %d\n", counter);
			fflush(fp);
		}
		else
		{
			// printing in log file that counter has been increased
			fprintf(fp, "[%d] ", counter);
			fflush(fp);
			counter ++;
		}				
	}

	// unlink created pipes
	unlink(f_comm);
	unlink(f_motor_x);
	unlink(f_motor_z);
	return 0;
}
