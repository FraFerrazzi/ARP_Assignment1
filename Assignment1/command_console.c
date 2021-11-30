#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>

#define TIMER 2
void print_ui();

int main(int argc, char *argv[])
{
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
	read(fd_comm, &pidwd, sizeof(pidwd));
	close(fd_comm);	
	printf("WD pid: %d", pidwd);
	fflush(stdout);
	// get master's PID for signal handling
	int pidMaster;
	fd_master = open(f_master, O_RDONLY);
	read(fd_master, &pidMaster, sizeof(pidMaster));
	close(fd_master);
	printf("Master PID: %d\n", pidMaster);
	fflush(stdout);

	print_ui();

	// infinte for loop 
	for(;;)
	{		
		char choice;
		// getting the command from the user
		scanf(" %c", &choice);
		// decide the behavior based on user's choice
		kill(pidwd, SIGUSR1);
		switch (choice)
		{
			case 'A':
			case 'a':
				choice = 'a';
				fd_comm_x = open(f_comm_x, O_WRONLY);
				write(fd_comm_x, &choice, sizeof(choice));
				close(fd_comm_x);
				break;

			case 'D':
			case 'd':
				choice = 'd';
				fd_comm_x = open(f_comm_x, O_WRONLY);
				write(fd_comm_x, &choice, sizeof(choice));
				close(fd_comm_x);
				break;

			case 'W':
			case 'w':
				choice = 'w';
				fd_comm_z = open(f_comm_z, O_WRONLY);
				write(fd_comm_z, &choice, sizeof(choice));
				close(fd_comm_z);
				break;	

			case 'S':
			case 's':
				choice = 's';
				fd_comm_z = open(f_comm_z, O_WRONLY);
				write(fd_comm_z, &choice, sizeof(choice));
				close(fd_comm_z);
				break;			

			case 'Q':
			case 'q':
				choice = 'q';
				fd_comm_x = open(f_comm_x, O_WRONLY);
				write(fd_comm_x, &choice, sizeof(choice));
				close(fd_comm_x);
				break;

			case 'Z':
			case 'z':
				choice = 'z';
				fd_comm_z = open(f_comm_z, O_WRONLY);
				write(fd_comm_z, &choice, sizeof(choice));
				close(fd_comm_z);
				break;	

			case 'K':
			case 'k':
				kill(pidMaster, SIGQUIT);
				break;

			default:
				printf("It's not a valid input, retry");
				break;
		}	
	}
	
	unlink(f_comm);
	unlink(f_comm_x);	
	unlink(f_comm_z);
}

// function that displays the possible commands that user can use
void print_ui()
{
	printf("Enter a choice to move the structure: of the HOIST\n\n");
	printf("A = left       ");
	printf("        D = right\n");
	printf("W = up         ");
	printf("        S = down\n");
	printf("Q = stop x-axis");
	printf("        Z = stop z-axis\n\n");
	printf("RESET and STOP are given in inspection console");
	printf("\n\n");
	fflush(stdout);

	//sleep(TIMER);
}
