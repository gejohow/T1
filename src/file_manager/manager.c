// Import used global libraries
#include <stdio.h>  // FILE, fopen, fclose, etc.
#include <string.h> // strtok, strcpy, etc.
#include <stdlib.h> // malloc, calloc, free, etc.
#include <signal.h>

// Import the header file of this module, because it has the struct declaration
#include "manager.h"

/*
* Splits a string "str" by a separator "sep", returns an array with The
* resulting strings. Equivalent to Python's str.split(sep).
*/
static char **split_by_sep(char *str, char *sep)
{
  char **new_str = calloc(MAX_SPLIT, sizeof(char *));
  int index = 0, len;

  char *token = strtok(str, sep);
  while (token != NULL)
  {
    new_str[index] = calloc(BUFFER_SIZE, sizeof(char));
    strcpy(new_str[index++], token);
    token = strtok(NULL, sep);
  }

  // Remove dangling Windows (\r) and Unix (\n) newlines
  len = strlen(new_str[index - 1]);
  if (len > 1 && new_str[index - 1][len - 2] == '\r')
    new_str[index - 1][len - 2] = '\0';
  else if (len && new_str[index - 1][len - 1] == '\n')
    new_str[index - 1][len - 1] = '\0';
  return new_str;
}

/*
* Reads a CSV with the following structure:
*
* 1. data,data,data,...
* ...
* n. data,data,data,...
*
* Where n is the FILE_SIZE compile-time constant
*/
InputFile *read_file(char *filename)
{
  // Read the file
  FILE *file_pointer = fopen(filename, "r");

  // Declare a buffer and read the first line
  char buffer[BUFFER_SIZE];

  // Define the struct and prepare its attributes
  InputFile *input_file = malloc(sizeof(InputFile));
  input_file->lines = calloc(FILE_SIZE, sizeof(char **));
  input_file->len = FILE_SIZE;

  // Parse each line
  int index = 0;
  while (fgets(buffer, BUFFER_SIZE, file_pointer))
  {
    input_file->lines[index++] = split_by_sep(buffer, ",");
  }

  // Close the file and return the struct
  fclose(file_pointer);
  return input_file;
}

/*
* Frees all the memory used by "input_file".
*/
void input_file_destroy(InputFile *input_file)
{
  // Free each separated data line and its contents
  for (int i = 0; i < input_file->len; i++)
  {
    for (int j = 0; j < MAX_SPLIT; j++)
    {
      free(input_file->lines[i][j]);
    }
    free(input_file->lines[i]);
  }

  // Free the lines array
  free(input_file->lines);

  // Free the input_file itself
  free(input_file);
}

/** Connects a signal to a more powerful handler
 * 
 * @param sig: Received signal (ex: SIGUSR1)
 * @param handler: Function to be called when the signal is received
*/
void connect_sigaction(int sig, void (*handler)(int, siginfo_t *, void *))
{
  // Define the sigaction struct required to setup the handler
  struct sigaction action;

  // Setup the handler function
  action.sa_sigaction = handler;

  // Set the mask as empty
  sigemptyset(&action.sa_mask);

  // Set SA_SIGINFO as the primary flag
  // This tells sigaction that the handler receives three parameters instead
  // of one
  action.sa_flags = SA_SIGINFO | SA_RESTART;

  // Associate the signal to the handler
  sigaction(sig, &action, NULL);
}

/** Sends a SIGUSR1 signal to a process with an int payload
 * 
 * @param pid: Process that will receive the signal
 * @param payload: Payload to send along with the signal
*/
void send_signal_with_int(int pid, int payload)
{
  union sigval sig = {};
  sig.sival_int = payload;
  sigqueue(pid, SIGUSR1, sig);
}
