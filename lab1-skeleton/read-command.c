// UCLA CS 111 Lab 1 command reading

/* DESIGN PROJECT

a >> b: Append a to b      
<& : Reverse of >&?
>& output : Redirect stderr and stdout to file Equivalent to output > output 2>&1?     & causes output that goes to stderr to go to stdout
j <> filename: Open filename for r/w, and assign file descriptor j to it 
>| : Force redirection

*/

#include "command.h"
#include "command-internals.h"
#include "alloc.h"

#include <error.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define INIT 0
#define AND 1
#define SEMICOLON 2
#define OR 3
#define PIPE 4
#define CMD 5
#define LEFT_SUBSHELL 6
#define RIGHT_SUBSHELL 7
#define LEFT_REDIR 8
#define RIGHT_REDIR 9
#define NEWLINE 10
#define APPEND 11       // >>

// Singly linked list of commands
struct command_stream {
    int size;         
    int iterator;
    command_t *commands;
}; 

/****************** Stack data structure ****************************/
// MAY NEED TO CHANGE OPERATOR STACK
typedef struct stack
{
    command_t command;
    struct stack* prev;   
}* myCommandStack;

void push(myCommandStack* stack, command_t command)
{
    myCommandStack temp = (myCommandStack) checked_malloc(sizeof(struct stack));
    temp->command = command;
    temp->prev = *stack;
    *stack = temp;
}

void pop(myCommandStack* stack)
{
    if (stack != NULL && (*stack) != NULL)
    	*stack = (*stack)->prev;
}

command_t peek(myCommandStack* stack)
{
    if (stack == NULL || *stack == NULL)
    	return NULL;
    return (*stack)->command;
}

typedef struct stack2
{
    int operator;
    struct stack2* prev;
}* myOperatorStack;

void push2(myOperatorStack* stack, int oper)
{
    myOperatorStack temp = (myOperatorStack) checked_malloc(sizeof(struct stack2));
    temp->operator = oper;
    temp->prev = *stack;
    *stack = temp;
}

void pop2(myOperatorStack* stack)
{
    if (stack != NULL && (*stack) != NULL)
    	*stack = (*stack)->prev;
}

int peek2(myOperatorStack* stack)
{
    if (stack == NULL || *stack == NULL)
    	return -1;
    return (*stack)->operator;
}
/******************** Tokenizer*************************************/

// Singly linked list structure
typedef struct token_Node {
    int type;
    char *string;
    struct token_Node *next;
} token_Node;

typedef struct token_stream {
    token_Node *head;
    token_Node *tail;
    struct token_stream *next;
    int size;
} token_stream;

void insert_token(token_stream* stream, token_Node token)
{
    // Create temp token
    token_Node* temp = (token_Node*) checked_malloc(sizeof(token_Node));
    temp->type = token.type;
    temp->string = token.string;
    temp->next = token.next;

    // Add to empty token stream
    if (stream->head == NULL)
    {
    stream->head = temp;
    stream->tail = temp;
    }
    else
    { 
    stream->tail->next = temp;
    stream->tail = temp;
    }

    stream->size++;
    return;
}

token_stream* tokenizer(char* input)
{
    char c;

    // Initialize token stream
    token_stream* stream = checked_malloc(sizeof(token_stream));
    stream->head = NULL;
    stream->tail = NULL;
    stream->next = NULL;
    stream->size = 0;

    token_stream* origin = checked_malloc(sizeof(token_stream));
    origin = stream;

    // Create new token
    token_Node temptoken;
    temptoken.type = INIT;
    temptoken.string = NULL;
    temptoken.next = NULL;

    size_t i;
    for (i = 0; i < strlen(input); i++)
    {
    c = input[i];
    switch(c)
    {
    case '#': break;    // no comments should be in the buffer
    case '&':
    {
    if (temptoken.type != INIT)
        insert_token(stream, temptoken);

    // if &&
    if (input[i+1] == '&')
    {
        char* str = checked_malloc(3*sizeof(char));
        str[0] = c;
        str[1] = c;
        str[2] = '\0';
        temptoken.string = str;
        temptoken.type = AND;
        i++;
        break;
    }
    }
    case '\n': 
    { 
        if (temptoken.type != INIT)      
        insert_token(stream, temptoken);

        stream->next = checked_malloc(sizeof(token_stream));
        stream = (token_stream*) (stream->next);
        char* str = checked_malloc(sizeof(char));
        str[0] = '\0';
        temptoken.string = str;
        temptoken.type = NEWLINE;
        break;
    }
    case ';':   // end of token
    {
        if (temptoken.type != INIT)      
        insert_token(stream, temptoken);

        char* str = checked_malloc(sizeof(char));
        str[0] = c;                       // CHANGE THIS BACK TO str[0] = c WHEN DONE
        str[1] = '\0';
        temptoken.string = str;
        temptoken.type = SEMICOLON;
        break;
    }
    case '|':
    {
    if (temptoken.type != INIT)
        insert_token(stream, temptoken);

    // if ||
    if (input[i+1] == '|')
    {
        char* str = checked_malloc(3*sizeof(char));
        str[0] = c;
        str[1] = c;
        str[2] = '\0';
        temptoken.string = str;
        temptoken.type = OR;
        i++;
        break;
    }

    // if pipe
    else if (input[i+1] != '|')
    {
        char* str = checked_malloc(2*sizeof(char));
        str[0] = c;
        str[1] = '\0';
        temptoken.string = str;
        temptoken.type = PIPE;
        break;
    }
    }

    // left redirect
    case '<':
    {
    if (temptoken.type != INIT)
        insert_token(stream, temptoken);

    char* str = checked_malloc(2*sizeof(char));
    str[0] = c;
    str[1] = '\0';
    temptoken.string = str;
    temptoken.type = LEFT_REDIR;
    break;
    }

    // right redirect
    case '>':
    {
    if (temptoken.type != INIT)
        insert_token(stream, temptoken);


    char* str = checked_malloc(2*sizeof(char));
    str[0] = c;
    str[1] = '\0';
    temptoken.string = str;
    temptoken.type = RIGHT_REDIR;
    break;
    }

    // subshell start
    case '(':
    {
    if (temptoken.type != INIT)
        insert_token(stream, temptoken);

    char* str = checked_malloc(2*sizeof(char));
    str[0] = c;
    str[1] = '\0';
    temptoken.string = str;
    temptoken.type = LEFT_SUBSHELL;
    break;
    }

    // subshell end
    case ')':
    {
    if (temptoken.type != INIT)
        insert_token(stream, temptoken);

    char* str = checked_malloc(2*sizeof(char));
    str[0] = c;
    str[1] = '\0';
    temptoken.string = str;
    temptoken.type = RIGHT_SUBSHELL;
    break;
    }
    case ' ':
    {
    // if the space is not inside a command, we don't care
    if (temptoken.type != CMD)
        break;
    else
    {
        if (temptoken.type != INIT)
        insert_token(stream, temptoken);
        char* str = checked_malloc(sizeof(char));
        str[0] = '\0';
        temptoken.string = str;
        temptoken.type = CMD;     
        break; 
    }
    // otherwise, let default handle it
    }
    default:
    {
        if (temptoken.type != CMD)  // get all, 
        {
        if (temptoken.type != INIT)    // not an new token
        insert_token(stream, temptoken);

        // save new character
        char* str = checked_malloc(sizeof(char));
        str[0] = '\0';
        temptoken.string = str;
        temptoken.type = CMD;
        }

        // add character to end of string
        size_t length = strlen(temptoken.string);
        temptoken.string = checked_realloc(temptoken.string, (length+1)*sizeof(token_Node));
        temptoken.string[length] = c;
        temptoken.string[length+1] = '\0';
        break;

    }

    }

    }

    // left-over tokens?
    if (temptoken.type != INIT)
    	insert_token(stream, temptoken);

    // insert NULL stream to mark end of tokens
    stream->next = checked_malloc(sizeof(token_stream));
    //stream = (token_stream*) (stream->next);


    //fprintf(stderr, "%s\n", "End of Tokenizer");

    return origin;
}


/*********************** Parser ************************************/

int precedence(int oper)
{
    int operators[5] = {SEMICOLON, NEWLINE, AND, OR, PIPE};
    int rank[5] = {0, 0, 1, 1, 2};
    int pos = -1;
    int i;

    for (i = 0; i < 5; i++)
    {
    if (oper == operators[i])
    {
        pos = i;
        break;
    }
    }
    if (pos == -1)
    return -1;
    return rank[pos];
}


// NOT SURE IF hANDLING NEWLINE PROPERLY (THINK I AM)
command_t combineCommand(command_t first, command_t second, int operator)
{
    switch(operator)
    {
	    case SEMICOLON:
	    case NEWLINE:
	    {
		    //printf("SEQUENCE\n");
		    command_t newCommand = (command_t) checked_malloc(sizeof(struct command));
		    newCommand->type = SEQUENCE_COMMAND;
		    newCommand->status = -1;
		    newCommand->input = NULL;
		    newCommand->output = NULL;
		    newCommand->u.command[0] = first;
		    newCommand->u.command[1] = second;
		    return newCommand;
		    break;
	    }
	    case AND:
	    {
		    //printf("AND\n");
		    command_t newCommand = (command_t) checked_malloc(sizeof(struct command));
		    newCommand->type = AND_COMMAND;
		    newCommand->status = -1;
		    newCommand->input = NULL;
		    newCommand->output = NULL;
		    newCommand->u.command[0] = first;
		    newCommand->u.command[1] = second;
		    return newCommand;
		    break;
	    }
	    case OR:
	    {
		    //printf("OR\n");
		    command_t newCommand = (command_t) checked_malloc(sizeof(struct command));
		    newCommand->type = OR_COMMAND;
		    newCommand->status = -1;
		    newCommand->input = NULL;
		    newCommand->output = NULL;
		    newCommand->u.command[0] = first;
		    newCommand->u.command[1] = second;
		    return newCommand;
		    break;
	    }
	    case PIPE:
	    {
		    //printf("PIPE\n");
		    command_t newCommand = (command_t) checked_malloc(sizeof(struct command));
		    newCommand->type = PIPE_COMMAND;
		    newCommand->status = -1;
		    newCommand->input = NULL;
		    newCommand->output = NULL;
		    newCommand->u.command[0] = first;
		    newCommand->u.command[1] = second;
		    return newCommand;
		    break;
	    }
	    case LEFT_REDIR:
	    {
	    	//printf("LEFT REDIR\n");
	    	first->input = second->u.word[0];		// Should be a string
	    	return first;

	    }
	    case RIGHT_REDIR:
	    {
	    	//printf("RIGHT REDIR\n");
	    	first->output = second->u.word[0];
	    	return first;
	    }
	    default: 
		    return NULL;
		    break;
    }
}

// MIGHT NEED TO ADD NULL BYTE AT END (DONT THINK SO)

command_t parser(token_stream* stream)
{
    //fprintf(stderr, "%s\n", "Entering Parser");

    //fprintf(stderr, "stream size: %d\n", stream->size); 

    if (stream->size == 0)
    {
        //fprintf(stderr, "%s\n", "NULL Stream");

        return NULL;
    }

    token_Node* iter = stream->head;

    myCommandStack command_stack = NULL;
    myOperatorStack operator_stack = NULL;
    size_t wordpos = 0;
    size_t word_length = 0;

    bool CMD_FLAG = false;

    while (iter != NULL)
    {
    	//printf("Looking at token: %s\n", iter->string);
	    //printf("iter->string: %s\n", iter->string);
	    if (iter->type == INIT || iter->type == NEWLINE)
	    {
	        iter = iter->next;
	        continue;
	    }
	    else if (iter->type == CMD)
	    {
	        //printf("Making a simple command\n");
	        if (CMD_FLAG == false)
	        {
	            command_t simple = (command_t) checked_malloc (sizeof(struct command));
	            simple->type = SIMPLE_COMMAND;
	            simple->status = -1;
	            simple->input = NULL;
	            simple->output = NULL;
	            simple->u.word = (char**) checked_malloc(2*sizeof(char*));
	            simple->u.word[wordpos] = iter->string;
	            simple->u.word[++wordpos] = NULL;
	            word_length += 2;
                simple->u.word_size = word_length; 
	            CMD_FLAG = true;
	            push(&command_stack, simple);
	            //printf("Simple->u.word[%d]: %s\n", (wordpos-1), simple->u.word[wordpos-1]);
	            iter = iter->next;
	            //printf("What's on the commandstack?\n");
	            //command_t test = peek(&command_stack);
	            //printf("Test: Type: %d Word[0]%s Word[1]%s\n", test->type, test->u.word[0], test->u.word[1]);

	            continue;
	        }
	        else
	        {
	            command_t simple = peek(&command_stack);
	            pop(&command_stack);
	            /*
	            if (peek(&command_stack) == NULL)
	                printf("This is NULL\n");
	            else
	                printf("WTF\n");
	            */

	            simple->u.word = (char**) checked_realloc(simple->u.word, (word_length+1)*sizeof(char*));
	            simple->u.word[wordpos] = iter->string;

	            simple->u.word[++wordpos] = NULL;
	            word_length++;
                simple->u.word_size = word_length; 
	            push(&command_stack, simple);

	            //printf("Simple->u.word[0]: %s\n", simple->u.word[0]);
	            //printf("Simple->u.word[%d]: %s\n", (wordpos-1), simple->u.word[wordpos-1]);
	            iter = iter->next;
	            //printf("What's on the commandstack?\n");
	            //command_t test = peek(&command_stack);
	            //printf("Test: Type: %d Word[0]%s Word[1]%s\n", test->type, test->u.word[0], test->u.word[1]);

	            continue;
	        }
	    }
	    // Subshell
	    else if (iter->type == LEFT_SUBSHELL)
	    {
	        //printf("Detected (\n");
	        wordpos = 0;
	        word_length = 0;
	        push2(&operator_stack, iter->type);
	        //int oper = peek2(&operator_stack);
	        //printf("On Operator stack: %d\n", oper);
	        iter = iter->next;
	        CMD_FLAG = false;
	        continue;
	    }
	    else if (iter->type == RIGHT_SUBSHELL)
	    {
	        //printf("Am I here?\n");
	        wordpos = 0;
	        word_length = 0;
	        int top_operator = peek2(&operator_stack);
	        //printf("On Operator stack: %d\n", top_operator);
	        pop2(&operator_stack);

	        /*
	        int sanity_check = peek2(&operator_stack);
	        if (sanity_check == -1)
	            printf("This is indeed empty\n");
	        else
	            printf("Why isnt this empty\n");
	        */
	        while (top_operator != LEFT_SUBSHELL)
	        {
	           // printf("Looking for (\n");
	            command_t second_command = peek(&command_stack);
	            pop(&command_stack);
	            command_t first_command = peek(&command_stack);
	            pop(&command_stack);
	            command_t new_command = combineCommand(first_command, second_command, top_operator);
	            push(&command_stack, new_command);
	            top_operator = peek2(&operator_stack);
	            pop2(&operator_stack);
	        }

	        command_t subshell = (command_t) checked_malloc (sizeof(struct command));
	        subshell->type = SUBSHELL_COMMAND;
	        subshell->status = -1;
	        subshell->input = NULL;
	        subshell->output = NULL;
	        command_t top_command = peek(&command_stack);
	        //printf("Top_command: Type: %d Word[0]%s Word[1]%s\n", top_command->type, top_command->u.word[0], top_command->u.word[1]);
	        subshell->u.subshell_command = top_command;
	        //printf("subshell_command: Type: %d Word[0]%s Word[1]%s\n", subshell->u.subshell_command->type, subshell->u.subshell_command->u.word[0], subshell->u.subshell_command->u.word[1]);
	        pop(&command_stack);
	        /*
	        if (peek(&command_stack) == NULL)
	            printf("This is empty!\n");
	        else
	            printf("WTF2\n");
	            */
	        push(&command_stack, subshell);

	        iter = iter->next;
	        CMD_FLAG = false;
	        continue;
	    }
	    // Redirection
	    else if (iter->type == LEFT_REDIR)
	    {
	        wordpos = 0;
	        word_length = 0;
	        iter = iter->next;
	        char* next_token = iter->string;
	        //printf("Next_token: %s\n", iter->string);
	        command_t top_command = peek(&command_stack);
	        pop(&command_stack);
	        //printf("top_command->u.word[0]: %s\n", top_command->u.word[0]);
	        top_command->input = next_token;
	        //printf("top_command->input: %s\n", top_command->input);
	        push(&command_stack, top_command);
	        iter = iter->next;
	        CMD_FLAG = false;
	        continue;
	    }
	    else if (iter->type == RIGHT_REDIR)
	    {
	        wordpos = 0;
	        word_length = 0;
	        iter = iter->next;
	        char* next_token = iter->string;
	        command_t top_command = peek(&command_stack);
	        pop(&command_stack);
	        top_command->output = next_token;
	        push(&command_stack, top_command);
	        iter = iter->next;
	        CMD_FLAG = false;
	        continue;
	    }
	    // Operators
	    else if (iter->type == AND || iter->type == OR || iter->type == SEMICOLON || iter->type == PIPE)
	    {
	        wordpos = 0;
	        word_length = 0;
	        if (operator_stack == NULL)
	        {
		        //printf("Empty operator stack\n");
		        push2(&operator_stack, iter->type);
		        //printf("Pushing %d!\n", iter->type);
		        iter = iter->next;
		        CMD_FLAG = false;
		        continue;
	        }

	        else
	        {
		        int top_operator = peek2(&operator_stack);
		        if (precedence(iter->type) > precedence(top_operator))
		        {
			        push2(&operator_stack, iter->type);
			        //printf("Pushing %d!\n", iter->type);
			        iter = iter->next;
			        CMD_FLAG = false;
			        continue;
	        	}

		        else
		        {
			        while(top_operator != LEFT_SUBSHELL && (precedence(iter->type) <= precedence(top_operator)))
			        {
				        int operator = peek2(&operator_stack);
				        pop2(&operator_stack);
				        command_t second_command = peek(&command_stack);
				        pop(&command_stack);
				        command_t first_command = peek(&command_stack);
				        pop(&command_stack);
				        //printf("combine operator: %d\n", operator);
				        command_t new_command = combineCommand(first_command, second_command, operator);
				        push(&command_stack, new_command);
				        top_operator = peek2(&operator_stack);
				        if (top_operator == -1)
				            break;
			        }
			        push2(&operator_stack, iter->type);
			        //printf("Pushing2 %d!\n", iter->type);
			        iter = iter->next;
			        CMD_FLAG = false;
			        continue;
			    }
		    }
	    }

	    // Catch some other token just to be safe
	    else
	    {
	        //printf("Why am i here\n");
	        wordpos = 0;
	        word_length = 0;
	        iter = iter->next;
	        CMD_FLAG = false;
	        continue;
	    }
    }

    //printf("Outside\n");

    // Left-over operators
    while (operator_stack != NULL)
    {
	    //printf("Operator stack is not empty\n");
	    int operator = peek2(&operator_stack);
	    //printf("Operator: %d\n", operator);
	    pop2(&operator_stack);
	    command_t second_command = peek(&command_stack);
	    pop(&command_stack);
	    command_t first_command = peek(&command_stack);
	    pop(&command_stack);
	    command_t new_command = combineCommand(first_command, second_command, operator);
	    push(&command_stack, new_command);
    }
    
    /*else
    {
	    //printf("Operator stack is empty\n");
	    command_t second_command = peek(&command_stack);
	    if (second_command != NULL)
	    {
	        pop(&command_stack);
	        command_t first_command = peek(&command_stack);
	        if (first_command != NULL)
	        {
	        //  printf("Two commands, no operators\n");
	        
	        command_t simple = (command_t) checked_malloc (sizeof(struct command));
	        simple->type = SIMPLE_COMMAND;
	        simple->status = -1;
	        simple->input = NULL;
	        simple->output = NULL;
	        simple->u.word = (char**) checked_malloc(3*sizeof(char*));
	        simple->u.word[0] = iter->string;
	        simple->u.word[++wordpos] = NULL;
	        word_length += 2;
	        CMD_FLAG = true;
	        push(&command_stack, simple);
	        
	        }
	        else
	        push(&command_stack, second_command);
	    }
    }
*/

    command_t root = peek(&command_stack);
    //if (root->u.word != NULL)
    //{
    //  printf("value of SIMPLE_COMMAND: %d\n", SIMPLE_COMMAND);
    //  printf("root: %s type: %d\n", root->u.word[0], root->type);
    //}
    if (root == NULL)
    {
    //printf("Nothing on commnad_stack\n");
        return NULL;
    }
    else
    {
        //printf("root type: %d\n", root->type);
        pop(&command_stack);

        return root;
    }

}
/*******************************************************************/

// FIX UNNCESSARY NEWLINES TOKENS (GHETTO FIXED)
// SOME WEIRD SHIT HAPPENS IN COMMENTS E.G. #a;b --> INVALID SEMICOLON
// GOT FUCKED ON SUBSHELL


command_stream_t
make_command_stream (int (*get_next_byte) (void *),
         void *get_next_byte_argument)
{
    char current;
    char* buffer = checked_malloc(sizeof(char));
    size_t unpair = 0;
    char last = '\0';      // last character read
    char last_nospace = '\0';   // tracks last previous nonspace character
    char last_last_nospace = '\0';    // tracks the last previous nonspace char before last_nospace
    bool AND_FLAG = false;        // if && then true
    bool OR_FLAG = false;         // if || then true
    bool REDIR_FLAG = false; 
    bool COMMENT_FLAG = false;
    int BEGINNING_FLAG = 1; 
    size_t allocSize = 0;
    bool LINE_FLAG = true;      // Flags beginning of line, used to remove whitespace
    bool SUBSHELL_FLAG = false;
    int num_lines = 1;

    while ((current = get_next_byte(get_next_byte_argument)) != EOF)
    {
        if (!COMMENT_FLAG)
        {
            if (current == '\n')
                num_lines++;

            // operator cannot be first character in command
            if ((BEGINNING_FLAG == 1) && (((current == '|') || (current == '<') || (current == '&')) && (last == '\0')))
            {
                fprintf(stderr, "%d: Invalid syntax\n", num_lines);
                exit(1);
            }

            BEGINNING_FLAG = 0; 


            if (current == '(')
            {
                SUBSHELL_FLAG = true;
                unpair++;
            }
            if (current == ')')
            {
                SUBSHELL_FLAG = false;
                unpair--;
            }

            if(current == '\n' && (last_nospace == '\0') && COMMENT_FLAG == false)
            {
                continue;
            }
            if ((current == ' ' || current == '\r') && (last_nospace == '\0') && COMMENT_FLAG == false)
            {
                //printf("Skipping whitespace!\n");
                continue;
            }
            if (current == ';' && (last == ';' || last_nospace == '\n' || last == '\0'))
            {
                //error(1, 0, "%d: Invalid semicolon!\n, num_lines");
                fprintf(stderr, "%d: Invalid semicolon\n", num_lines);
                exit(1);
            }
            if (current == '|' && (last_nospace == '\n' || last == '\0'))
            {
                fprintf(stderr, "%d: Invalid semicolon\n", num_lines);
                exit(1);
            }
            if ((current == '<' && last_nospace == '<' && last_last_nospace == '<') || (current == '>' && last_nospace == '>' && last_last_nospace == '>'))
            {
                fprintf(stderr, "%d: Invalid redirection\n", num_lines);
                exit(1);
            }
            if (current == ';')
            {
                ;
                //current = '\n';
                //continue;
            }

            // checking for invalid character
            if (current == '`')
            {
                fprintf(stderr, "%d: Invalid syntax\n", num_lines);
                exit(1);
            }

            // Check for &&& and ||| and >>> and ;;
            if (AND_FLAG == true && current == '&')
            {
                fprintf(stderr, "%d: Invalid &\n", num_lines);
                exit(1);
            }
            if (OR_FLAG == true && current == '|')
            {
                fprintf(stderr, "%d: Invalid |\n", num_lines);
                exit(1); 
            }
            if (REDIR_FLAG == true && current == '>')
            {
                fprintf(stderr, "%d: Invalid >\n", num_lines);
                exit(1); 
            }

            // Set flag to be true if && or ||
            if (current == '&' && last == '&' && AND_FLAG == false)
                AND_FLAG = true;
            else
                AND_FLAG = false;

            if (current == '|' && last == '|' && OR_FLAG == false)
                OR_FLAG = true;
            else
                OR_FLAG = false;

            // Set flag to be true if >>
            if (current == '>' && last == '>' && REDIR_FLAG == false)
                REDIR_FLAG = true; 
            else
                REDIR_FLAG = false; 

            // Get rid of spaces between special tokens
            if ((current == '>' || current == '<' || current == '|' || current == '&') && last == ' ')
            {
                size_t length = strlen(buffer);
                buffer[length-1] = '\0';
            }

            if ((current == ' ') && (last == '>' || last == '<' || last == '|' || last == '&'))
                continue;

            // Comment after special token
            if ((current == '#') && (last_nospace == '>' || last_nospace == '<' || last_nospace == '|' || last_nospace == '&'))
            {
                fprintf(stderr, "%d: Invalid comment\n", num_lines);
                exit(1); 
            }

            if ((last == ' ' && current == ' ') || current == '\t' || (LINE_FLAG == true && current == ' '))
                continue;

            if (current == '#')
            {
                //printf("IN A COMMENT!\n");
                COMMENT_FLAG = true;
                continue;
            }

            /*
            if (current == '\n' && COMMENT_FLAG == true)
            {
                COMMENT_FLAG = false;
                //printf("Changing COMMENT_FLAG to false!\n");
                continue;
            }
            */

            if (current == '\n' && (last_nospace == '|' || last_nospace == '&'))
            {
                LINE_FLAG = true;
                continue;
            }

            if (current == '\n' && (last_nospace == '>' || last_nospace == '<'))
            {
                num_lines--; 
                fprintf(stderr, "%d: Invalid redirection\n", num_lines);
                exit(1); 
            }

            if (current == '\n' && last == '\n')
            {
        	    LINE_FLAG = true;
        	    continue;
            }

            if (current == '\n' && SUBSHELL_FLAG == true)
            {
            	continue;
            }

            if (current == ')' && SUBSHELL_FLAG == true)
            {
        	    allocSize++;
        	    buffer = checked_realloc(buffer, (2+allocSize)*sizeof(char));
        	    size_t length = strlen(buffer); 
        	    //printf("Buffer char%c", buffer[length]);
        	    buffer[length] = current; 
        	    buffer[length+1] = '\0'; 
        	    SUBSHELL_FLAG = false;
        	    continue;
            }

            if(!COMMENT_FLAG)
            {
                //printf("Not a comment!\n");
                if (current != '\n')
                LINE_FLAG = false;
                else
                LINE_FLAG = true;

                allocSize++;
                buffer = checked_realloc(buffer, (2+allocSize)*sizeof(char));   // 1 for null-byte and 1 for next char
                //append
                size_t length = strlen(buffer);
                buffer[length] = current;
                buffer[length+1] = '\0';
                //printf("%c", buffer[length]);
                //printf("\n");
                last = current;
                if (current != ' ')
                {
        	        if (last_nospace != '\0')
        	            last_last_nospace = last_nospace;

        	        last_nospace = current;
                }
            }
        }
        else
        {
            if (current == '\n' && COMMENT_FLAG == true)
            {
                COMMENT_FLAG = false;
                //printf("Changing COMMENT_FLAG to false!\n");
                continue;
            }
            else
                continue;
        }
    }

    // ghetto fix for detecting invalid ||
    if (last == '|')
    {
        fprintf(stderr, "%d: Invalid syntax\n", num_lines);
        exit(1); 
    }

    if (last == ')' && unpair != 0)
    {
	    fprintf(stderr, "%d: Unpaired parantheses\n", num_lines);
	    exit(1);
    }
    if (unpair != 0)
    {
	    fprintf(stderr, "%d: Unpaired parantheses\n", num_lines);
	    exit(1); 
    }

/*
    int j;
    printf("Buffer\n");
    for (j = 0; j < strlen(buffer); j++)
    {
    //if (buffer[j] == '\n')
        //printf("new\n");
    //else
        printf("%c", buffer[j]);
    }

    printf("\n-----------------\n");
*/    

    token_stream* stream = tokenizer(buffer);

    /*token_stream* stream2 = stream;


    int stream_counter = 0;


    printf("Token stream %d size: %d\n", stream_counter, stream2->size);
    int counter = 0;

    token_Node* temp = stream2->head;
    while (temp != NULL)
    {
        printf("Token %d: %s Type: %d\n", counter, temp->string, temp->type);
        counter++;

        if (temp->next == NULL)
        {
            if (stream2->next != NULL)
            {
            printf("Entering next sequence of commands\n");
            stream2 = (token_stream*)stream2->next;
            temp = stream2->head;
            stream_counter++;
            printf("Token stream %d size: %d\n", stream_counter, stream2->size);
            }
            else
            {
            printf("No More!\n");
            break;
            }
        }
        else
            temp = temp->next;
    }*/
   
    

    command_stream_t cmd_stream = checked_malloc(sizeof(struct command_stream));
    cmd_stream->size = 0;
    cmd_stream->iterator = 0;
    cmd_stream->commands = checked_malloc(sizeof(struct command));
    int pos = 0;

    while (stream != NULL)
    {
        /*
        int k;
        for (k = 0; k < cmd_stream->size; k++)
        {
            printf("Prev Command %d: ", k);
            if (cmd_stream->commands[k]->type == AND_COMMAND)
            {
                printf("AND_COMMAND\n");
            }
            else if (cmd_stream->commands[k]->type == OR_COMMAND)
                printf("OR_COMMAND\n");
            else if (cmd_stream->commands[k]->type == PIPE_COMMAND)
                printf("PIPE_COMMAND\n");
            else if (cmd_stream->commands[k]->type == SEQUENCE_COMMAND)
                printf("SEQUENCE_COMMAND\n");
            else if (cmd_stream->commands[k]->type == SIMPLE_COMMAND)
            {
                printf("SIMPLE_COMMAND:\n");
                printf("words[0]: %s\n", cmd_stream->commands[k]->u.word[0]);
            //printf("words[1]: %s\n", cmd_stream->commands[k]->u.word[1]);
            //printf("words[2]: %s\n", cmd_stream->commands[k]->u.word[2]);
            }
            else if (cmd_stream->commands[k]->type == SUBSHELL_COMMAND)
                printf("SUBSHELL_COMMAND\n");
            else
            {
                printf("Random command? %d\n", cmd_stream->commands[k]->type);
                //printf("words[0]: %s\n", cmd_stream->commands[k]->u.word[0]);
                //printf("words[1]: %s\n", cmd_stream->commands[k]->u.word[1]);
            }
        }
        */

        command_t newcommand = parser(stream);

        // that means it was the last extra token we have at the end
        if (newcommand == NULL)
        {
            //fprintf(stderr, "%s\n", "End of Parser");
            break;
        }
        else
        {
          /*  
            printf("newcommand->type: %d\n", newcommand->type);
            printf("Commands up to current pos: %d\n", pos);
            for (k = 0; k < pos; k++)
            {
                printf("Command %d: ", k);
                if (cmd_stream->commands[k]->type == AND_COMMAND)
                {
                    printf("AND_COMMAND\n");
                }
                else if (cmd_stream->commands[k]->type == OR_COMMAND)
                    printf("OR_COMMAND\n");
                else if (cmd_stream->commands[k]->type == PIPE_COMMAND)
                    printf("PIPE_COMMAND\n");
                else if (cmd_stream->commands[k]->type == SEQUENCE_COMMAND)
                    printf("SEQUENCE_COMMAND\n");
                else if (cmd_stream->commands[k]->type == SIMPLE_COMMAND)
                {
                    printf("SIMPLE_COMMAND:\n");
                    printf("words[0]: %s\n", cmd_stream->commands[k]->u.word[0]);
                //printf("words[1]: %s\n", cmd_stream->commands[k]->u.word[1]);
                //printf("words[2]: %s\n", cmd_stream->commands[k]->u.word[2]);
                }
                else if (cmd_stream->commands[k]->type == SUBSHELL_COMMAND)
                    printf("SUBSHELL_COMMAND\n");
                else
                {
                    printf("Random command? %d\n", cmd_stream->commands[k]->type);
                    //printf("words[0]: %s\n", cmd_stream->commands[k]->u.word[0]);
                    //printf("words[1]: %s\n", cmd_stream->commands[k]->u.word[1]);
                }
            }
            */

            cmd_stream->commands[pos] = newcommand;
            //printf("Why does command[0] get changed?\n");
            //printf("Type: %d word[0]: %s\n", cmd_stream->commands[0]->type, cmd_stream->commands[0]->u.word[0]);
            cmd_stream->size++;
            cmd_stream->commands = checked_realloc(cmd_stream->commands, (cmd_stream->size)*sizeof(struct command));        // WTF WHY RE_ALLOC???
            pos++;
        }
        /*
         printf("Pos: %d ", pos-1);
         if (cmd_stream->commands[pos-1] != NULL)
             printf("Command Type: %d\n", cmd_stream->commands[pos-1]->type);   // Should match root->type

        for (k = 0; k < cmd_stream->size; k++)
        {
            printf("Command %d: ", k);
            if (cmd_stream->commands[k]->type == AND_COMMAND)
            {
                printf("AND_COMMAND\n");
            }
            else if (cmd_stream->commands[k]->type == OR_COMMAND)
                printf("OR_COMMAND\n");
            else if (cmd_stream->commands[k]->type == PIPE_COMMAND)
                printf("PIPE_COMMAND\n");
            else if (cmd_stream->commands[k]->type == SEQUENCE_COMMAND)
                printf("SEQUENCE_COMMAND\n");
            else if (cmd_stream->commands[k]->type == SIMPLE_COMMAND)
            {
                printf("SIMPLE_COMMAND:\n");
                printf("words[0]: %s\n", cmd_stream->commands[k]->u.word[0]);
            //printf("words[1]: %s\n", cmd_stream->commands[k]->u.word[1]);
            //printf("words[2]: %s\n", cmd_stream->commands[k]->u.word[2]);
            }
            else if (cmd_stream->commands[k]->type == SUBSHELL_COMMAND)
                printf("SUBSHELL_COMMAND\n");
            else
            {
                printf("Random command? %d\n", cmd_stream->commands[k]->type);
                //printf("words[0]: %s\n", cmd_stream->commands[k]->u.word[0]);
                //printf("words[1]: %s\n", cmd_stream->commands[k]->u.word[1]);
            }
        }

        printf("----------------\n");
        */

        //  printf("Next stream\n");
        token_stream* toDelete = stream;
        stream = (token_stream*)stream->next;
        free(toDelete);
    }

    //printf("words[0]: %s\n, words[1]: %s\n words[2]: %s\n", cmd_stream->commands[0]->u.word[0], cmd_stream->commands[0]->u.word[1], cmd_stream->commands[0]->u.word[2]);

    /*printf("cmd_stream size: %d\n", cmd_stream->size);

    int k;
    for (k = 0; k < cmd_stream->size; k++)
    {
        printf("Command %d: ", k);
        if (cmd_stream->commands[k]->type == AND_COMMAND)
        {
            printf("AND_COMMAND\n");
        }
        if (cmd_stream->commands[k]->type == OR_COMMAND)
            printf("OR_COMMAND\n");
        if (cmd_stream->commands[k]->type == PIPE_COMMAND)
            printf("PIPE_COMMAND\n");
        if (cmd_stream->commands[k]->type == SEQUENCE_COMMAND)
            printf("SEQUENCE_COMMAND\n");
        if (cmd_stream->commands[k]->type == SIMPLE_COMMAND)
        {
            printf("SIMPLE_COMMAND:\n");
            printf("words[0]: %s\n", cmd_stream->commands[k]->u.word[0]);
            //printf("words[1]: %s\n", cmd_stream->commands[k]->u.word[1]);
            //printf("words[2]: %s\n", cmd_stream->commands[k]->u.word[2]);
        }
        if (cmd_stream->commands[k]->type == SUBSHELL_COMMAND)
        {
            printf("SUBSHELL_COMMAND\n");
            command_t inner = cmd_stream->commands[k]->u.subshell_command;
            if (inner->type == SIMPLE_COMMAND)
            {
                printf("inner SIMPLE_COMMAND\n");
                if (inner->input != NULL)
                {
                    printf("inner input not null\n");
                }
                if (inner->output != NULL)
                {
                    printf("inner output not null\n");
                }
            }
            if (cmd_stream->commands[k]->input != NULL)
            {
                printf("subshell input not null\n");
                printf("input: %s\n", cmd_stream->commands[k]->input);
            }
            if (cmd_stream->commands[k]->output != NULL)
            {
                printf("subshell output not null\n");
                printf("output: %s\n", cmd_stream->commands[k]->output);
            }
        }
    }*/


    free(buffer);
    return cmd_stream;

}

command_t
read_command_stream (command_stream_t s)
{
    if (s->iterator >= s->size)
        return NULL;
    return s->commands[(s->iterator++)];
}

