#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/types.h>

void getCurrentDirectory();
void printPrompt();
void quotationmarks();
void command_help();
void command_exit();
void read_command();
void command_cd();

char path[512], USER[512];
char input[1024];
char *parameters[20], *input_p, *command, *param_p;
#define numbercommands 3
char *my_commands[numbercommands] = {"help", "exit", "cd"}; /*have to update */


int main()
{

    while(1){

        printPrompt();
        read_command();

        if (command != NULL){

         if (strcmp(command, "help") == 0 ){
            command_help();
        }else if (strcmp(command, "exit") == 0 ){
            command_exit();
        }else if (strcmp(command, "cd") == 0){
            command_cd();

        }else {

            pid_t pid = fork();

             if ( pid == 0){

                if (execvp(command, parameters) == -1){  // execvp(), bo parameters jest tablica
                    printf("Blad (%d) : %s\n",errno, strerror(errno));
                    exit(1);
               }

            }else {

                wait(NULL);

            }
        }
   }
    }

    return 0;
    exit(0);
}

void read_command() /* gets input and parse it */
{
    fgets(input, 1024, stdin);
    quotationmarks();
    input_p = input;
    input_p = strtok(input_p, "\n");

    param_p = strtok(input_p, " ");
    command = param_p;
    int i = 0;
    while (param_p != 0){
        parameters[i] = param_p;
        param_p = strtok(NULL, " ");
        i++;
    }
    parameters[i++] = NULL;

}

void quotationmarks() /* converts if input is in " " */
{
    int len = strlen(input);

  	for(int i = 0; i < len; i++)
	{
		if(input[i] == 34)
		{
			for(int j = i; j < len; j++)
			{
				input[j] = input[j + 1];
			}
			len--;
			i--;
		}
	}

}


/*Commands*/

void command_help()
{
    printf("Wiktoria Grzesik - Projekt Microshell\n");
    printf("Available commands : help, exit, cd\n");

}

void command_exit()
{
    exit(0);
}

void command_cd()
{

    if (parameters[1] != NULL){

        if (strcmp(parameters[1], "~") == 0){

            chdir(getenv("HOME"));

        }else if(chdir(parameters[1]) == -1){

                printf("Blad (%d) : %s\n",errno, strerror(errno));
            }
}
}


/*Prompt*/
void getCurrentDirectory()
{
    getcwd(path,sizeof(path));
    printf("%s]\n$ ",path);
}


void printPrompt()
{
    printf("[%s : ", getenv("USER"));   //gets login
    getCurrentDirectory();
}


 /* FORK()
  int pid;
    pid = fork();

    if (pid > 0 ){

        printf("Parent pid : %d\n", getpid());

    }else if (pid == 0){

        sleep(2);
        printf("Child pid : %d\n", getpid());
        printf("My parent pid : %d\n", getppid());

    }
    exit(0);



            if (pid == -1){
                return 1;
            }
            if (pid){
                PID = getpid();
                PPID = getppid();

                printf("[parent] pid=%d Ppid=%d \n", PID, PPID);

            }





        }*/
