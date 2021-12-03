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

int main(int argc, char *argv[])
{
	// initialize variables for time informations for log file
	time_t rawtime;
  	struct tm * timeinfo;
	
	// Open a file pointer named "logfileCommandCons.txt" for writing (w+)
	FILE *fp; 
	fp = fopen("./logfile/logfileCommandCons.txt", "w");
	if(fp == NULL)
   	{
    	printf("Error opening the logfileMaster!");   
    	exit(1);             
   	}
	// get time of the day
	time ( &rawtime );
  	timeinfo = localtime ( &rawtime );
	// printing in the log file that command console is forked by master
	fprintf(fp, "%sCOMMAND CONSOLE IS FORKED BY MASTER\n\n\n", asctime (timeinfo));
	fflush(fp);
	
	// initialize file descriptors for pipes
	int fd_comm_x;
	int fd_comm_z;
	int fd_comm;
	int fd_master;

	// initialize the temporary file
	char* f_comm_x = "/tmp/f_comm_x";
	char* f_comm_z = "/tmp/f_comm_z";
	char* f_comm = "/tmp/f_comm";
	char* f_master = "/tmp/f_master";

	// get watchdog's PID for signal handling
	int pidwd;
	fd_comm = open(f_comm, O_RDONLY);
	if (fd_comm < 0) 
	{
		time ( &rawtime );
  		timeinfo = localtime ( &rawtime );
		fprintf(fp, "%sError opening command pipe", asctime (timeinfo));
		fflush(fp);
        perror("fd_comm");
        return -1;
    }
	read(fd_comm, &pidwd, sizeof(pidwd));
	close(fd_comm);	
	time ( &rawtime );
  	timeinfo = localtime ( &rawtime );
	fprintf(fp, "%sWatchdog PID: %d\n\n", asctime (timeinfo), pidwd);
	fflush(fp);

	// get master's PID for signal handling
	int pidMaster;
	fd_master = open(f_master, O_RDONLY);
	if (fd_master < 0) 
	{
		time ( &rawtime );
  		timeinfo = localtime ( &rawtime );
		fprintf(fp, "%sError opening master pipe", asctime (timeinfo));
		fflush(fp);
        perror("fd_master");
        return -1;
    }
	read(fd_master, &pidMaster, sizeof(pidMaster));
	close(fd_master);
	time ( &rawtime );
  	timeinfo = localtime ( &rawtime );
	fprintf(fp, "%sMaster PID: %d\n\n", asctime (timeinfo), pidMaster);
	// printing in the log file that all pipes used by command console are open
	time ( &rawtime );
  	timeinfo = localtime ( &rawtime );
	fprintf(fp, "%sAll pipes used by Command Console are correctly open\n\n", asctime (timeinfo));
	fflush(fp);
	

	// calling the function that prints the inputs that the user can use on the command console
	// these commands are used in order to control the behavior of the hoist
	print_ui();
	// printing in the log file that print_ui() function is called
	time ( &rawtime );
  	timeinfo = localtime ( &rawtime );
	fprintf(fp, "%sprint_ui() function has been called... User interface was printed on the shell\n\n\n", asctime (timeinfo));
	fflush(fp);

	// open pipes to send commands to motors
	fd_comm_x = open(f_comm_x, O_WRONLY);
	if (fd_comm_x < 0) 
	{
		time ( &rawtime );
  		timeinfo = localtime ( &rawtime );
		fprintf(fp, "%sError opening command x pipe", asctime (timeinfo));
		fflush(fp);
        perror("fd_comm_x");
        return -1;
    }

	fd_comm_z = open(f_comm_z, O_WRONLY);
	if (fd_comm_z < 0) 
	{
		time ( &rawtime );
  		timeinfo = localtime ( &rawtime );
		fprintf(fp, "%sError opening command z pipe", asctime (timeinfo));
		fflush(fp);
        perror("fd_comm_z");
        return -1;
    }

	// printing in the log file that pipes used by command console for sending commands to motors are open
	time ( &rawtime );
  	timeinfo = localtime ( &rawtime );
	fprintf(fp, "%sAll pipes used by Command Console for sending instructions to motors are correctly open\n\n\n", asctime (timeinfo));
	fflush(fp);
	

	// infinte for loop 
	for(;;)
	{		
		char choice;
		// getting the command from the user
		scanf(" %c", &choice);
		// printing in the log file the command that was chosen by the user
		time ( &rawtime );
  		timeinfo = localtime ( &rawtime );
		fprintf(fp, "%sCommand that was chosen by the user: %c\n\n", asctime (timeinfo), choice);
		fflush(fp);
		// every time there is a standard input which can be handled by the switch function
		// a signal is sent to watchdog with the sys call "kill(pidwd, SIGUSR1);"
		// this is used in order to reset watchdog's counter that reset motors positions
		// decide the behavior based on user's choice
		switch (choice)
		{
			case 'A': // user types a||A, hoist moves right
			case 'a':
				choice = 'a';
				write(fd_comm_x, &choice, sizeof(choice));
				kill(pidwd, SIGUSR1);
				break;

			case 'D': // user types d||D, hoist moves left
			case 'd':
				choice = 'd';
				write(fd_comm_x, &choice, sizeof(choice));
				kill(pidwd, SIGUSR1);
				break;

			case 'W': // user types w||W, hoist moves up
			case 'w':
				choice = 'w';
				write(fd_comm_z, &choice, sizeof(choice));
				kill(pidwd, SIGUSR1);
				break;	

			case 'S': // user types s||S, hoist moves down
			case 's':
				choice = 's';
				write(fd_comm_z, &choice, sizeof(choice));
				kill(pidwd, SIGUSR1);
				break;			

			case 'Q': // user types q||Q, hoist stops motor x
			case 'q':
				choice = 'q';
				write(fd_comm_x, &choice, sizeof(choice));
				kill(pidwd, SIGUSR1);
				break;

			case 'Z': // user types z||Z, hoist stops motor z
			case 'z':
				choice = 'z';
				write(fd_comm_z, &choice, sizeof(choice));
				kill(pidwd, SIGUSR1);
				break;	

			case 'K': // user types k||K, closes all programs
			case 'k':
				time ( &rawtime );
  				timeinfo = localtime ( &rawtime );
				fprintf(fp, "%sSignal in order to kill the processes has been sent to the master\n\n", asctime (timeinfo));
				fflush(fp);
				kill(pidMaster, SIGQUIT);
				break;

			default: // user types everything else, hoist does nothing
				printf("It's not a valid input, retry");
				break;
		}	
		system("clear");
		print_ui();
	}
	close(fd_comm_x);
	close(fd_comm_z);
	unlink(f_comm);
	unlink(f_comm_x);	
	unlink(f_comm_z);
}

// function that displays the possible commands that user can use
// if user decides to use anything else, the switcher will handle the exception
void print_ui()
{
	printf("Enter a choice to move the structure: of the HOIST\n\n");
	printf("A = left       ");
	printf("        D = right\n");
	printf("W = up         ");
	printf("        S = down\n");
	printf("Q = stop x-axis");
	printf("        Z = stop z-axis\n\n");
	printf("K = exit the program\n\n");
	printf("RESET and STOP has to be given from the inspection console\n");
	printf("R = reset motors");
	printf("        E = emergency stop\n\n\n");
	fflush(stdout);
}
