// UCLA CS 111 Lab 1 command execu

#include "command.h"
#include "command-internals.h"
#include "alloc.h"

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

/****************************** LAB 1B IMPLEMENTATION ************************/

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
	//fprintf(stderr, "Executing Simple\n");
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
		c->status = WEXITSTATUS(status);
	}
	else // child process
	{
		if (c->input != NULL)
		{
			//printf("There is an input\n");
			int inputRedir = open(c->input, O_RDONLY, 0644);
			if (inputRedir < 0)
			{
				error(1, 0, "Unable to open inputfile");
			}
			if (dup2(inputRedir, 0) < 0)
			{
				error(1, 0, "Unable to use dup2");
			}
			if  (close(inputRedir) < 0)
			{
				error(1, 0, "Problem closing input file");
			}
		}

		if (c->output != NULL)
		{
			//printf("There is an output\n");
			int outputRedir = open(c->output, O_WRONLY | O_TRUNC| O_CREAT, 0644);
		  	if (outputRedir < 0)
				error(1, 0, "Unable to open outputfile");
		  	if (dup2(outputRedir, 1) < 0)
				error(1, 0, "Unable to use dup2");
			if (close(outputRedir) < 0)
			{
				error(1, 0, "Problem closing output file");
			}
		}

		// want to exit out of a : command
		if(c->u.word[0][0] == ':')
			_exit(0);

		if(execvp(c->u.word[0], c->u.word) < 0)
		{
			error(1, 0, "Error executing command");
		}
	}

	return;
}

void executingAnd(command_t c)
{
	command_t left = c->u.command[0];
	command_t right = c->u.command[1];
	execute_command(left, false);
	c->status = left->status;

	if (left->status == 0)
	{
		execute_command(right, false);
		c->status = right->status;
	}
	return;
}

void executingSubshell(command_t c)
{

	if (c->input != NULL)
	{
		//printf("There is an input\n");
		int inputRedir = open(c->input, O_RDONLY, 0644);
		if (inputRedir < 0)
			error(1, 0, "Unable to open inputfile");
		if (dup2(inputRedir, 0) < 0)
			error(1, 0, "Unable to use dup2");
		if  (close(inputRedir) < 0)
		{
			error(1, 0, "Problem closing input file");
		}
	}

	if (c->output != NULL)
	{
		//printf("There is an output\n");
		int outputRedir = open(c->output, O_WRONLY | O_TRUNC| O_CREAT, 0644);
	  	if (outputRedir < 0)
			error(1, 0, "Unable to open outputfile");
	  	if (dup2(outputRedir, 1) < 0)
			error(1, 0, "Unable to use dup2");
		if (close(outputRedir) < 0)
		{
			error(1, 0, "Problem closing output file");
		}
	}

	execute_command(c->u.subshell_command, false); 
	c->status = c->u.subshell_command->status;
	return;
}

void executingOr(command_t c)
{
	command_t left = c->u.command[0];
	command_t right = c->u.command[1];
	execute_command(left, false);
	c->status = left->status;

	// keep on executing a command until you execute one that succeeds (ie. returns 0)
	// else until you reach the end
	if (left->status != 0)
	{
		execute_command(right, false);
		c->status = right->status;
	}
	return;
}

void executingSequence(command_t c)
{
	command_t left = c->u.command[0];
	command_t right = c->u.command[1];
	execute_command(left, false);
	execute_command(right, false);
	c->status = right->status;
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




// /*************************** LAB 1C IMPLEMENTATION ***************************/

// /************************** DATA STRUCTURES **********************************/

/*
	GraphNodes for execution
*/

typedef struct GraphNode {
	command_t command;
	struct GraphNodeStream* before;
	pid_t pid;
} GraphNode;

/*
	Circular Queue
*/
typedef struct Queue
{
	int size;
	int capacity;
	int front;
	int back;
	struct GraphNode *elements;
} myQueue;

/*
	Nodes that hold inputs and outputs for readList and writeList.
*/
typedef struct myNode{
	struct myNode* next;
	char* data;
} myNode;

typedef struct myList {
	struct myNode* head;
	struct myNode* tail;
	int size;
} myList;

typedef struct DependencyGraph {
	struct Queue* no_dependency;
	struct Queue* dependency;
} DependencyGraph;

typedef struct ListNode {
	struct GraphNode g_node;
	struct myList* readList;
	struct myList* writeList;
	struct ListNode* next;
} ListNode;

typedef struct myList2 {
	struct ListNode* head;
	int size;
} myList2;

typedef struct GraphNodeStream {
	struct GraphNode* gn_arr; 
	int size; 
} GraphNodeStream; 

/*
	Creates a queue.
*/
myQueue* createQueue(int cap)
{
	myQueue	*queue = (myQueue*) malloc(sizeof(myQueue));

	queue->elements = (GraphNode*) malloc(sizeof(GraphNode) * cap);
	queue->size = 0;
	queue->capacity = cap;
	queue->front = 0;
	queue->back = -1;

	return queue;
}

/*
	Inserts element to the back of the queue.
*/

void pushQueue(myQueue* queue, GraphNode element)
{
	if (queue->size == queue->capacity)
	{
		printf("Queue is full!\n");
		return;
	}
	else
	{
		queue->size++;
		queue->back = queue->back + 1;

		if (queue->back == queue->capacity)
			queue->back = 0;

		queue->elements[queue->back] = element;
	}
	return;
}

/********************* LIST STRUCTURE TO FOR READ/WRITELIST ******************/

/*
	Inserts at tail.
*/
void insert(myList* list, myNode* node)
{
	myNode* temp =  (myNode*) checked_malloc(sizeof(myNode));
	temp->next = NULL;
	temp->data = node->data;
	//fprintf(stderr, "Insert: %s\n", temp->data);

	if (list->head == NULL)
	{
		list->head = temp;
		list->tail = temp;
	}
	else
	{
		list->tail->next = temp;
		list->tail = temp;
	}

	//fprintf(stderr, "Tail: %s\n", list->tail->data);

	list->size++;
	return;
}

/*
	Inserts at beginning.
*/
void insert2(myList2* list, ListNode* node)
{
	//fprintf(stderr, "Insert2\n");

	ListNode* temp = (ListNode*) checked_malloc (sizeof(ListNode));
	temp->readList = node->readList;
	temp->writeList = node->writeList;

	/*if (temp->readList == NULL)
	{
		fprintf(stderr, "readList is NULL\n");
	}
	else
	{
		myNode* iter = temp->readList->head;

		while (iter != NULL)
		{
			fprintf(stderr, "iter not NULL\n");
			fprintf(stderr, "Read: %s ", iter->data);
			iter = iter->next;
		}
	}*/

	/*if (temp->writeList == NULL)
	{
		fprintf(stderr, "writeList is NULL\n");
	}
	else
	{
		myNode* iter = temp->readList->head;

		while (iter != NULL)
		{
			fprintf(stderr, "Write: %s ", iter->data);
			iter = iter->next;
		}
	}*/

	if (list->head == NULL)
	{
		temp->next = NULL;
		list->head = temp;
	}
	else
	{
		temp->next = list->head;
		list->head = temp;
	}

	list->size++;

	return;

}

void insert3(GraphNodeStream* s, GraphNode node)
{
	s->gn_arr[s->size] = node; 
	s->size++; 
	s->gn_arr = checked_realloc(s->gn_arr, (s->size)*sizeof(struct GraphNode)); 
}

void processCommandHelper(command_t command, myList* readList, myList* writeList, GraphNode g_node)
{

	if (command->type == SIMPLE_COMMAND)
	{
		//fprintf(stderr, "SIMPLE\n");
		//fprintf(stderr, "word_size: %d\n", command->word_size);
		int i;

		if (command->input != NULL)
		{
			//fprintf(stderr, "Inserting into readList\n");
			myNode* tempNode = checked_malloc(sizeof(myNode));
			tempNode->next = NULL; 
			tempNode->data = command->input; 
			insert(readList, tempNode);
		}

		if (command->word_size > 1)
		{
			for (i = 1; i < command->word_size; i++)
			{
				//fprintf(stderr, "Inserting into readList\n");
				myNode* tempNode = (myNode*) checked_malloc(sizeof(myNode));
				tempNode->next = NULL;
				tempNode->data = command->u.word[i];
				insert(readList, tempNode);
			}
		}

		if (command->output != NULL)
		{
			//fprintf(stderr, "Inserting into writeList\n");
			myNode* tempNode = (myNode*) checked_malloc(sizeof(myNode));
			tempNode->next = NULL; 
			tempNode->data = command->output; 
			insert(writeList, tempNode);
			
		}

		return;
	}

	else if (command->type == SUBSHELL_COMMAND)
	{
		//fprintf(stderr, "SUBSHELL\n");
		//fprintf(stderr, "word_size: %d\n", command->word_size);
		if (command->input != NULL)
		{
			//fprintf(stderr, "Inserting into readList\n");
			myNode* tempNode = (myNode*) checked_malloc(sizeof(myNode)); 
			tempNode->next = NULL; 
			tempNode->data = command->input; 
			insert(readList, tempNode);
			
		}

		if (command->output != NULL)
		{
			//fprintf(stderr, "Inserting into writeList\n");
			myNode* tempNode = (myNode*) checked_malloc(sizeof(myNode));
			tempNode->next = NULL; 
			tempNode->data = command->output; 
			
		}
		processCommandHelper(command->u.subshell_command, readList, writeList, g_node); 
		return;
	}

	else 
	{
		//fprintf(stderr, "OTHER\n");
		//fprintf(stderr, "word_size: %d\n", command->word_size);

		processCommandHelper(command->u.command[0], readList, writeList, g_node);
		processCommandHelper(command->u.command[1], readList, writeList, g_node);
		return;
	}
} 

ListNode* processCommand(command_t command)
{
	myList* readList = (myList*) checked_malloc (sizeof(myList)); 
	readList->head = NULL;
	readList->tail = NULL; 
	readList->size = 0; 

	myList* writeList = (myList*) checked_malloc (sizeof(myList)); 
	writeList->head = NULL;
	writeList->tail = NULL; 
	writeList->size = 0; 

	GraphNodeStream* gnstream = (GraphNodeStream*) checked_malloc (sizeof(GraphNodeStream));
	gnstream->size = 0; 
	gnstream->gn_arr = checked_malloc(sizeof(GraphNode*)); 

	GraphNode g_node;
	g_node.pid = -1;
	g_node.before = gnstream;  
	g_node.command = command;

	ListNode* newNode = (ListNode*) checked_malloc(sizeof(ListNode));

	processCommandHelper(command, readList, writeList, g_node); 

	newNode->readList = readList; 
	newNode->writeList = writeList; 
	newNode->next = NULL; 
	newNode->g_node = g_node; 

	/*

	if (newNode->readList != NULL)
	{
		myNode* iter = newNode->readList->head;

		while (iter != NULL)
		{
			fprintf(stderr, "Read: %s ", iter->data);
			iter = iter->next;
		}
	}

	if (newNode->writeList != NULL)
	{
		myNode* iter = newNode->writeList->head;
		while (iter != NULL)
		{
			fprintf(stderr, "Write: %s ", iter->data);
			iter = iter->next;
		}
	}
	*/

	return newNode; 
}

/*
	Checks to see if list1 and list2 share any nodes.
	Returns true if they do, else false.
*/

bool intersect(myList* list1, myList* list2)
{
	//fprintf(stderr, "In intersect!\n");

	if (list1 == NULL || list2 == NULL)
		return false;

	myNode* iter1 = list1->head;
	myNode* iter2 = list2->head;

	while (iter1 != NULL)
	{
		//fprintf(stderr, "iter1->data: %s", iter1->data);
		//fprintf(stderr, "\n");
		while (iter2 != NULL)
		{
			//fprintf(stderr, "iter2->data: %s", iter2->data);
			if (iter1->data == iter2->data)
				return true;

			iter2 = iter2->next;
		}
		//fprintf(stderr, "\n");
		
		iter1 = iter1->next;
	}

	return false;
}

DependencyGraph* createGraph(command_stream_t s)
{
	//fprintf(stderr, "Running createGraph\n");
	int i; 

	DependencyGraph* graph = (DependencyGraph*) checked_malloc(sizeof(DependencyGraph));
	graph->no_dependency = createQueue(10);
	graph->dependency = createQueue(10);

	myList2* list = (myList2*) checked_malloc(sizeof(myList2));
	list->head = NULL;
	list->size = 0;

 	for (i = 0; i < s->size; i++)
	{
		ListNode* newListNode = processCommand(s->commands[i]);

	/*	
		if (newListNode->readList != NULL)
		{
			myNode* iter = newListNode->readList->head;
			while (iter != NULL)
			{
				fprintf(stderr, "Read: %s ", iter->data);
				iter = iter->next;
			}
		}

		if (newListNode->writeList != NULL)
		{
			myNode* iter = newListNode->writeList->head;
			while (iter != NULL)
			{
				fprintf(stderr, "Write: %s ", iter->data);
				iter = iter->next;
			}
		}
	*/
		insert2(list, newListNode);

		//fprintf(stderr, "After insert2\n");
		
		ListNode* iter = list->head->next;

		//fprintf(stderr, "After iter\n");

		while (iter != NULL)
		{
			//fprintf(stderr, "Checking intersections\n");
			if (intersect(newListNode->readList, iter->writeList) == true ||
				intersect(newListNode->writeList, iter->readList) == true ||
				intersect(newListNode->writeList, iter->writeList) == true )
				{
					insert3(newListNode->g_node.before, iter->g_node);
				}

			iter = iter->next;
		}

		//fprintf(stderr, "Gonna push some queues\n");
		if (newListNode->g_node.before == NULL)
			pushQueue(graph->no_dependency, newListNode->g_node);
		else
			pushQueue(graph->dependency, newListNode->g_node);

	}

	//fprintf(stderr, "Out the loop\n");

	return graph;
}

void execute_noDependency(myQueue* queue)
{
	//printf("In noDependency\n");
	int i;

	for (i = 0; i < queue->size; i++)
	{
		pid_t p = fork();

		if (p == 0)
		{
			execute_switch((queue->elements[i]).command);
			exit(0);
		}
		else if (p > 0)
			(queue->elements[i]).pid = p;
	}
}

void execute_Dependency(myQueue* queue)
{
	//printf("In Dependency\n");
	int i;
	int j;

	// Polling
	for (i = 0; i < queue->size; i++)
	{
		GraphNodeStream* before = queue->elements[i].before;

		loop_label:
			for (j = 0; j < before->size; j++)
			{
				if (before->gn_arr[j].pid == -1)
					goto loop_label;
			}

		// Child process has been forked
		int status;

		for (j = 0; j < before->size; j++)
			waitpid(before->gn_arr[j].pid, &status, 0);

		pid_t p = fork();

		if (p == 0)
		{
			execute_switch((queue->elements[i]).command);
			exit(0);
		}
		else if (p > 0)
			(queue->elements[i]).pid = p;
	}
}

void executeGraph(DependencyGraph* g)
{
	//printf("In executeGraph\n");
	execute_noDependency(g->no_dependency);
	execute_Dependency(g->dependency);
}


void run_timetravel(command_stream_t stream)
{
	DependencyGraph* g = createGraph(stream);
	executeGraph(g);
}
