// UCLA CS 111 Lab 1 command reading

#include "command.h"
#include "command-internals.h"
#include "alloc.h"

#include <error.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

/* FIXME: You may need to add #include directives, macro definitions,
   static function definitions, etc.  */

/* FIXME: Define the type 'struct command_stream' here.  This should
   complete the incomplete type declaration in command.h.  */

// Singly linked list of commands
struct command_stream {
  struct command_Node *head;
  struct command_Node *tail;
  int size;         
  int iterator;
}; 

struct command_Node {
  struct command *command;
  struct command_Node *next;
};

/* FIX SIZES LATER */
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

void delete_command(char* curr_command)
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

struct command_Node *head = NULL;
struct command_Node *tail = NULL;

/****************** Stack data structure ****************************/
typedef struct stack
{
    command_t command;
    struct stack* prev;   
} myStack;

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
  size_t num_parantheses = 0;
  char last = '\0'      // last character read
  char last_nospace = '\0';
  bool AND_FLAG = false;        // if && then true
  bool OR_FLAG = false;         // if || then true

  while ((current = get_next_byte(get_next_byte_argument)) != EOF)
  {
    if (!isValid(current))
        error(1, 0, "Invalid character!");
    if (current == '(')
        num_parantheses++;
    if (current == ')')
        num_parantheses--;

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

  }
  

  return 0;
}

command_t
read_command_stream (command_stream_t s)
{
  if (s->iterator >= s->size)
    return NULL;
  return s->command[(s->iterator++)];
}
