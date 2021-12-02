#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h> 
#include <fcntl.h> 
#include <signal.h>
#include <sys/stat.h> 
#include <sys/types.h> 


// initialize PID of processes
int pidComm;
int pidX;
int pidZ;
int pidInspect;
int pidWd;
int pidMaster;

//creating all the needed pipes for interprocess communication
char* f_master = "/tmp/f_master";
char* f_comm_x = "/tmp/f_comm_x";
char* f_comm_z = "/tmp/f_comm_z";
char* f_motor_x = "/tmp/f_motor_x";
char* f_comm = "/tmp/f_comm";
char* f_motor_z = "/tmp/f_motor_z";
char* f_motor_x_to_insp = "/tmp/f_motor_x_to_insp";
char* f_motor_z_to_insp = "/tmp/f_motor_z_to_insp";
char* f_insp = "/tmp/f_insp";

FILE *fp;

// signal handling from command console
// if user presses 'k' then it sends a signal to the master which unlink
// all the pipes that were created and closes every program
void sig_handler(int signo)
{
	if(signo == SIGQUIT)
	{
		// printing in the log file that processes are going to be killed
		fprintf(fp, "\nUser gave the QUIT Command\n");
		fflush(fp);

		unlink(f_comm_x);
		unlink(f_comm_z);
		unlink(f_comm);
		unlink(f_motor_x);
		unlink(f_motor_z);
		unlink(f_motor_x_to_insp);
		unlink(f_motor_z_to_insp);
		unlink(f_insp);
		unlink(f_master);

		kill(pidComm, SIGQUIT);
		kill(pidX, SIGQUIT);
		kill(pidZ, SIGQUIT);
		kill(pidInspect, SIGQUIT);
		kill(pidMaster, SIGQUIT);
		kill(pidWd, SIGQUIT);
		// printing in the log file that processes were correctly killed
		fprintf(fp, "All processes were correctly killed and all pipes were unlinked!\n");
		fflush(fp);
		exit(0);
	}
}


// function to spawn processes
int spawn(const char * program, char ** arg_list) 
{
	pid_t child_pid = fork(); // create parent and child processes
	// if we are in parent process, return the PID of the child
	if (child_pid != 0) 
		return child_pid;
	// if we are in child process, call execvp in order to sobstitute caller's image with the
	// executable file program
	else 
	{
    	execvp (program, arg_list);
    	perror("exec failed");
    	return 1;
    }
}

int main()
{
	// Open a file pointer named "logfileMaster.txt" for writing (w+)
    fp = fopen("./logfile/logfileMaster.txt", "w");
	// printing in the log file that master has been launched
	fprintf(fp, "MASTER PROGRAM IS LAUNCHED!!\n\n");
	fflush(fp);

	if(fp == NULL)
   	{
    	printf("Error opening the logfileMaster!");   
    	exit(1);             
   	}

	//creating all the needed pipes for interprocess communication
	int ret_master = mkfifo(f_master, 0666);
	if(ret_master < 0)
	{
		fprintf(fp, "Error creating master pipe");
		fflush(fp);
		perror("f_master");
		return -1;
	}
	
	int ret_mk_comm_x = mkfifo(f_comm_x, 0666);
    if (ret_mk_comm_x < 0)
    {
		fprintf(fp, "Error creating command x pipe");
		fflush(fp);
        perror("f_comm_x");
        unlink(f_comm_x);
        return -1;
    }

	int ret_mk_comm_z = mkfifo(f_comm_z, 0666);
    if (ret_mk_comm_z < 0)
    {
		fprintf(fp, "Error creating command z pipe");
		fflush(fp);
        perror("f_comm_z");
        unlink(f_comm_z);
        return -1;
    }

	int ret_mk_comm = mkfifo(f_comm, 0666);
   if (ret_mk_comm < 0)
    {
		fprintf(fp, "Error creating command pipe");
		fflush(fp);
        perror("f_comm_x");
        unlink(f_comm_x);
        return -1;
    }

	int ret_mk_motor_x = mkfifo(f_motor_x, 0666);
	if (ret_mk_motor_x< 0)
    {
		fprintf(fp, "Error creating motor x pipe");
		fflush(fp);
        perror("f_motor_x");
        unlink(f_motor_x);
        return -1;
    }

	int ret_mk_motor_z = mkfifo(f_motor_z, 0666);
	if (ret_mk_motor_z< 0)
    {
		fprintf(fp, "Error creating motor z pipe");
		fflush(fp);
        perror("f_motor_z");
        unlink(f_motor_z);
        return -1;
    }

	int ret_mk_mot_x_to_insp = mkfifo(f_motor_x_to_insp, 0666);
	if (ret_mk_mot_x_to_insp < 0)
    {
		fprintf(fp, "Error creating motor x to inspection pipe");
		fflush(fp);
        perror("f_motor_x_to_insp");
        unlink(f_motor_x_to_insp);
        return -1;
    }
	
	int ret_mk_mot_z_to_insp = mkfifo(f_motor_z_to_insp, 0666);
	if (ret_mk_mot_z_to_insp < 0)
    {
		fprintf(fp, "Error creating motor z to inspection pipe");
		fflush(fp);
        perror("f_motor_z_to_insp");
        unlink(f_motor_z_to_insp);
        return -1;
    }

	int ret_mk_insp = mkfifo(f_insp, 0666);
	if (ret_mk_insp< 0)
    {
		fprintf(fp, "Error creating inspection pipe");
		fflush(fp);
        perror("f_insp");
        unlink(f_insp);
        return -1;
    }

	// printing in the log file that all pipes needed were correctly created
	fprintf(fp, "ALL PIPES ARE CORRECTLY CREATED\n");
	fflush(fp);

	// handler for command console signal
	signal(SIGQUIT, sig_handler);

	// defining the arg list needed for watchdog
	// calling the spawn function for watchdog
	char * arg_list_wd [] = { "./watchdog", (char*)NULL };
	pidWd = spawn("./watchdog", arg_list_wd); 

 	// defining the arg list needed for each program
 	char * arg_list_comm [] = { "/usr/bin/konsole",  "-e", "./command_console", "", (char*)NULL };
 	char * arg_list_motorX [] = { "./motor_x", " ", (char*)NULL };
 	char * arg_list_motorZ [] = { "./motor_z", " ", (char*)NULL };
 	char * arg_list_inspect [] = { "/usr/bin/konsole",  "-e", "./inspection_console", " ", (char*)NULL };
 	
	// calling the spawn function for each program needed
 	pidComm = spawn("/usr/bin/konsole", arg_list_comm); 	
 	pidX = spawn("./motor_x", arg_list_motorX); 	
 	pidZ = spawn("./motor_z", arg_list_motorZ); 		
	pidInspect = spawn("/usr/bin/konsole", arg_list_inspect); 		
 	
	// printing in the log file that all processes are correctly created
	fprintf(fp, "ALL PROCESSES ARE CORRECTLY CREATED\n");
	// printing in the log file that all consoles are correctly opene
	fprintf(fp, "ALL CONSOLES ARE CORRECTLY OPEN\n");
	fflush(fp);
	


	//get the pid and send it to the command console for signal handling
	int fd_master;
	pidMaster = getpid();
	printf("Master says: my pid is %d\n", pidMaster);
	fflush(stdout);
	fd_master = open(f_master, O_WRONLY);
	if (fd_master < 0) 
	{
		fprintf(fp, "Error opening master pipe");
		fflush(fp);
        perror("fd_master");
        return -1;
    }
	write(fd_master, &pidMaster, sizeof(pidMaster));
	close(fd_master);
	// printing in the log file that master pid was sent to command console
	fprintf(fp, "Master PID is sent to Command Console\n");
	fflush(fp);
	
	// master keeps running for an hour if nothing kills it
	fprintf(fp, "Master waits for something to happen...\n");
	fflush(fp);
	sleep(3600);

	printf("Main program exiting...\n");
 	fflush(stdout);
 	return 0; 			
}
