#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>

// This code is given for illustration purposes. You need not include or follow this
// strictly. Feel free to writer better or bug free code. This example code block does not
// worry about deallocating memory. You need to ensure memory is allocated and deallocated
// properly so that your shell works without leaking memory. //

//Define the max number of arguments to be 20
#define MAX_ARGS 20
#define MAX_DIRECTORY_SIZE 1024
#define MAX_BG_JOBS 20

//Global variable for the main pid for the shell
pid_t mainPid;

int getcmd(char *currentDirectory, char *args[], int *background)
{
  int length, i = 0;
  char* token, *loc;
  char* line = NULL;
  size_t linecap = 0;
  printf("\nSHELL >> %s: ", currentDirectory);
  length = getline(&line, &linecap, stdin);

   if (length <= 0) {
     exit(-1);
   }
// Check if background is specified..
  if ((loc = index(line, '&')) != NULL) {
     *background = 1;
     *loc = ' ';
   }
   else {
     *background = 0;
   }

   while ((token = strsep(&line, " \t\n")) != NULL)
   {
     for (int j = 0; j < strlen(token); j++) {
      if (token[j] <= 32) {
           token[j] = '\0';
        }
      }
    if (strlen(token) > 0) {
      args[i++] = token;
    }
  }

  return i;
}

//Function that will quit any processes running in the shell if SIGINT (CTRL+C) is picked up
static void siginthandler (int signum) {

    pid_t currentPid = getpid();

    if (currentPid != mainPid) {
      exit(EXIT_SUCCESS);
    }

    printf("\n");

}

//Function that will ignore the SIGTSTP (CTRL+Z) signal
static void sigtstphandler (int signum) {
}

//Struct for background jobs
typedef struct backgroundJob {
  int key;
  pid_t jobPid;
  char command[10];
} backgroundJob;


int main(void)
{

  //Assign the main pid for the shell
  mainPid = getpid();

  signal(SIGINT, siginthandler);
  signal(SIGTSTP, sigtstphandler);

   time_t now;
   srand((unsigned int) (time(&now)));

   char* args[MAX_ARGS];

   int bg;

   //Key to be used for background jobs
   int bgJobCount = 1;

   while(1) {

     bg = 0;

     char buf[MAX_DIRECTORY_SIZE];
     char* currentDirectory;

     //Define an array which keeps track of bg jobs
     backgroundJob jobList[MAX_BG_JOBS];
     backgroundJob newJob;

     currentDirectory = getcwd(buf, sizeof(buf));

     int cnt = getcmd(currentDirectory, args, &bg);

     //Check to see if the user requested to fg before forking
     if (strcmp("fg", args[0]) == 0 ) {

       //Obtain the key of the job that the user would like to foreground
       int requestedJob = atoi(args[1]);


       //If the key exists, wait on that job to finish before continuing
       if (requestedJob < bgJobCount && requestedJob !=0) {
         pid_t requestedPid = jobList[requestedJob].jobPid;
         int jobStatus;
         waitpid(requestedPid, &jobStatus, WUNTRACED);
       }


       //Obtain the next command
       continue;
     }

     //Check if the user wants to change the working directory before forking
     if (strcmp("cd", args[0]) == 0) {
       char* directory = args[1];
       chdir(directory);

       //Obtain the next command
       continue;
     }

     //Check if the user specified the jobs command
     if (strcmp("jobs", args[0]) == 0 ) {

       printf("ID\tName\tPID\n");

       //Print out the background jobs
       for (int i=1; i<bgJobCount; i++) {
         printf("[%d]\t %s\t %ld\n", jobList[i].key, jobList[i].command, (long)jobList[i].jobPid);
       }

       //Obtain the next command
       continue;
     }

     //Check if the user wishes to exit the shell
     if (strcmp("exit", args[0]) == 0) {
       exit(EXIT_SUCCESS);
     }


     //Before forking, check if the user specified for bg and assign key
     if (bg) {
       newJob.key = bgJobCount++;
       strcpy(newJob.command, args[0]);
     }

     //Fork the child process
     pid_t pid = fork();

     //If necessary, assign the appropriate pid to the bg job
     if (pid && bg) {
       newJob.jobPid = pid;

      jobList[newJob.key] = newJob;
     }

     //Child process
     if (pid == 0) {

       //Set all the remaining arguments to be null
       for(int i=cnt; i<(MAX_ARGS-cnt); i++) {
         args[i] = NULL;
       }

         //Sleep before executing command
         int w;
         w = rand() % 10;
         sleep(w);

         //Execute the command
        execvp(args[0], args);


    }
     //Parent process
     else {
       int status;

       //If background was not specified, wait for the child to complete
       if (!bg) {
         waitpid(pid, &status, WUNTRACED);
      }

    }
  }
}
