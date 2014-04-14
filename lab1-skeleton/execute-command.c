// UCLA CS 111 Lab 1 command execution

#include "command.h"
#include "command-internals.h"

#include <error.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <fnctl.h>		// open file
#include <unistd.h>		// dup2

/* FIXME: You may need to add #include directives, macro definitions,
   static function definitions, etc.  */

int
command_status (command_t c)
{
  return c->status;
}

void
execute_command (command_t c, bool time_travel)
{
  int fd[2];	// For PIPE

  switch (c->type)
  {
  	case SIMPLE_COMMAND:
  	{
  		int status;
  		pid_t pid = fork();

  		if (pid > 0)
  			exit(1);
  		else
  		{
  			if (c->input != NULL)
  			{
  				int inputRedir = open(c->input, O_RDONLY);
  				if (inputRedir < 0)
  					exit(1);
  				if (dup2(inputRedir, 0) < 0)
  					exit(1);
  			}
  			if (c->output != NULL)
  			{
  				int outputRedir = open(c->output, O_WRONLY);
  				if (outputRedir < 0)
  					exit(1);
  				if (dup2(outputRedir, 1) < 0)
  					exit(1);
  			}

  			char **split = str_split(*c->u.word);

  			if (pid == 0)
  				execvp(split[0], split);
  			else
  				waitpid(pid, &status, 0);

  			c->status = status;
  			break;
  		}

  		break;
  	}
  }
  
}
