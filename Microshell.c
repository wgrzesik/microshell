#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/types.h>

#define MAXHISTORY 100        // <-- zmienic
#define MAXCOMMANDS 6
#define MAXINPUT 100

void printPrompt();
void quotationmarks();
void read_command();
void updatehistory();

void command_help();
void command_exit();
void command_cd();
void command_history();
void command_nfromhistory();
void command_lastfromhistory();

void RemoveRepeatsInHistory();
void offsethistory(int linia);

char path[512], USER[512], lastpath[512];
char input[MAXINPUT];
char history[MAXHISTORY][MAXINPUT];
int counter;
int hiscount = 0;
int len, num, linia;
int komenda;
int suma = 0;

char shell[5] = "shell";
char *parameters[10], *input_p, *command, *param_p, *archive;
//char *my_commands[MAXCOMMANDS] = {"help", "exit", "cd", "history", "!!", "!n"};


int main()
{
    strcpy(path, "/home/students/s473561");

    while(1){

        strcpy(lastpath,path);

        if (komenda == 0){
            RemoveRepeatsInHistory();
            printPrompt();
            fgets(input, 1024, stdin);
        }else {
            strcpy(input, history[num]);    //komenda = 1 -> uruchamiany main() w main() komendą jest wcześniej wybrane polecenie

        }

        read_command();
        updatehistory();
        hiscount++;

        RemoveRepeatsInHistory();


        if (command != NULL){

            if (strcmp(command, "help") == 0){
                command_help();
            }else if (strcmp(command, "exit") == 0 ){
                command_exit();
            }else if (strcmp(command, "cd") == 0){
                command_cd();
            }else if(strcmp(command, "history") == 0){
                command_history();
            }else if (strcmp(command, "!!") == 0){
                                                    //wykonywane jest ostatnie polecenie
                komenda = 1;
                command_lastfromhistory();


            }else if (command[0] == '!'){

                komenda = 1;
                command_nfromhistory();

            }else{

                pid_t pid = fork();

                if ( pid  == 0){

                    if (execvp(command, parameters) == -1){  // execvp(), bo parameters jest tablica
                    printf("-%s: %s: command not found\n",shell, command);
                    exit(1);
                    }

                }else {

                    wait(NULL);

                }
            }

            komenda = 0;

        }else{

            linia = hiscount;      //usuwanie pustych linijek z historii, czyli gdy wpiszemy enter
            offsethistory(linia);
        }
    }

    exit(0);

}

void read_command() /* gets input and parse it */
{
    quotationmarks();
    input_p = input;
    len = strlen(input_p);
    input_p = strtok(input_p, "\n");
    param_p = strtok(input_p, " ");
    command = param_p;
    int i = 0;
    counter = 0;

    while (param_p != 0){
        parameters[i] = param_p;
        param_p = strtok(NULL, " ");

        i++;

    }
    parameters[i++] = NULL;
    counter = i-1;


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
    if (counter == 1){

        printf("Wiktoria Grzesik - Projekt Microshell\n");
        printf("Available commands : help, exit, cd, history, !!, !n\n");

    }else{

        printf("-%s: %s: too many arguments\n", shell, command);
    }
}

void command_exit()
{
    if (counter == 1){

        exit(0);
    }else{

        printf("-%s: %s: too many arguments\n", shell, command);
    }
}

void command_cd()
{

    if (counter == 1){

        chdir(getenv("HOME"));

    }else if (counter == 2){

        if (strcmp(parameters[1], "~") == 0){

            chdir(getenv("HOME"));

        }else if(strcmp(parameters[1], "-") == 0){

            printf("%s\n", lastpath);
            chdir(lastpath);

        }else if(chdir(parameters[1]) == -1){

            printf("-%s: %s: %s: No such file or directory\n", shell, command, parameters[1]);
        }

    }else{
             printf("-%s: %s: too many arguments\n", shell, command);
    }
}
void command_history()
{
    if (counter > 3){

         printf("-%s: %s: too many arguments\n", shell, command);
    }else {

     if (parameters[1] != NULL){

        if (strcmp(parameters[1],"-c") == 0){       //usuwa cala historie

            memset(history, 0, MAXINPUT*MAXHISTORY);
            hiscount = 0;

         }else if ((strcmp(parameters[1],"-d") == 0) || (strcmp(parameters[1],"offset") == 0)){     //usuwa podana linie z historii

            if (counter == 3){

                if (atoi(parameters[2]) == 0){
                    printf("-%s: %s: %s: position out of range\n", shell, command, parameters[2]);

                }else{

                    int linia = atoi(parameters[2]);
                    offsethistory(linia);
                }

            }else {
                printf("-%s: %s: %s: option requires an argument\n",shell, command, parameters[1]);      //blednie podana flaga
                printf("history: usage: [-c] [-d offset] [n]\n");
            }

            }else if (atoi(parameters[1]) > 0 ){        //wypisuje n ostatnich polecen

                for (int j = (hiscount - atoi(parameters[1])); j < hiscount; j++){

                     printf("\t%d %s\n",j+1,history[j]);
                }

           }else{

                printf("-%s: %s: %s: invalid option\n",shell, command, parameters[1]);      //blednie podana flaga
                printf("history: usage: [-c] [-d offset] [n]\n");

         }


    }else {

        for (int j = 0;j < hiscount; j++){

            printf("\t%d %s\n",j+1,history[j]);

        }

    }

}
}



void offsethistory(int linia)
{

    if (abs(linia) > hiscount){       // czy to dodawac -> || (abs(linia)-hiscount == 0)

       printf("-%s: %s: position out of range\n", shell, command);

    }else {

        char history_buff[MAXHISTORY][MAXINPUT] = {};


        if ((linia < 0)  ){

            if (hiscount + linia == 0){

                 printf("-%s: %s: history position out of range\n", shell, command);
            }else{

                linia = hiscount + linia;
            }

        }else{

            linia -= 1;
        }

        for (int c = 0; c < hiscount; c++){
            strcpy(history_buff[c], history[c]);
        }

        memset(history, 0,  MAXHISTORY*MAXINPUT);

        for (int c = 0; c < linia; c++){

            strcpy(history[c], history_buff[c]);
        }

        for (int c = linia+1; c < hiscount; c++){

            strcpy(history[c-1], history_buff[c]);
        }


        memset(history_buff, 0,MAXHISTORY*MAXINPUT);

        hiscount -= 1;

    }
    linia = 0;
}
void updatehistory()
{

    if (hiscount == MAXHISTORY){    //sprawdzanie czy historia jest pełna, jeżeli tak to usuwamy najstarszy element, by zrobić miejsce na nowy

        linia = 1;
        offsethistory(linia);
   }

    char s[MAXINPUT] = {};

    for (int a = 0; a < counter; a++){

        strcat(s, parameters[a]);
        strcat(s, " ");
        }


        strcpy(history[hiscount], s);
}


void RemoveRepeatsInHistory()
{
    if (hiscount > 1){

            if (strcmp(history[hiscount - 1], history[hiscount - 2]) == 0){

                linia = hiscount - 1;
                offsethistory(linia);
            }

        }

}
void command_nfromhistory()    // !n - wykonuje n polecenie z historii, jezeli n < 0 to wykona n polecenie od konca listy
{

    int ujemna = 0;
    if (command[3] != '\0'){        //nie moze byc liczba trzycyfrowa, bo historia jest do 100 lini

        printf("-%s: %s: too big number, history holds up to 100 commands\n", shell, command);
    }else {

        if (command[2] != '\0'){
            if (command[1] == '-'){        //sprawdzenie czy liczba jest ujemna (np !-2)
                num = (command[2] - '0');
                ujemna = 1;
            }else {
                int num1 = (command[1] - '0');
                int num2 = (command[2] - '0');

                num = num1*10 + num2;
            }

        }else{

            num = (command[1] - '0');
        }

        if (num != 0){

            if (abs(num) < hiscount){   //podana liczba musi byc wieksza niz przechoywana ilosc polecen w historii

                if (ujemna == 1){

                    num = hiscount - num;

                }else{

                    num -= 1;
                }

                linia = hiscount;       //usuwa z historii komendę !n
                offsethistory(linia);

                printf("%s\n", history[num]);
                main();
                exit(0);

            }else{

                printf("-%s: %s: event not found\n", shell, command);
            }

        }else{

            printf("-%s: %s: position out of range\n", shell, command);
        }
    }

}

void command_lastfromhistory()
{

    linia = hiscount;       //usuwa z historii komendę !!
    offsethistory(linia);

    num = hiscount-1;
    printf("%s\n", history[num]);

    main();
    exit(0);
}
/*Prompt*/

void printPrompt()
{
    printf("[%s : ", getenv("USER"));   //gets login
    getcwd(path,sizeof(path));
    printf("%s]\n$ ",path);

    //memset(path, 0, sizeof path);
}

