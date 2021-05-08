#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

#define MAX_SIZE 100
#define DELIMITERS " \t\r\n\a\0"

char *system_commands[50] = {
    "/bin/",
    "/usr/bin/",
    "./",
    NULL
};

char error_message[30] = "An error has occurred\n";

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

char ** split_line(char *command_line, int *n);
int command_cd(char **args);
int wish_launch(char **args);
void command_exit(char **args);
void command_path(char **args);
int wish_launch_redirect(char **args, char *file);
int absolute_path(char **arg_2);
int find_parallel_paths(int n, int ampersan_position[n], char **arg_2);
int parallel_paths(int n, int ampersan_position[n] , char **arg_2);
int wish_launch_parallel(int n, int ampersan_position[n] , char **arg_2);

int main(int argc, char*argv[]){
    char str[MAX_SIZE];
    int *n = (int*)malloc(sizeof(int*));
    if(argc == 1){/*interactivo*/
        do{
            printf("wish> ");
            fgets(str, MAX_SIZE, stdin);
            char * p = str;
            while(*p != '\n'){
                p++;
            }
            *p = '\0';
            //char ** arg_2=  malloc(MAX_SIZE * sizeof(char*));
            char ** arg_2 = split_line(str,n);

            builtin_command command = str_to_command(arg_2[0]);
            
            
            if(command != not_command){
                switch(command){
                    case cd:
                        command_cd(arg_2);
                        break;
                    case path:
                        command_path(arg_2);
                        
                        break;
                    case endup:
                        command_exit(arg_2);
                        break;
                    default:
                        write(STDERR_FILENO, error_message, strlen(error_message));
                        //perror("wish: unkwon command ");
                }
            } else {
                int result=0;
                int i=0;
                if(*n==0){
                    int exist_path=absolute_path(arg_2);
                    if(exist_path==1){
                        int i_found=0,aux=0;
                        do{
                            if (strcmp(arg_2[i],">")==0 ){
                                aux=aux+1;
                                i_found=i;
                            }else if(strchr(arg_2[i],'>')!=NULL){
                                aux=aux+2;
                            }
                            i++;
                        }while (arg_2[i]!=NULL);
                        if(aux==0){
                            //no encontro ningun >
                            wish_launch(arg_2);
                        }else if(aux==1){
                            //encontro 1 >
                            if(arg_2[i_found+1]==NULL){
                                write(STDERR_FILENO, error_message, strlen(error_message));
                                //printf("wish: unkwon output file \n");
                            }
                            else if(arg_2[i_found+2]!=NULL){
                                //printf("wish: multiple output file  \n");
                                write(STDERR_FILENO, error_message, strlen(error_message));
                            }
                            else{
                                char *file  = malloc(MAX_SIZE * sizeof(char*));
                                strcpy(file,arg_2[i_found+1]);
                                arg_2[i_found]=NULL;
                                arg_2[i_found+1]=NULL;
                                wish_launch_redirect(arg_2,file);
                            }
                        }
                        else{
                            //printf("wish: multiple >  \n");
                            write(STDERR_FILENO, error_message, strlen(error_message));
                        }
                    }
                    else{
                        //printf("wish: not found command in path");
                        write(STDERR_FILENO, error_message, strlen(error_message));
                    }
                }
                else{
                    //printf("entro en paralelo \n");
                    //paralelo
                    int ampersan_position[*n];
                    for(int i=0;i<*n;i++){
                        ampersan_position[i]=0;
                    }
                    int parallel_error=find_parallel_paths(*n,ampersan_position,arg_2);
                    /*for(int i=0;i<*n;i++){
                        printf("%d\n",ampersan_position[i]);
                    }*/
                    
                    if(parallel_error>0){
                        //printf("encontro los & paralelo \n");
                        parallel_error=parallel_paths(*n,ampersan_position,arg_2);
                        if(parallel_error>0){
                          /*  printf("asigno los path paralelo \n");
                            printf("el comando 0 esta en :%s\n",arg_2[0]);
                            for(int i=0;i<*n;i++){
                                printf("el comando %d esta en :%s\n",i+1,arg_2[ampersan_position[i]+1]);
                            }*/
                            wish_launch_parallel(*n,ampersan_position,arg_2);
                            //printf("ejecuto launch paralelo \n");
                        }else{
                            write(STDERR_FILENO, error_message, strlen(error_message));
                        }

                    }else{
                        write(STDERR_FILENO, error_message, strlen(error_message));
                    }
                    
                    
                }
                
            }
        }while(1);
    }else{
        perror("wish: bash command ");
        /*bash*/

    }
    
}

int find_parallel_paths(int n, int ampersan_position[n] , char **arg_2){
    int i=0,aux=0;
     do{
        if(strcmp(arg_2[i],"&")==0){
            if(arg_2[i+1]==NULL || strcmp(arg_2[i+1],"&")==0 ){
                return -1;
            }
            else{
                ampersan_position[aux]=i;
                aux+=1;
            }
        }else if(strchr(arg_2[i],'&')!=NULL){
            return -2;
        }
        i++;
    }while (arg_2[i]!=NULL);
    return 1;
}
int parallel_paths(int n, int ampersan_position[n] , char **arg_2){
    int aux=0;
    int band=0;
    for(int i =0;i<=n;i++){
        band=0;
        int j=0;
        char *path1 = malloc(MAX_SIZE * sizeof(char*));
        do{
            strcpy(path1,"");
            strcat(path1,system_commands[j]);
            strcat(path1,arg_2[aux]);
            if (access( path1 , X_OK) == 0 ){
                arg_2[aux] = path1;
                band=1;
                break;
            }
            j++;
        }while (system_commands[j]!=NULL);
        if(band!=1){
            return -1;
        }
        if(i<n){
            aux=ampersan_position[i]+1;
        }
        
    }
    
    return 1;
}

int absolute_path(char **arg_2){
    int i=0;
    char *path1 = malloc(MAX_SIZE * sizeof(char*));
    do{
        strcpy(path1,"");
        strcat(path1,system_commands[i]);
        strcat(path1,arg_2[0]);
        if (access( path1 , X_OK) == 0 ){
            arg_2[0] = path1;
            //result = wish_launch(arg_2);
            return 1;
        }
        i++;
    }while (system_commands[i]!=NULL);

    return 0;
}

int command_cd(char **args){
  if (args[1] == NULL || args[2] != NULL)  {
    fprintf(stderr, "wish: expected argument to \"cd\"\n");
  } else {
    if (chdir(args[1]) != 0) {
      perror("wish");
    }
  }
  return 1;
}

void command_exit(char **args){
  if (args[1] != NULL)  {
    fprintf(stderr, "wish: not expected argument to \"exit\"\n");
  } else {
    exit(0);
  }  
}
void command_path(char **args){
  int i=0;
  if (args[1] == NULL)  {
    printf("se pierde : %s\n", system_commands[0]);
    //i=0;
    do{
      printf("%s\n", system_commands[i]);
      i++;
    }while (system_commands[i]!=NULL);
  } else {
    //i=0;
    do{
      //printf("el %d a guardar es :%s\n",i,args[i+1]);
      //printf("antes el %d en sc es:%s\n",i,system_commands[i]);
      system_commands[i] = NULL;
      system_commands[i] = args[i+1];
      //printf("despues el %d en sc es:%s\n",i,system_commands[i]);
      //strcpy(system_commands[i],args[i+1] );
      i++;
    }while (args[i]!=NULL);
    //strcpy(system_commands[i], NULL);
    //system_commands[i]=NULL;
    
  }  
}

char ** split_line(char *command_line, int *n){
    int position = 0;
    char **tokens = malloc(MAX_SIZE * sizeof(char*));
    char *token;
    int aux=0;
    token = strtok(command_line, DELIMITERS);
    while (token != NULL) {
        tokens[position] = token;
        if(strcmp(token,"&")==0){
            aux+=1;
        }
        position++;
        token = strtok(NULL, DELIMITERS);
    }
    tokens[position] = NULL;
    *n=aux;
    return tokens;
}

int wish_launch(char **args){
    pid_t pid,wpid;
    int status;
    pid = fork();
    if (pid == 0) {
    // Child process
        execv(args[0], args);
    } else if (pid < 0) {
        // Error forking
        perror("wish");
    } else {
        // Parent process
       wait(NULL);
    }
    return 1;
}

int wish_launch_parallel(int n, int ampersan_position[n] , char **arg_2){
    int status;
    pid_t pids[n+1];
    /*for(int i=0;i<=n;i++){
        pids[i]=0;
    }*/
    int aux=0;
    for(int i =0;i<=n;i++){
        
        
        // Child process
            char ** arg = malloc(MAX_SIZE * sizeof(char*));
            int p=0;
            do{
                arg[p]=arg_2[aux];
                printf("p value:%d  arg :%s aux value%d\n",p,arg[p],aux);
                p+=1;
                aux+=1;
            }while(strcmp(arg_2[aux],"&")!=0 && arg_2[aux]!=NULL);
            arg[p]=NULL;
            for(int x=0; x<p;x++){
                printf("%s\n",arg[x]);
            }
            printf("contador");
            pid_t pid = fork();
            if (pid == 0) {
            execv(arg[0], arg);
            
            
        } else if (pid < 0) {
            // Error forking
            perror("wish");
        } else {
            // Parent process
            //wait(NULL);
            printf("en padre asigna nuevo pid en pids %d\n",i);
            pids[i]=pid;
        }
        if(i<n){
            aux=ampersan_position[i]+1;
        }else{
            aux+=1;
        }
    }
    //printf("esta en padre\n");
    for(int i=0;i<=n;i++){
        
        waitpid(pids[i],NULL,0);
        printf("%d\n",pids[i]);
    }
    //printf("terminaron los hijos\n");
    return 1;
}


int wish_launch_redirect(char **args, char *file){
    pid_t pid, wpid;
    int status;
    pid = fork();
    if (pid == 0) {
    // Child process
        int fd = open(file, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);

        dup2(fd, 1);   // make stdout go to file
        dup2(fd, 2);   // make stderr go to file - you may choose to not do this
                   // or perhaps send stderr to another file
        close(fd);
        execv(args[0], args);
    } else if (pid < 0) {
        // Error forking
        write(STDERR_FILENO, error_message, strlen(error_message));
        //perror("wish");
    } else {
        // Parent process
       wait(NULL);
    }
    return 1;
}