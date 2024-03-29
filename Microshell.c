#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdbool.h>
#include <errno.h>

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
void Command_Cp();
void Command_History();
void Command_nFromHistory();
void Command_lastFromHistory();

void PrintPrompt();
void QuotationMark();
void ReadCommand();
void UpdateHistory();
void RemoveRepeatsInHistory();
bool IsSpaceTheFirstCharacter();
void RemoveIfSpaceIsFirstCharacter();
void OffsetHistory(int line_number);
void StoreHistory(char *letter);
void History_r();

char USER[512], path[512], lastpath[512], input[MAXINPUT], history[MAXHISTORY][MAXINPUT], space_buff;
char *parameters[10], *input_p, *command, *param_p;
char shell[5] = "shell";
char *file_name;
char redirection[MAXINPUT];
char *parameters1[5], *parameters2[5];

int num, line_number, command_counter;
int hiscount = 0;
int recursively_main = 0;

FILE *historyfile, *new_file1, *new_file2;



int main()
{
    strcpy(path, "/home/students/s473561");

    while(1){

        strcpy(lastpath,path);

        if (recursively_main == 0){

            RemoveRepeatsInHistory();
            PrintPrompt();
            fgets(input, MAXINPUT, stdin);

        }else {

            strcpy(input, history[num]);
        }

        ReadCommand();
        UpdateHistory();
        hiscount++;

        RemoveRepeatsInHistory();
        RemoveIfSpaceIsFirstCharacter();


        if ((strcmp(redirection, "overwrite") == 0) || (strcmp(redirection, "append") == 0) || (strcmp(redirection, "stdin") == 0) ){

            if (parameters[command_counter - 1] != NULL){

                file_name = parameters[command_counter - 1];
                parameters[command_counter - 1] = NULL;
                parameters[command_counter - 2] = NULL;
                command_counter = command_counter - 2;

            }
        }

        if (command != NULL){

            if (strcmp(command, "help") == 0){
                Command_Help();
            }else if (strcmp(command, "exit") == 0 ){
                Command_Exit();
            }else if (strcmp(command, "cd") == 0){
                Command_Cd();
            }else if(strcmp(command, "cp") == 0){
                Command_Cp();
            }else if(strcmp(command, "history") == 0){
                Command_History();
            }else if (strcmp(command, "!!") == 0){

                recursively_main = 1;
                Command_lastFromHistory();

            }else if (command[0] == '!'){

                recursively_main = 1;
                Command_nFromHistory();

            }else{

                int fd;

                int my_stdout = dup(1);

                pid_t pid = fork();

                if ( pid  == 0){

                    if (strcmp(redirection, "overwrite") == 0){

                        fflush(stdout);
                        fd = open(file_name,  O_WRONLY|O_CREAT|O_TRUNC, 0777);
                        dup2(fd, 1);
                        close(fd);

                    }else if (strcmp(redirection, "append") == 0){

                        fflush(stdout);
                        fd = open(file_name, O_APPEND | O_WRONLY |O_CREAT, 0777);
                        dup2(fd, 1);
                        close(fd);

                    }

                    if (execvp(command, parameters) == -1){

                        dup2(my_stdout, 1);
                        close(my_stdout);
                        printf(RED "-%s: %s: command not found\n" WHITE ,shell, command);
                        exit(1);

                    }

                }else {

                    wait(NULL);

                }

            }
            recursively_main = 0;
            memset(redirection, 0, MAXINPUT);


        }else{

            line_number = hiscount;      // usuwa puste linie z historii - gdy zostanie wpisany enter
            OffsetHistory(line_number);
        }
    }

    StoreHistory("a");
    exit(0);


}

void PrintPrompt()      // wyświetla login i path
{
    getcwd(path,sizeof(path));
    printf( GREEN "[" BLUE "%s"  GREEN ":" PINK "%s" GREEN "]" NEON_BLUE "\n$ " WHITE, getenv("USER"), path);

}

void ReadCommand()      // wczytuje polecenie i zapisuje do tablicy wslaźników
{
    QuotationMark();

    input_p = input;
    input_p = strtok(input_p, "\n");

    param_p = strtok(input_p, " ");
    command = param_p;

    int i = 0;
    while (param_p != 0){

        parameters[i] = param_p;

        if (strcmp(param_p, ">") == 0){     // sprawdza czy jest przekierowanie > lub >>

           strcpy(redirection, "overwrite") ;

        }else if (strcmp(param_p, ">>") == 0){

            strcpy(redirection, "append");

        }

        param_p = strtok(NULL, " ");
        i++;
    }
    parameters[i++] = NULL;
    command_counter = i-1;


}

void QuotationMark()       // przekształca komendę jeżeli jest w cudzysłowie
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
    if (hiscount == MAXHISTORY){    // sprawdza czy historia jest pełna, jeżeli tak to usuwany jest najstarszy element, by zrobić miejsce na nowy

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

void RemoveRepeatsInHistory()   // usuwa powtarzające się linjki w historii
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

        if (strcmp(redirection, "overwrite") == 0){

            new_file1 = fopen(file_name,"w");

            fprintf(new_file1, BROWN "\tWiktoria Grzesik - Projekt Microshell\n");
            fprintf(new_file1, YELLOW "Available commands : help, exit, cd, cp, history, !!, !n\n" WHITE);

            fclose(new_file1);


        }else if (strcmp(redirection, "append") == 0){

            new_file1 = fopen(file_name,"a");

            fprintf(new_file1, BROWN "\tWiktoria Grzesik - Projekt Microshell\n");
            fprintf(new_file1, YELLOW "Available commands : help, exit, cd, cp, history, !!, !n\n" WHITE);

            fclose(new_file1);

        }else{

            printf(BROWN "\tWiktoria Grzesik - Projekt Microshell\n");
            printf(YELLOW "Available commands : help, exit, cd, cp, history, !!, !n\n" WHITE);

        }

    }else{

        printf(RED "-%s: %s: too many arguments\n" WHITE , shell, command);
    }
}


void Command_Exit()
{
    if (command_counter == 1){

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

void Command_Cp()
{
    if (command_counter == 3){

        if (strcmp(parameters[1], parameters[2]) != 0 ){

            char *source = parameters[1];
            char *destination = parameters[2];
            char cp_buffer[1024];

            int file_source = open(source, O_RDONLY);

            if (file_source == -1){

                printf(RED "-%s: %s: cannot stat '%s': No such file or directory\n" WHITE, shell, command, parameters[1]);

            }else {

                int number_of_bytes = read(file_source, cp_buffer, 1024);
                close(file_source);

                if (number_of_bytes == -1){

                   printf(RED "-%s: %s: '%s' is a directory\n" WHITE, shell, command, parameters[1]);

                }else {

                    int file_destination = open(destination, O_WRONLY|O_CREAT|O_TRUNC, 0666);

                    int written_number = write(file_destination, cp_buffer, number_of_bytes);

                    if (written_number == -1){

                        printf(RED "-%s: %s: '%s' is a directory\n" WHITE, shell, command, parameters[2]);
                    }

                    close(file_destination);
                }
            }

        }else {

        printf(RED "-%s: %s: " , shell, command);
        printf(RED "'%s' and '%s' are the same file\n "WHITE , parameters[2], parameters[2]);

        }

    }else if (command_counter == 2){

        printf(RED "-%s: %s: " WHITE, shell, command);
        printf(RED "missing destination file operand after '%s'\n", parameters[1]);

    }else if (command_counter == 1){

        printf(RED "-%s: %s: missing file operand\n" WHITE, shell, command);

    }else {

        printf(RED "-%s: %s: too many arguments\n" WHITE, shell, command);
    }

}

void Command_History()
{
    if (command_counter > 3){

         printf(RED "-%s: %s: too many arguments\n" WHITE , shell, command);

    }else {

        if (parameters[1] == NULL){    // gdy polecenie to "history", bez żadnej flagi

         //   if (redirection_overwrite == 1){
            if (strcmp(redirection, "overwrite") == 0){

                new_file2 = fopen(file_name, "w");

                for (int c = 0; c < hiscount; c++){

                    fprintf(new_file2, "%s\n", history[c]);

                }

                fclose(new_file2);

            }else if (strcmp(redirection, "append") == 0){

                new_file2 = fopen(file_name, "a");

                for (int c = 0; c < hiscount; c++){

                    fprintf(new_file2, "%s\n", history[c]);

                }

                fclose(new_file2);

            }else{

                 for (int j = 0;j < hiscount; j++){  // wypisanie historii

                printf("\t%d %s\n",j+1,history[j]);

                }
            }


        }else{

            if (strcmp(parameters[1],"-c") == 0){       // usuwa cala historie

                memset(history, 0, MAXINPUT*MAXHISTORY);
                hiscount = 0;

            }else if ((strcmp(parameters[1],"-d") == 0) || (strcmp(parameters[1],"offset") == 0)){     // usuwa podany numer lini z historii, jezeli cyfra ujemna to od ko

                    if (command_counter == 3){

                        if (atoi(parameters[2]) == 0){

                            printf(RED "-%s: %s: %s: position out of range\n" WHITE, shell, command, parameters[2]);

                        }else{

                            line_number = atoi(parameters[2]);
                            OffsetHistory(line_number);
                        }

                    }else {

                        printf(RED "-%s: %s: %s: option requires an argument\n",shell, command, parameters[1]);      // błędnie podana flaga do history -d
                        printf(RED "history: usage: [-c] [-r] [-w] [-a] [-d offset] [n]\n" WHITE);

                    }

            }else if((strcmp(parameters[1],"-w") == 0) || (strcmp(parameters[1],"-a") == 0)){   // w zależności od podanej flagi, wykonywane "history -w" lub "history -a"

                char *letter = &parameters[1][1];
                StoreHistory(letter);

            }else if (strcmp(parameters[1],"-r") == 0){

                History_r();

            }else if (atoi(parameters[1]) > 0 ){        // wypisuje n ostatnich polecen

                for (int j = (hiscount - atoi(parameters[1])); j < hiscount; j++){

                     printf("\t%d %s\n",j+1,history[j]);

                }

            }else{

                printf(RED "-%s: %s: %s: invalid option\n",shell, command, parameters[1]);      // błędnie podana flaga/polecenie
                printf(RED "history: usage: [-c] [-r] [-w] [-a] [-d offset] [n]\n" WHITE);

            }
        }
    }

}

void OffsetHistory(int line_number)   // usuwa podana linie z historii, jezeli podana liczba jest mniejsza od zera to usuwa n-tą linię od końca
{
    if (abs(line_number) > hiscount){

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

void StoreHistory(char *letter)     // dodaje liste historii do pliku shell_history (letter to "w" lub "a")
{                                                       // "a" dopisuje na końcu pliku, "w" nadpisuje plik

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



