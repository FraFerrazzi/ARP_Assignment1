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
//char* f_insp = "/tmp/f_insp";
char* f_motor_x_to_insp = "/tmp/f_motor_x_to_insp";
char* f_motor_z_to_insp = "/tmp/f_motor_z_to_insp";

//signal handling
	void sig_handler(int signo)
	{
		if(signo == SIGQUIT)
		{
			unlink(f_comm_x);
			unlink(f_comm_z);
			unlink(f_comm);
			unlink(f_motor_x);
			unlink(f_motor_z);
			//unlink(f_insp);
			unlink(f_motor_x_to_insp);
			unlink(f_motor_z_to_insp);
			unlink(f_master);

			kill(pidComm, SIGQUIT);
			kill(pidX, SIGQUIT);
			kill(pidZ, SIGQUIT);
			kill(pidInspect, SIGQUIT);
			kill(pidMaster, SIGQUIT);
			kill(pidWd, SIGQUIT);
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
	
 	
 	/*char buf1[20];
 	char buf2[20];
 	char buf3[20];
 	char buf4[20];
 	char buf5[20];*/

	//creating all the needed pipes for interprocess communication
	int ret_master = mkfifo(f_master, 0666);
	if(ret_master < 0)
	{
		perror("f_master");
		return -1;
	}

	int ret_mk_comm_x = mkfifo(f_comm_x, 0666);
    if (ret_mk_comm_x < 0)
    {
        perror("f_comm_x");
        unlink(f_comm_x);
        return -1;
    }

	int ret_mk_comm_z = mkfifo(f_comm_z, 0666);
    if (ret_mk_comm_z < 0)
    {
        perror("f_comm_z");
        unlink(f_comm_z);
        return -1;
    }

	int ret_mk_comm = mkfifo(f_comm, 0666);
   if (ret_mk_comm < 0)
    {
        perror("f_comm_x");
        unlink(f_comm_x);
        return -1;
    }

	int ret_mk_motor_x = mkfifo(f_motor_x, 0666);
	if (ret_mk_motor_x< 0)
    {
        perror("f_motor_x");
        unlink(f_motor_x);
        return -1;
    }

	int ret_mk_motor_z = mkfifo(f_motor_z, 0666);
	if (ret_mk_motor_z< 0)
    {
        perror("f_motor_z");
        unlink(f_motor_z);
        return -1;
    }

	/*int ret_mk_insp = mkfifo(f_insp, 0666);
	if (ret_mk_insp< 0)
    {
        perror("f_insp");
        unlink(f_insp);
        return -1;
    }*/

	int ret_mk_mot_x_to_insp = mkfifo(f_motor_x_to_insp, 0666);
	if (ret_mk_mot_x_to_insp < 0)
    {
        perror("f_motor_x_to_insp");
        unlink(f_motor_x_to_insp);
        return -1;
    }
	
	int ret_mk_mot_z_to_insp = mkfifo(f_motor_z_to_insp, 0666);
	if (ret_mk_mot_z_to_insp < 0)
    {
        perror("f_motor_z_to_insp");
        unlink(f_motor_z_to_insp);
        return -1;
    }

	signal(SIGQUIT, sig_handler);

 	// defining the arg list needed for each program
 	char * arg_list_comm [] = { "/usr/bin/konsole",  "-e", "./command_console", "", (char*)NULL };
 	char * arg_list_motorX [] = { "/usr/bin/konsole",  "-e", "./motor_x", " ", (char*)NULL };
 	char * arg_list_motorZ [] = { "/usr/bin/konsole",  "-e", "./motor_z", " ", (char*)NULL };
 	char * arg_list_inspect [] = { "/usr/bin/konsole",  "-e", "./inspection_console", " ", (char*)NULL };
 	char * arg_list_wd [] = { "/usr/bin/konsole",  "-e", "./watchdog", (char*)NULL };

	// calling the spawn function for each program needed
 	pidComm = spawn("/usr/bin/konsole", arg_list_comm); 	
 	pidX = spawn("/usr/bin/konsole", arg_list_motorX); 	
 	pidZ = spawn("/usr/bin/konsole", arg_list_motorZ); 	
 	pidInspect = spawn("/usr/bin/konsole", arg_list_inspect); 		
 	pidWd = spawn("/usr/bin/konsole", arg_list_wd); 	
 	printf("Main program exiting...\n");
 	fflush(stdout);

	//get the pid and send it to the command console for signal handling
	int fd_master;
	pidMaster = getpid();
	printf("Master says: my pid is %d\n", pidMaster);
	fflush(stdout);
	fd_master = open(f_master, O_WRONLY);
	write(fd_master, &pidMaster, sizeof(pidMaster));
	close(fd_master);
	
	sleep(3600);
 	return 0; 
			
}
