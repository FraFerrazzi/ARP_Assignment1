#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>

void print_ui();

int main(int argc, char *argv[])
{
	// initialize file descriptors for pipes
	int fd_comm_x;
	int fd_comm_z;
	int fd_comm;

	// initialize the temporary file
	char* f_comm_x = "/tmp/f_comm_x";
	char* f_comm_z = "/tmp/f_comm_z";
	char* f_comm = "/tmp/f_comm";

	// get the PID of the command console and send it to the watchdog
 	int pid_comm = getpid();
 	printf("comm console says: my pid_comm is  %d\n\n\n", pid_comm);
 	fflush(stdout);
 	fd_comm = open(f_comm, O_WRONLY);
    if (fd_comm < 0){
        perror(f_comm);
        return -1;
    }
 	write(fd_comm, &pid_comm, sizeof(pid_comm));
	close(fd_comm);

	// get watchdog's PID for signal handling
	int pidwd;
	fd_comm = open(f_comm, O_RDONLY);
    if (fd_comm < 0){
        perror(f_comm);
        return -1;
    }
	read(fd_comm, &pidwd, sizeof(pidwd)); // dobbiamo cambiare se non funziona 
	close(fd_comm); // ( passo direttamente la variabile)
	printf("WD pid_comm: %d", pidwd);
	fflush(stdout);
	
	print_ui();

	// infinte for loop 
	for(;;)
	{		
        // send signal to watchdog
		kill(pidwd, SIGUSR1);
        char choice;
		// getting the command from the user
		scanf(" %c", &choice);
        // decide the behavior based on user's choice
		switch (choice)
		{
			case 'A':
			case 'a':
				choice = 'a';
				fd_comm_x = open(f_comm_x, O_WRONLY);
                if (fd_comm_x < 0){
                    perror(f_comm_x);
                    return -1;
                }   
				write(fd_comm_x, &choice, sizeof(choice));
				close(fd_comm_x);
				break;

			case 'D':
			case 'd':
				choice = 'd';
				fd_comm_x = open(f_comm_x, O_WRONLY);
                if (fd_comm_x < 0){
                    perror(f_comm_x);
                    return -1;
                }   
				write(fd_comm_x, &choice, sizeof(choice));
				close(fd_comm_x);
				break;

			case 'W':
			case 'w':
				choice = 'w';
				fd_comm_z = open(f_comm_z, O_WRONLY);
                if (fd_comm_z < 0){
                    perror(f_comm_z);
                    return -1;
                }   
				write(fd_comm_z, &choice, sizeof(choice));
				close(fd_comm_z);
				break;	

			case 'S':
			case 's':
				choice = 's';
				fd_comm_z = open(f_comm_z, O_WRONLY);
                if (fd_comm_z < 0){
                    perror(f_comm_z);
                    return -1;
                }   
				write(fd_comm_z, &choice, sizeof(choice));
				close(fd_comm_z);
				break;			

			case 'Q':
			case 'q':
				choice = 'q';
				fd_comm_x = open(f_comm_x, O_WRONLY);
                if (fd_comm_x < 0){
                    perror(f_comm_x);
                    return -1;
                }   
				write(fd_comm_x, &choice, sizeof(choice));
				close(fd_comm_x);
				break;

			case 'Z':
			case 'z':
				choice = 'z';
				fd_comm_z = open(f_comm_z, O_WRONLY);
                if (fd_comm_z < 0){
                    perror(f_comm_z);
                    return -1;
                }   
				write(fd_comm_z, &choice, sizeof(choice));
				close(fd_comm_z);
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
}