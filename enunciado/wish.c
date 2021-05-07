#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAX_SIZE 100
#define DELIMITERS " \t\r\n\a\0"

char *system_commands[] = {
    "./",
    "/usr/bin/",
    "/bin/",
    NULL
};

typedef enum{endup, cd, path, not_command} builtin_command;

const static struct{
    builtin_command command;
    char* string_command;
} commands[] = {
    {cd, "cd"},
    {path, "path"},
    {endup, "exit"}
};

builtin_command str_to_command(char * str_command){
    for(int i=0; i<3; i++){
        if(!strcmp(str_command,commands[i].string_command)){
            return commands[i].command;
        }
    }
    return not_command;
}

char ** split_line(char *comand_line);
int command_cd(char **args);
int wish_launch(char **args);

int main(int argc, char*argv[]){
    char str[MAX_SIZE];
    if(argc == 1){/*interactivo*/
        do{
        printf("wish> ");
        fgets(str, MAX_SIZE, stdin);
        char * p = str;
        while(*p != '\n'){
            p++;
        }
        *p = '\0';
        builtin_command command = str_to_command(str);
        arg_2= split_line(str)
        if(command != not_command){
            switch(command){
                case cd:
                    command_cd(arg_2)
                    printf("cd\n");
                    break;
                case path:
                    printf("path:\n "); //imprimir system_commands
                    for(int i=0; i<3; i++){
                        printf("  %s\n", commands[i].string_command);
                    }
                    break;
                case endup:
                    exit(0);
                    break;
                default:
                    printf("command not found\n");
            }
        } else {
            path1 = strcat("./",arg_2[0])
            path2 = strcat("/usr/bin/",arg_2[0])
            path3 = strcat("/bin/",arg_2[0])
            if (access( path1 , X_OK) !=0 ){
                strcpy(arg_2[0],path1)
                wish_launch(arg_2)//ejecutar fork 
            }
            else if( access( path2 , X_OK) !=0 ){
                strcpy(arg_2[0],path2)
                wish_launch(arg_2)//ejecutar
            }else if( access( path3 , X_OK) !=0 ){
                strcpy(arg_2[0],path3)
                wish_launch(arg_2)//ejecutar
            }
            else{
                perror("wish: unkwon command ");
            }
            
        
        }
        
        }while(1);
    }else{
        
        /*bash*/

    }
    
}

int command_cd(char **args){
  if (args[1] == NULL) {
    fprintf(stderr, "wish: expected argument to \"cd\"\n");
  } else {
    if (chdir(args[1]) != 0) {
      perror("wish");
    }
  }
  return 1;
}

char ** split_line(char *command_line){
    int position = 0;
    char **tokens = malloc(MAX_SIZE * sizeof(char*));
    char *token;
    token = strtok(command_line, DELIMITERS);
    while (token != NULL) {
        tokens[position] = token;
        position++;
        token = strtok(NULL, DELIMITERS);
    }
    tokens[position] = NULL;
    return tokens;
}

int wish_launch(char **args){
  pid_t pid, wpid;
  int status;

  pid = fork();
  if (pid == 0) {
    // Child process
    if (execvp(args[0], args) == -1) {
      perror("lsh");
    }
    exit(EXIT_FAILURE);
  } else if (pid < 0) {
    // Error forking
    perror("lsh");
  } else {
    // Parent process
    do {
      wpid = waitpid(pid, &status, WUNTRACED);
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
  }

  return 1;
}