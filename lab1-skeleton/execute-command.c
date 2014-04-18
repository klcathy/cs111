// UCLA CS 111 Lab 1 command execution

#include "command.h"
#include "command-internals.h"

#include <error.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <fcntl.h>    // open file
#include <unistd.h>   // dup2
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <errno.h>

void executingSimple(command_t c);
void executingSubshell(command_t c);
void executingAnd(command_t c);
void executingOr(command_t c);
void executingSequence(command_t c);
void executingPipe(command_t c);

int
command_status (command_t c)
{
  return c->status;
}

void execute_switch (command_t c)
{
	switch (c->type)
	{
		case SIMPLE_COMMAND:
			executingSimple(c);
			break;
		case SUBSHELL_COMMAND:
			executingSubshell(c);
			break;
		case AND_COMMAND:
			executingAnd(c);
			break;
		case OR_COMMAND:
			executingOr(c);
			break;
		case SEQUENCE_COMMAND:
			executingSequence(c);
			break;
		case PIPE_COMMAND:
			executingPipe(c);
			break;
		default:
			error(1, 0, "Not a valid command");
	}
}

void executingSimple(command_t c)
{
	int status;
	pid_t pid = fork();

	if (pid < 0) // unsuccessful fork
	{
		error (1, errno, "fork was unsuccesful\n");
	}

	if (pid > 0) // parent process
	{
		//printf("Parent\n");
		waitpid(pid, &status, 0);
	}
	else // child process
	{
		if (c->input != NULL)
		{
			//printf("There is an input\n");
			int inputRedir = open(c->input, O_RDONLY | O_CREAT, 0644);
			if (inputRedir < 0)
				exit(1);
			if (dup2(inputRedir, 0) < 0)
				exit(1);
		}

		if (c->output != NULL)
		{
			//printf("There is an output\n");
			int outputRedir = open(c->output, O_WRONLY | O_TRUNC| O_CREAT, 0644);
		  	if (outputRedir < 0)
				exit(1);
		  	if (dup2(outputRedir, 1) < 0)
				exit(1);
		}

		execvp(c->u.word[0], c->u.word);
	}
	c->status = WEXITSTATUS(status);
	return;
}

void executingAnd(command_t c)
{
	command_t left = c->u.command[0];
	command_t right = c->u.command[1];
	execute_command(left, false);
	c->status = WEXITSTATUS(left->status);

	if (left->status == 0)
	{
		execute_command(right, false);
		c->status = WEXITSTATUS(right->status);
	}
	return;
}

void executingSubshell(command_t c)
{
	if (c->input != NULL)
	{
		//printf("There is an input\n");
		int inputRedir = open(c->input, O_RDONLY | O_CREAT, 0644);
		if (inputRedir < 0)
			exit(1);
		if (dup2(inputRedir, 0) < 0)
			exit(1);
	}

	if (c->output != NULL)
	{
		//printf("There is an output\n");
		int outputRedir = open(c->output, O_WRONLY | O_TRUNC| O_CREAT, 0644);
	  	if (outputRedir < 0)
			exit(1);
	  	if (dup2(outputRedir, 1) < 0)
			exit(1);
	}

	execute_command(c->u.subshell_command, false); 
	c->status = WEXITSTATUS((c->u.subshell_command)->status);
	return;
}

void executingOr(command_t c)
{
	command_t left = c->u.command[0];
	command_t right = c->u.command[1];
	execute_command(left, false);
	c->status = WEXITSTATUS(left->status);

	// keep on executing a command until you execute one that succeeds (ie. returns 0)
	// else until you reach the end
	if (left->status == 1)
	{
		execute_command(right, false);
		c->status = WEXITSTATUS(right->status);
	}
	return;
}

void executingSequence(command_t c)
{
	command_t left = c->u.command[0];
	command_t right = c->u.command[1];
	execute_command(left, false);
	execute_command(right, false);
	c->status = WEXITSTATUS(right->status);
	return;
}

void executingPipe(command_t c)
{
	pid_t returnedPid;
	pid_t firstPid;
	pid_t secondPid;
	int buffer[2];
	int eStatus;

	if ( pipe(buffer) < 0 )
	{
		error (1, errno, "pipe was not created");
	}

	firstPid = fork();
	if (firstPid < 0)
    {
		error(1, errno, "fork was unsuccessful");
    }
	else if (firstPid == 0) //child executes command on the right of the pipe
	{
		close(buffer[1]); //close unused write end

        //redirect standard input to the read end of the pipe
        //so that input of the command (on the right of the pipe)
        //comes from the pipe
		if ( dup2(buffer[0], 0) < 0 )
		{
			error(1, errno, "error with dup2");
		}
		execute_switch(c->u.command[1]);
		_exit(c->u.command[1]->status);
	}
	else 
	{
		// Parent process
		secondPid = fork(); //fork another child process
                            //have that child process executes command on the left of the pipe
		if (secondPid < 0)
		{
			error(1, 0, "fork was unsuccessful");
		}
        else if (secondPid == 0)
		{
			close(buffer[0]); //close unused read end
			if(dup2(buffer[1], 1) < 0) //redirect standard output to write end of the pipe
            {
				error (1, errno, "error with dup2");
            }
			execute_switch(c->u.command[0]);
			_exit(c->u.command[0]->status);
		}
		else
		{
			// Finishing processes
			returnedPid = waitpid(-1, &eStatus, 0); //this is equivalent to wait(&eStatus);
                        //we now have 2 children. This waitpid will suspend the execution of
                        //the calling process until one of its children terminates
                        //(the other may not terminate yet)

			//Close pipe
			close(buffer[0]);
			close(buffer[1]);

			if (secondPid == returnedPid )
			{
			    //wait for the remaining child process to terminate
				waitpid(firstPid, &eStatus, 0); 
				c->status = WEXITSTATUS(eStatus);
				return;
			}
			
			if (firstPid == returnedPid)
			{
			    //wait for the remaining child process to terminate
   				waitpid(secondPid, &eStatus, 0);
				c->status = WEXITSTATUS(eStatus);
				return;
			}
		}
	}	
}

void
execute_command (command_t c, bool time_travel)
{
 	if (time_travel == false)
 		execute_switch(c);
}
