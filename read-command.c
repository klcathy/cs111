// UCLA CS 111 Lab 1 command reading

#include "command.h"
#include "command-internals.h"
#include "alloc.h"

#include <error.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define INIT 0;
#define AND 1;
#define SEMICOLON 2;
#define OR 3;
#define PIPE 4;
#define CMD 5;
#define LEFT_SUBSHELL 6;
#define RIGHT_SUBSHELL 7;
#define LEFT_REDIR 8;
#define RIGHT_REDIR 9;
#define NEWLINE 10;
#define MISC 11;

/* FIXME: Define the type 'struct command_stream' here.  This should
   complete the incomplete type declaration in command.h.  */

// Singly linked list of commands
struct command_stream {
  struct command_Node *head;
  struct command_Node *tail;
  int size;         
  int iterator;
  struct command_t *command;
  struct command_stream *next;
}; 

struct command_Node {
  struct command *command;
  struct command_Node *next;
};

/* FIX SIZES LATER */
/*
void insert_command(struct command* curr_command)
{
  struct command_Node* new_command = (struct command_Node*) checked_malloc(sizeof(struct command_Node));
  new_command->command = curr_command;
  new_command->next = NULL;

  // Empty list
  if (head == NULL && tail == NULL)
  {
    head = new_command;
    tail = new_command;
    //size++;       
    return;
  }
  // Insert at end
  else
  {
    tail->next = new_command;
    tail = new_command;
   // size++;
    return;
  }
}

void delete_command(struct command* curr_command)
{
  struct command_Node* temp = head;
  int pos = 0;

  // Delete at beginning of the list
  if (temp->command == curr_command)
  {
    // if the list is only 1 command
    if (size == 1)
    {
      struct command_Node* toDelete = head;
      head = NULL;
      tail = NULL;
      free(toDelete);
      size--;
      return;
    }
    else
    {
      struct command_Node* toDelete = head;
      head = head->next;
      free(toDelete);
      size--;
      return;
    }
  }

  pos++;

  while (temp != NULL)
  {
    if ((temp->next)->command == curr_command)
    {
      struct command_Node* toDelete = temp->next;
      temp->next = (temp->next)->next;

      if (pos == size-1)
      {
        struct command_Node* iter = head;
        while (iter->next != tail)
        {
          iter = iter->next;
        }
        tail = iter;
      }

      free(toDelete);
      size--;
      return;
    }
    temp = temp->next;
  }

  return;
}
*/

/******************** Tokenizer/Parser *****************************/

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
  size_t size;
} token_stream;

void insert_token(token_stream* stream, token_Node token)
{
  // Create temp token
  token_Node* temp = (token_Node*) checked_malloc(sizeof(token_Node));
  temp->type = token->type;
  temp->string = token->string;
  temp->next = token->next;

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

token_stream* Tokenizer(char* input)
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
  token_Node newtoken;
  newtoken.string = NULL;
  newtoken.type = INIT;
  newtoken.next = NULL;

  size_t i;
  for (i = 0; i < strlen(input); i++)
  {
    c = input[i];
    switch(c)
    {
      case '#': break;    // no comments should be in the buffer
      case '&':
      {
        if (newtoken.type != INIT)
          insert_token(stream, newtoken);

        // if &&
        if (input[i+1] == '&')
        {
          char* newstring = checked_malloc(3*sizeof(char));
          newstring[0] = c;
          newstring[1] = c;
          newstring[2] = '\0';
          newtoken.string = newstring;
          newtoken.type = AND;
          i++;
          break;
        }
      }
      case '\n':
      case ';':   // end of token
        {
          if (newtoken.type != INIT)      
            insert_token(stream, newtoken);

          stream->next = checked_malloc(sizeof(token_stream));
          stream = (token_stream*) (stream->next);
          char* newstring = checked_malloc(sizeof(char));
          newstring[0] = c;
          newstring[1] = '\0';
          newtoken.string = newstring;
          newtoken.type = NEWLINE;
          break;
        }
      case '|':
      {
        if (newtoken.type != INIT)
          insert_token(stream, newtoken);

        // if ||
        if (input[i+1] == '|')
        {
          char* newstring = checked_malloc(3*sizeof(char));
          newstring[0] = c;
          newstring[1] = c;
          newstring[2] = '\0';
          newtoken.string = newstring;
          newtoken.type = OR;
          i++;
          break;
        }

        // if pipe
        else if (input[i+1] != '|')
        {
          char* newstring = checked_malloc(2*sizeof(char));
          newstring[0] = c;
          newstring[1] = '\0';
          newtoken.string = newstring;
          newtoken.type = PIPE;
          break;
        }
      }

      // left redirect
      case '<':
      {
        if (newtoken.type != INIT)
          insert_token(stream, newtoken);

        char* newstring = checked_malloc(2*sizeof(char));
        newstring[0] = c;
        newstring[1] = '\0';
        newtoken.string = newstring;
        newtoken.type = LEFT_REDIR;
        break;
      }

      // right redirect
      case '>':
      {
        if (newtoken.type != INIT)
          insert_token(stream, newtoken);


        char* newstring = checked_malloc(2*sizeof(char));
        newstring[0] = c;
        newstring[1] = '\0';
        newtoken.string = newstring;
        newtoken.type = RIGHT_REDIR;
        break;
      }

      // subshell start
      case '(':
      {
        if (newtoken.type != INIT)
          insert_token(stream, newtoken);

        char* newstring = checked_malloc(2*sizeof(char));
        newstring[0] = c;
        newstring[1] = '\0';
        newtoken.string = newstring;
        newtoken.type = LEFT_SUBSHELL;
        break;
      }

      // subshell end
      case ')':
      {
        if (newtoken.type != INIT)
          insert_token(stream, newtoken);

        char* newstring = checked_malloc(2*sizeof(char));
        newstring[0] = c;
        newstring[1] = '\0';
        newtoken.string = newstring;
        newtoken.type = RIGHT_SUBSHELL;
        break;
      }
      case ' ':
      {
        // if the space is not inside a command, we don't care
        if (newtoken.type != CMD)
          break;
      }
      default:
        {
          if (newtoken.type != CMD)  // get all, 
          {
            if (newtoken.type != INIT)    // not an new token
              insert_token(stream, newtoken);

            // save new character
            char* newstring = checked_malloc(sizeof(char));
            newstring[0] = '\0';
            newtoken.string = newstring;
            newtoken.type = CMD;
          }

          // add character to end of string
          size_t length = strlen(newtoken.string);
          newtoken.string = checked_realloc(newtoken.string, (length+1)*sizeof(token_Node));
          newtoken.string[length] = c;
          newtoken.string[length+1] = '\0';
          break;

        }

    }

  }

  // left-over tokens?
  if (newtoken.type != INIT)
    insert_token(stream, newtoken);

  return origin;
}

command_t Parser(token_stream* stream)
{
  if (stream == NULL)
    return NULL;

  token_Node* iter = stream->head;

  myStack commands = NULL;
  myStack operators = NULL;

  while (iter != NULL)
  {
    switch (iter->type)
    {
      case INIT: break;
      case CMD: 
    }
  }

}

/****************** Stack data structure ****************************/
typedef struct stack
{
    command_t command;
    struct stack* prev;   
}* myStack;

void push(myStack* stack, command_t command)
{
    myStack temp = (myStack) checked_malloc(sizeof(struct stack));
    temp->command = command;
    temp->prev = *stack;
    *stack = temp;
}

void pop(myStack* stack)
{
    if (stack != NULL && (*stack) != NULL)
        *stack = (*stack)->prev;
}

command_t peek(myStack* stack)
{
    if (stack == NULL || *stack == NULL)
        return NULL;
    return (*stack)->command;
}

/*******************************************************************/
bool isValid(char c)
{
    if (isalnum(c) || isspace(c))
        return true;
    switch (c) {
        case '!':
        case '%':
        case '+':
        case ',':
        case '-':
        case '.':
        case '/':
        case ':':
        case '@':
        case '^':
        case '_':
        case ';':
        case '|':
        case '&':
        case '(':
        case ')':
        case '<':
        case '>':
        case '\n':
        case '\t':
        case '#':
        case '\r':
            return true;
            break;
        default:
            return false;
            break;
    }
}

command_stream_t
make_command_stream (int (*get_next_byte) (void *),
		     void *get_next_byte_argument)
{
  char current;
  char* buffer = checked_malloc(sizeof(char));
  size_t unpair = 0;
  char last = '\0';      // last character read
  char last_nospace = '\0';
  bool AND_FLAG = false;        // if && then true
  bool OR_FLAG = false;         // if || then true
  bool COMMENT_FLAG = false;
  size_t allocSize = 0;

  while ((current = get_next_byte(get_next_byte_argument)) != EOF)
  {
    if (!isValid(current))
        error(1, 0, "Invalid character!");
    if (current == '(')
        unpair++;
    if (current == ')')
        unpair--;

    if ((current == ' ' || current == '\n' || current == '\r') && (last_nospace == '\0'))
        continue;
    if (current == ';' && (last_nospace == '\n' || last == '\0'))
        error(1, 0, "Invalid semicolon!");
    if ((current == '<' && last_nospace == '<') || (current == '>' && last_nospace == '>'))
        error(1, 0, "Invalid redirection!");
    if (current == ';')
        current = '\n';

    // Check for &&& and |||
    if (AND_FLAG == true && current == '&')
        error(1, 0, "Invalid &");
    if (OR_FLAG == true && current == '|')
        error(1, 0, "Invalid |");

    // Set flag to be true if && or ||
    if (current == '&' && last == '&' && AND_FLAG == false)
        AND_FLAG = true;
    else
        AND_FLAG = false;

    if (current == '|' && last == '|' && OR_FLAG == false)
        OR_FLAG = true;
    else
        OR_FLAG = false;

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
        error(1, 0, "Invalid comment!");


      if (current == '#')
        COMMENT_FLAG = true;

      if (current == '\n' && COMMENT_FLAG == true)
      {
        COMMENT_FLAG = false;
        continue;
      }

      if (current == '\n' && (last_nospace == '|' || last_nospace == '&'))
        continue;

      if (current == '\n' && (last_nospace == '>' || last_nospace == '<'))
        error(1, 0, "Invalid redirection!");

      if (current == '\n' && last == '\n')
        continue;

      if(!COMMENT_FLAG)
      {
        allocSize++;
        buffer = checked_realloc(buffer, (2+allocSize)*sizeof(char));   // 1 for null-byte and 1 for next char
        //append
        size_t length = strlen(buffer);
        buffer[length] = current;
        buffer[length+1] = '\0';
        last = current;
        if (current != ' ')
          last_nospace = current;
      }

  }

  // MAYBE \n SYNTAX VALIDATION???

  if (last == ')' && unpair != 0)
    error(1, 0, "Unpaired parantheses!");
  if (unpair != 0)
    error(1, 0, "Unpaired parantheses!");

  token_stream* stream = Tokenizer(buffer);

  command_stream_t cmd_stream = checked_malloc(sizeof(struct command_stream));
  cmd_stream->size = 0;
  cmd_stream->iterator = 0;

  while (stream != NULL)
  {
    // make commands from tokens
  }

  free(buffer);
  return cmd_stream;

}

command_t
read_command_stream (command_stream_t s)
{
  if (s->iterator >= s->size)
    return NULL;
  return s->command[(s->iterator++)];
}

