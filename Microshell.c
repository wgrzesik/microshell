#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <stdbool.h>

#define MAXHISTORY 100
#define MAXCOMMANDS 6
#define MAXINPUT 50

#define RED "\033[91m"
#define WHITE "\033[97m"
#define GREEN "\033[32m"
#define BLUE "\033[94m"
#define PINK "\033[95m"
#define YELLOW "\033[93m"
#define BROWN "\033[33m"
#define NEON_BLUE "\033[96m"

void Command_Help();
void Command_Exit();
void Command_Cd();
void Command_History();
void Command_nFromHistory();
void Command_lastFromHistory();

void PrintPrompt();
void QuotationMark();
void ReadCommand();
void UpdateHistory();
void RemoveRepeatsInHistory();
void OffsetHistory(int line_number);
void StoreHistory(char *letter);
void History_r();
bool IsSpaceTheFirstCharacter();
void  RemoveIfSpaceIsFirstCharacter();

char USER[512], path[512], lastpath[512], input[MAXINPUT], history[MAXHISTORY][MAXINPUT], space_buff;
char *parameters[10], *input_p, *command, *param_p;
char shell[5] = "shell";

int num, line_number, command_counter;
int hiscount = 0;
int recursively_main = 0;

FILE *historyfile;

int main()
{
    strcpy(path, "/home/students/s473561");

    while(1){

        strcpy(lastpath,path);

        if (recursively_main == 0){

            RemoveRepeatsInHistory();
            PrintPrompt();
            fgets(input, 1024, stdin);

        }else {

            strcpy(input, history[num]);
        }

        ReadCommand();
        UpdateHistory();
        hiscount++;

        RemoveRepeatsInHistory();
        RemoveIfSpaceIsFirstCharacter();

        if (command != NULL){

            if (strcmp(command, "help") == 0){
                Command_Help();
            }else if (strcmp(command, "exit") == 0 ){
                Command_Exit();
            }else if (strcmp(command, "cd") == 0){
                Command_Cd();
            }else if(strcmp(command, "history") == 0){
                Command_History();
            }else if (strcmp(command, "!!") == 0){

                recursively_main = 1;
                Command_lastFromHistory();

            }else if (command[0] == '!'){

                recursively_main = 1;
                Command_nFromHistory();

            }else{

                pid_t pid = fork();
                if ( pid  == 0){

                    if (execvp(command, parameters) == -1){

                        printf(RED "-%s: %s: command not found\n" WHITE ,shell, command);
                        exit(1);

                    }

                }else {

                    wait(NULL);

                }
            }
            recursively_main = 0;


        }else{

            line_number = hiscount;      //usuwanie pustych linijek z historii, gdy zostanie wpisany enter
            OffsetHistory(line_number);
        }
    }

    StoreHistory("a");
    exit(0);

}

void PrintPrompt()  // wyświetla login i path
{
    getcwd(path,sizeof(path));
    printf( GREEN "[" BLUE "%s"  GREEN ":" PINK "%s" GREEN "]" NEON_BLUE "\n$ " WHITE, getenv("USER"), path);

}

void ReadCommand() // wczytuje polecenie i zapisuje do tablicy wslaźników
{
    QuotationMark();

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
    command_counter = i-1;
}

void QuotationMark() //przekształca komendę jeżeli jest w cudzysłowie
{
    int length = strlen(input);

  	for(int c = 0; c < length; c++)
	{
		if(input[c] == 34)
		{
			for(int l = c; l < length; l++)
			{
				input[l] = input[l + 1];
			}
			c--;
			length--;
		}
	}
}

void UpdateHistory()    // dodaje do historii wpisane polecenie
{
    if (hiscount == MAXHISTORY){    //sprawdza czy historia jest pełna, jeżeli tak to usuwany jest najstarszy element, by zrobić miejsce na nowy

        line_number = 1;
        OffsetHistory(line_number);
   }

    char input_buff[MAXINPUT] = {};

    for (int a = 0; a < command_counter; a++){

        strcat(input_buff, parameters[a]);
        strcat(input_buff, " ");

    }

    strcpy(history[hiscount], input_buff);

}

void RemoveRepeatsInHistory()   //usuwa powtarzające się linjki w historii
{
    if (hiscount > 1){

        if (strcmp(history[hiscount - 1], history[hiscount - 2]) == 0){

            line_number = hiscount - 1;
            OffsetHistory(line_number);

        }
    }
}

bool IsSpaceTheFirstCharacter()     // sprawdza czy pierwszy znak jest spacją
{
    if (((int)(input[0])) == 32){

        return true;

    }else{

        return false;

    }
}

void  RemoveIfSpaceIsFirstCharacter()   // usuwa z historii polecenia zaczynające się od spacji
{
    if (IsSpaceTheFirstCharacter()){

        line_number = hiscount;
        OffsetHistory(line_number);

    }
}

void Command_Help()
{
    if (command_counter == 1){

        printf(BROWN "\tWiktoria Grzesik - Projekt Microshell\n");
        printf(YELLOW " Available commands : help, exit, cd, history, !!, !n\n" WHITE);

    }else{

        printf(RED "-%s: %s: too many arguments\n" WHITE, shell, command);
    }
}

void Command_Exit()
{
    if (command_counter == 1){

        //char *letter = "a";
        StoreHistory("a");

        exit(0);

    }else{

        printf(RED"-%s: %s: too many arguments\n" WHITE, shell, command);
    }
}

void Command_Cd()
{
    if (command_counter == 1){

        chdir(getenv("HOME"));

    }else if (command_counter == 2){

        if (strcmp(parameters[1], "~") == 0){

            chdir(getenv("HOME"));

        }else if(strcmp(parameters[1], "-") == 0){

            printf("%s\n", lastpath);
            chdir(lastpath);

        }else if(chdir(parameters[1]) == -1){

            printf(RED "-%s: %s: %s: No such file or directory\n" WHITE , shell, command, parameters[1]);

        }

    }else{

        printf(RED "-%s: %s: too many arguments\n" WHITE, shell, command);

    }
}

void Command_History()
{
    if (command_counter > 3){

         printf(RED "-%s: %s: too many arguments\n" WHITE , shell, command);

    }else {

        if (parameters[1] == NULL){    // gdy polecenie sklada się z ponad jednej czesci, czyli ma flagi

            for (int j = 0;j < hiscount; j++){  // wypisanie historii

                printf("\t%d %s\n",j+1,history[j]);

            }

        }else{

            if (strcmp(parameters[1],"-c") == 0){       //usuwa cala historie

                memset(history, 0, MAXINPUT*MAXHISTORY);
                hiscount = 0;

            }else if ((strcmp(parameters[1],"-d") == 0) || (strcmp(parameters[1],"offset") == 0)){     //usuwa podany numer lini z historii, jezeli cyfra ujemna to od ko

                    if (command_counter == 3){

                        if (atoi(parameters[2]) == 0){

                            printf(RED "-%s: %s: %s: position out of range\n" WHITE, shell, command, parameters[2]);

                        }else{

                            line_number = atoi(parameters[2]);
                            OffsetHistory(line_number);
                        }

                    }else {

                        printf(RED "-%s: %s: %s: option requires an argument\n",shell, command, parameters[1]);      //blednie podana flaga do history -d
                        printf("history: usage: [-c] [-r] [-w] [-a] [-d offset] [n]\n" WHITE);

                    }

            }else if((strcmp(parameters[1],"-w") == 0) || (strcmp(parameters[1],"-a") == 0)){   //w zależności od podanej flagi, wykonywane "history -w" lub "history -a"

                char *letter = &parameters[1][1];
                StoreHistory(letter);

            }else if (strcmp(parameters[1],"-r") == 0){

                History_r();

            }else if (atoi(parameters[1]) > 0 ){        //wypisuje n ostatnich polecen

                for (int j = (hiscount - atoi(parameters[1])); j < hiscount; j++){

                     printf("\t%d %s\n",j+1,history[j]);

                }

            }else{

                printf(RED "-%s: %s: %s: invalid option\n",shell, command, parameters[1]);      //blednie podana flaga/polecenie
                printf("history: usage: [-c] [-r] [-w] [-a] [-d offset] [n]\n" WHITE);

            }
        }
    }

}

void OffsetHistory(int line_number)   //usuwa podana linie z historii, jezeli podana liczba jest mniejsza od zera to usuwa n-tą linię od końca
{
    if (abs(line_number) > hiscount){       // czy to dodawac -> || (abs(linia)-hiscount == 0)

       printf(RED "-%s: %s: position out of range\n" WHITE , shell, command);

    }else {

        char history_buff[MAXHISTORY][MAXINPUT] = {};

        if ((line_number < 0)  ){

            if (hiscount + line_number == 0){

                 printf(RED "-%s: %s: history position out of range\n" WHITE, shell, command);

            }else{

                line_number = hiscount + line_number;

            }

        }else{

            line_number -= 1;
        }

        for (int c = 0; c < hiscount; c++){

            strcpy(history_buff[c], history[c]);
        }

        memset(history, 0,  MAXHISTORY*MAXINPUT);

        for (int c = 0; c < line_number; c++){

            strcpy(history[c], history_buff[c]);
        }

        for (int c = line_number+1; c < hiscount; c++){

            strcpy(history[c-1], history_buff[c]);
        }

        memset(history_buff, 0, MAXHISTORY*MAXINPUT);
        hiscount -= 1;

    }

    line_number = 0;

}

void StoreHistory(char *letter)     // dodaje do liste historii do pliku shell_history (letter to "w" lub "a")
{

    historyfile = fopen("shell_history", letter);

    for (int c = 0; c < hiscount; c++){

        fprintf(historyfile, "%s\n", history[c]);

    }

    fclose(historyfile);
}

void History_r()    // polecenie "history -r" - czyta plik shell_history i dołącza zawartość do historii
{
    historyfile = fopen("shell_history", "r");

        char buff[MAXINPUT];
        char *buff_P;

        for(int c = 0; c < 10; c++){

            fgets(buff, sizeof buff, historyfile);

            buff_P = buff;
            buff_P = strtok(buff_P, "\n");

            strcpy(history[hiscount], buff_P);
            hiscount++;
        }

    memset(buff, 0, MAXINPUT);


    fclose(historyfile);
}

void Command_lastFromHistory() // !! - wykonuje ostatnie polecenie
{
    line_number = hiscount;       //usuwa z historii komendę "!!"
    OffsetHistory(line_number);

    num = hiscount-1;
    printf("%s\n", history[num]);

    main();
    exit(0);
}

void Command_nFromHistory()    // !n - wykonuje n-te polecenie z historii, jezeli n < 0 to wykona n-te polecenie od konca listy
{
    int negative_number = 0;

    if (command[3] != '\0'){        //nie moze byc liczba trzycyfrowa, bo historia jest do 100 lini

        printf(RED "-%s: %s: too big number, history holds up to 100 commands\n" WHITE, shell, command);

    }else {

        if (command[2] != '\0'){

            if (command[1] == '-'){        //sprawdzenie czy liczba jest ujemna (np !-2)

                num = (command[2] - '0');
                negative_number = 1;

            }else {

                int num1 = (command[1] - '0');
                int num2 = (command[2] - '0');

                num = num1*10 + num2;
            }

        }else{

            num = (command[1] - '0');
        }

        if (num != 0){

            if (abs(num) < hiscount){   //wartosc bezwzględna liczby musi być mniejsza niż ilość poleceń w historii

                if (negative_number == 1){

                    num = hiscount - num - 1;

                }else{

                    num -= 1;
                }

                line_number = hiscount;       //usuwa z historii komendę "!n"
                OffsetHistory(line_number);

                printf("%s\n", history[num]);
                main();
                exit(0);

            }else{

                printf(RED "-%s: %s: event not found\n" WHITE, shell, command);
            }

        }else{

            printf(RED "-%s: %s: position out of range\n" WHITE, shell, command);
        }
    }
}




