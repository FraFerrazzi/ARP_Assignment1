#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h> 
#include <fcntl.h> 
#include <signal.h>
#include <sys/stat.h> 
#include <sys/types.h> 

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
	// initialize PID of processes
	int pidComm;
 	int pidX;
 	int pidZ;
 	int pidInspect;
 	int pidWd;

    // creating the needed pipes for each process
    char* f_comm_x = "/tmp/f_comm_x";
    int ret_mk_comm_x = mkfifo(f_comm_x, 0666);
    if (ret_mk_comm_x < 0){
        perror(f_comm_x);
        unlink(f_comm_x);
        return -1;
    }

    char* f_comm_z = "/tmp/f_comm_z";
    int ret_mk_comm_z = mkfifo(f_comm_z, 0666);
    if (ret_mk_comm_z < 0){
        perror(f_comm_z);
        unlink(f_comm_z);
        return -1;
    }

    char* f_comm = "/tmp/f_comm";
    int ret_mk_comm = mkfifo(f_comm, 0666);
    if (ret_mk_comm < 0){
        perror(f_comm_x);
        unlink(f_comm_x);
        return -1;
    }
	
    char* f_insp = "/tmp/f_insp";
    int ret_mk_insp = mkfifo(f_insp, 0666);
    if (ret_mk_insp < 0){
        perror(f_insp);
        unlink(f_insp);
        return -1;
    }

    char* f_motor_x = "/tmp/f_motor_x";
    int ret_mk_mot_x = mkfifo(f_motor_x, 0666);
    if (ret_mk_mot_x < 0){
        perror(f_motor_x);
        unlink(f_motor_x);
        return -1;
    }

    char* f_motor_z = "/tmp/f_motor_z";
    int ret_mk_mot_z = mkfifo(f_motor_z, 0666);
    if (ret_mk_mot_z < 0){
        perror(f_motor_z);
        unlink(f_motor_z);
        return -1;
    }

	char* f_motor_x_to_insp = "/tmp/f_motor_x_to_insp";
	int ret_mk_mot_x_to_insp = mkfifo(f_motor_x_to_insp, 0666);
	if (ret_mk_mot_x_to_insp < 0){
        perror(f_motor_x_to_insp);
        unlink(f_motor_x_to_insp);
        return -1;
    }
	
	char* f_motor_z_to_insp = "/tmp/f_motor_z_to_insp";
	int ret_mk_mot_z_to_insp = mkfifo(f_motor_z_to_insp, 0666);
	if (ret_mk_mot_z_to_insp < 0){
        perror(f_motor_z_to_insp);
        unlink(f_motor_z_to_insp);
        return -1;
    }

 	// defining the arg list needed for each program
 	char * arg_list_comm[] = { "/usr/bin/konsole",  "-e", "./command_console", "", (char*)NULL };
 	char * arg_list_motorX[] = { "/usr/bin/konsole",  "-e", "./motor_x", " ", (char*)NULL };
 	char * arg_list_motorZ[] = { "/usr/bin/konsole",  "-e", "./motor_z", " ", (char*)NULL };
 	char * arg_list_inspect[] = { "/usr/bin/konsole",  "-e", "./inspection_console", " ", (char*)NULL };
 	char * arg_list_wd[] = { "/usr/bin/konsole",  "-e", "./watchdog", (char*)NULL };

	// calling the spawn function for each program needed
 	pidComm=spawn("/usr/bin/konsole", arg_list_comm);
 	pidX=spawn("/usr/bin/konsole", arg_list_motorX);
 	pidZ=spawn("/usr/bin/konsole", arg_list_motorZ);
 	pidInspect=spawn("/usr/bin/konsole", arg_list_inspect);	
 	pidWd=spawn("/usr/bin/konsole", arg_list_wd);
 	
    //sleep(3600);		decommentare in caso non funzioni
    // verrà sostituito da un while infinito che riceve segnali (dobbiamo parlarne)
 	printf("Main program exiting...\n");
 	fflush(stdout);
 	return 0; 		
}