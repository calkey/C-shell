#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define norm      "\x1B[0m"
#define black     "\x1B[30m"
#define red       "\x1B[31m"
#define green     "\x1B[32m"
#define yellow    "\x1B[33m"
#define blue      "\x1B[34m"
#define magenta   "\x1B[35m"
#define cyan      "\x1B[36m"
#define white     "\x1B[37m"

int inputCd(char **args);
int inputInfo(char **args);
int inputExit(char **args);
int inputPwd(char **args);
int inputExecute(char **args);
int inputAdditional(char **args);
int inputColour(char **args);

char **parseInput(char *line);
char *readInput(void);

int fileIO(char ** args, char* outputFilePosition);
void shellLoop(void);

struct Builtin
{
    char* input;
    int (*function) (char**);
};

struct Builtin builtins[] =
{
    { "info", &inputInfo },
    { "exit", &inputExit },
    { "pwd", &inputPwd },
    { "cd", &inputCd },
    { "ex", &inputExecute },
    { "exb", &inputExecute},
    { "colour", &inputColour }
};


int numOfBuiltinFunctions()
{
  return sizeof(builtins) / sizeof(builtins[0]);
}

//function to change the colour of output text
int inputColour(char **args)
{
  if(strcmp(args[1],"red")==0)
  {
    printf("%s",red );
  }
  else if(strcmp(args[1],"blue")==0)
  {
    printf("%s",blue );
  }
  else if(strcmp(args[1],"green")==0)
  {
    printf("%s",green );
  }
  else if(strcmp(args[1],"magenta")==0)
  {
    printf("%s",magenta );
  }
  else if(strcmp(args[1],"cyan")==0)
  {
    printf("%s",cyan );
  }
  else if(strcmp(args[1],"yellow")==0)
  {
    printf("%s",yellow );
  }
  else if(strcmp(args[1],"white")==0)
  {
    printf("%s",white );
  }
  else if(strcmp(args[1],"black")==0)
  {
    printf("%s",black );
  }
  else if(strcmp(args[1],"norm")==0)
  {
    printf("%s",norm );
  }
}

int inputCd(char **args)
{
    int chdirRetrun = chdir(args[1]);
    if(chdirRetrun == -1)
    {
      printf("chdir() failed");
    }
    return 1;
}


int inputInfo(char **args)
{
  printf("COMP2211 Simplified Shell by sc16kh\n");
  return 1;
}


int inputExit(char **args)
{
  printf("%s",norm );
  return 0;
}

int inputPwd(char **args)
{
    char *currentDirectory = (char*) calloc(1024, sizeof(char));
	  printf("%s",getcwd(currentDirectory, 1024));
    return 1;
}

int inputAdditional(char **args)
{
    int systemReturn = system(args[0]);
    if (systemReturn == -1)
    {
      printf("System() failed");
    }
    return 1;
}


int inputExecute(char **args)
{

  bool pipe = false;
  bool fileRedirection = false;
  int argNumber = 0;
  int i;
  int lessThanCharacterPosition;
  int outputFilePosition;

  //find number of arguments after 'ex' or 'exb'
  for(i=1; args[i]!=NULL; i++)
  {
    argNumber += 1;
  }

  char **argsWeWant = malloc((argNumber+1)*sizeof(char*));

  for(i=0; i<argNumber; i++)
  {
    argsWeWant[i] = malloc((256)*sizeof(char));
  }

  //create new string array containing all arguments excluding 'ex' or 'exb'
  for(i=0; i<argNumber; i++)
  {
    argsWeWant[i] = args[i+1];
  }

  for(i=0; i<argNumber; i++)
  {
    if (strcmp(">", argsWeWant[i])==0)
    {
      lessThanCharacterPosition = i;
      outputFilePosition = i+1;
      fileRedirection = true;
      break;
    }
  }

  char **argsForFileRedirection = malloc((lessThanCharacterPosition+1)*sizeof(char*));
  for(i=0; i<lessThanCharacterPosition;i++)
  {
    argsForFileRedirection[i] = malloc((256)*sizeof(char));
  }

  //array of strings before the '>' character
  for(i=0; i<lessThanCharacterPosition; i++)
  {
    argsForFileRedirection[i] = argsWeWant[i];
  }


  if(fileRedirection)
  {
    fileIO(argsForFileRedirection, argsWeWant[outputFilePosition]);
    excecuteProgram(argsForFileRedirection, 0);
  }

  int amountOfPipes = 0;
  for(i=0; i<argNumber; i++)
  {
    if(strcmp(argsWeWant[i],"|")==0)
    {
      pipe = true;
      amountOfPipes++;
    }
  }

  if(pipe)
  {
    pipelineCommands(argsWeWant, amountOfPipes);
  }
  else if(strcmp(args[0],"ex")==0 && !fileRedirection)
  {
    excecuteProgram(argsWeWant,0);
  }
  else if(strcmp(args[0],"exb")==0 && !fileRedirection)
  {
    excecuteProgram(argsWeWant,1);
  }

  return 1;
}

int excecuteProgram(char **argsWeWant, int option)
{
  int childStatus;
  int i;

  pid_t childPid, wPid;

  childPid = fork();

  if (childPid == 0)
  {
    // Child process
    int execvpReturn = execvp(argsWeWant[0], argsWeWant);
    if(execvpReturn==-1)
    {
      printf("Argument error\n");
    }

    exit(EXIT_FAILURE);
  }
  else if (childPid == -1)
  {
    // Error forking
    printf("Error forking\n");
  }
  else
  {
    if(option == 0)
    {
      do
      {
        wPid = waitpid(childPid, &childStatus, WUNTRACED);
      }
      while (!WIFEXITED(childStatus) && !WIFSIGNALED(childStatus));
    }
    else if(option == 1) //background process
    {
      //do nothing
    }
  }

  return 1;
}




int pipelineCommands(char ** args, int pipes)
{
  const int commands = pipes + 1;
  int i = 0;

  int pipefds[2*pipes]; //array of pipe file descriptors

  //create all needed pipes
  for(i = 0; i < pipes; i++)
  {
    if(pipe(pipefds + i*2) < 0)
    {
      printf("Couldn't Pipe\n");
      exit(EXIT_FAILURE);
    }
  }

  int childPid;
  int childStatus;

  int j = 0;
  int k = 0;
  int s = 1;
  int place;
  int commandStarts[10];
  commandStarts[0] = 0;

  //set pipes to null and create array of positions of commands
  while (args[k] != NULL)
  {
    if(strcmp(args[k], "|")==0)
    {
      args[k] = NULL;
      commandStarts[s] = k+2;
      s++;
    }
    k++;
  }

  for (i = 0; i < commands; ++i)
  {
    // place is where in args the program should
    // start running when it gets to the execution
    // command
    place = commandStarts[i];

    childPid = fork();
    if(childPid == 0)
    { //child process

      //if not first command, child gets input from previous command
      if(i < pipes)
      {
        if(dup2(pipefds[j + 1], 1) < 0)
        {
          printf("dup2 error\n");
          exit(EXIT_FAILURE);
        }
      }

      // child outputs to next command, if it's not the last command
      if(j != 0 )
      {
        if(dup2(pipefds[j-2], 0) < 0)
        {
          printf("dup2 error\n");
          exit(EXIT_FAILURE);
        }
      }

      //close all pipe file descriptors
      int q;
      for(q = 0; q < 2*pipes; q++)
      {
        close(pipefds[q]);
      }

      //execute command
      int execvpReturn =  execvp(args[place], args+place);
      if( execvpReturn == -1 )
      {
        perror(*args);
        exit(EXIT_FAILURE);
      }
    }
    else if(childPid == -1)
    {
      printf("Fork error\n");
      exit(EXIT_FAILURE);
    }

    //command = next command
    j+=2;
  }

  for(i = 0; i < 2 * pipes; i++)
  {
    close(pipefds[i]);
  }

  for(i = 0; i < pipes + 1; i++)
  {
    wait(&childStatus);
  }

}

int fileIO(char ** args, char* outputFile)
{

	int fd; // file descriptor
  pid_t childPid;

  childPid = fork();

	if(childPid==-1)
  {
		printf("Child process could not be created\n");
		return 1;
	}

  if(childPid == 0)
  {
    // create the file truncating it at 0, for write only
    fd = open(outputFile, O_CREAT | O_TRUNC | O_WRONLY, 0600);
    // replace the standard output with the created file
    dup2(fd, STDOUT_FILENO);
    close(fd);

    int execvpReturn = execvp(args[0],args);
    if(execvpReturn==-1)
    {
      printf("Argument error\n");
    }
  }


	waitpid(childPid,NULL,0);
  return 1;
}


int runFunction(char **args)
{
  int i;
  bool unknownCommand = false;
  if (args[0] == NULL)
  {
    // An empty command was entered.
    return 1;
  }

  int x = numOfBuiltinFunctions();
  /*
   * Cycle through each of the builtin functions, if the first argument given
   * by the user matches any, run it.
   */
  for (i = 0; i < x; i++)
  {
    if (strcmp(args[0], builtins[i].input) == 0)
    {
      return (*builtins[i].function)(args);
    }
    else
    {
      unknownCommand = true;
    }
  }

  if(unknownCommand)
  {
    inputAdditional(args);
  }

}

char *readInput(void)
{
  int bufsize = 256;
  int position = 0;
  char *buffer = malloc(sizeof(char) * bufsize);
  int c;

  if (!buffer)
  {
    printf("Allocation error\n");
    exit(EXIT_FAILURE);
  }

  while (1)
  {
    c = getchar();

    if (c == EOF || c == '\n')
    {
      buffer[position] = '\0'; //set final position to null
      return buffer; //we have reached the end of input so return the array
    }
    else
    {
      buffer[position] = c;
    }
    position++;


    if (position >= bufsize)
    {
      bufsize += 256;
      buffer = realloc(buffer, bufsize);
      if (!buffer)
      {
        printf("Allocation error\n");
        exit(EXIT_FAILURE);
      }
    }
  }
}




char **parseInput(char *line)
{
  int bufsize = 64;
  int position = 0;
  char **tokens = malloc(bufsize * sizeof(char*));
  char *token;

  if (!tokens)
  {
    printf("Allocation error\n");
    exit(EXIT_FAILURE);
  }

  token = strtok(line, " \t\r\n\a");
  while (token != NULL)
  {

    tokens[position] = token;
    position++;

    if (position >= bufsize)
    {
      bufsize += 64;
      tokens = realloc(tokens, bufsize * sizeof(char*));
      if (!tokens)
      {
        printf("Allocation error\n");
        exit(EXIT_FAILURE);
      }
    }

    token = strtok(NULL, " \t\r\n\a");
  }
  tokens[position] = NULL;
  return tokens;
}


void shellLoop(void)
{
  char *line;
  char **args;
  int status;

  do
  {
    printf("> ");
    line = readInput();
    args = parseInput(line);
    status = runFunction(args);

    free(line);
    free(args);
  }
  while (status);
}


int main(int argc, char **argv)
{
  shellLoop();
  return EXIT_SUCCESS;
}
