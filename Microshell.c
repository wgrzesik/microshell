#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

void getCurrentDirectory();
void getLogin();
void printPrompt();
void command_help();

char path[512], USER[512];
char command[1024];

int main()
{


   while (strcmp(command,"exit")!=0){
    printPrompt();
    scanf("%s", command);

    if (strcmp(command, "help")==0){
       command_help();
    }else {

    }

   }

    return 0;
}


/*Commands*/
void command_help()
{
    printf("Wiktoria Grzesik - Projekt Microshell\n");
    printf("Available commands: exit\n");
}

/*Prompt*/
void getCurrentDirectory()
{
    getcwd(path,sizeof(path));
    printf("%s]\n$ ",path);
}

void getLogin()
{
    printf("[%s : ", getenv("USER"));
}

void printPrompt()
{
    getLogin();
    getCurrentDirectory();
}
