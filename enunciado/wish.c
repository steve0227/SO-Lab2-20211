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

void run_shell(char str[MAX_SIZE], int*n,char**system_commands);
char ** split_line(char *command_line, int *n);
int command_cd(char **args);
int wish_launch(char **args);
void command_exit(char **args);

void command_path(char **args,char**system_commands);
int wish_launch_redirect(char **args, char *file);

int absolute_path(char **arg_2,char**system_commands);
int find_parallel_paths(int n, int ampersan_position[n], char **arg_2);

int parallel_paths(int n, int ampersan_position[n] , char **arg_2,char**system_commands);
int wish_launch_parallel(int n, int ampersan_position[n] , char **arg_2);

int main(int argc, char*argv[]){
    char **system_commands = NULL;
    system_commands = malloc(3 * sizeof(*system_commands)); 
    for (int i = 0; i < 3; i++){
        system_commands[i] = malloc((MAX_SIZE) * sizeof(*system_commands[i]));
    }
    strcpy(system_commands[0],".");
    strcpy(system_commands[1],"/bin");
    system_commands[2]=NULL;
    char str[MAX_SIZE];
    int *n = (int*)malloc(sizeof(int*));
    if(argc == 1){
        /*interactivo*/
        do{
            printf("wish> ");
            fgets(str, MAX_SIZE, stdin);
            char * p = str;
            while(*p != '\n'){
                p++;
            }
            *p = '\0';
            run_shell(str,n,system_commands);
        }while(1);
    }else{
        /*batch*/
        if (argc < 2) {
            write(STDERR_FILENO, error_message, strlen(error_message));
		    return EXIT_FAILURE;
	    }
	    FILE *fp=fopen(argv[1],"r");
    	if(!fp) {
            write(STDERR_FILENO, error_message, strlen(error_message));
		    return EXIT_FAILURE;
	    }

        while(fgets(str,MAX_SIZE,fp)) {
            if(strcmp(str,"")!=0 && strcmp(str,"\0")!=0 && strcmp(str," ")!=0 && strcmp(str,"\n")!=0 ){
                run_shell(str,n,system_commands);
            }
            
        }
    }
    
}

void run_shell(char str[MAX_SIZE], int*n,char**system_commands){
    char ** arg_2 = split_line(str,n);
    builtin_command command = str_to_command(arg_2[0]);
    if(command != not_command){
        switch(command){
            case cd:
                command_cd(arg_2);
                break;
            case path:
                command_path(arg_2,system_commands);
                break;
            case endup:
                command_exit(arg_2);
                break;
            default:
                write(STDERR_FILENO, error_message, strlen(error_message));
            }
    } else {
        int result=0;
        int i=0;
        if(*n==0){
            int exist_path=absolute_path(arg_2, system_commands);
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
                    }else if(arg_2[i_found+2]!=NULL){
                        write(STDERR_FILENO, error_message, strlen(error_message));
                    }else{
                        char *file  = malloc(MAX_SIZE * sizeof(char*));
                        strcpy(file,arg_2[i_found+1]);
                        arg_2[i_found]=NULL;
                        arg_2[i_found+1]=NULL;
                        wish_launch_redirect(arg_2,file);
                    }
                }else{
                    //encontro + de 1 >
                    write(STDERR_FILENO, error_message, strlen(error_message));
                }
            }else{
                write(STDERR_FILENO, error_message, strlen(error_message));
            }
        }else{
            //paralelo
            int ampersan_position[*n];
            for(int i=0;i<*n;i++){
                ampersan_position[i]=0;
            }
            int parallel_error=find_parallel_paths(*n,ampersan_position,arg_2);
            if(parallel_error>0){
                parallel_error=parallel_paths(*n,ampersan_position,arg_2,system_commands);
                if(parallel_error>0){
                    wish_launch_parallel(*n,ampersan_position,arg_2);
                }else{
                    write(STDERR_FILENO, error_message, strlen(error_message));
                }
            }else{
                write(STDERR_FILENO, error_message, strlen(error_message));
            }
        }
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


int parallel_paths(int n, int ampersan_position[n] , char **arg_2, char **system_commands){
    int aux=0;
    int band=0;
    for(int i =0;i<=n;i++){
        band=0;
        int j=0;
        char *path1 = malloc(MAX_SIZE * sizeof(char*));
        do{
            strcpy(path1,"");
            strcat(path1,system_commands[j]);
            strcat(path1,"/");
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

int absolute_path(char **arg_2,char **system_commands){
    int i=0;
    char *path1 = malloc(MAX_SIZE * sizeof(char*));
    do{
        strcpy(path1,"");
        strcat(path1,system_commands[i]);
        strcat(path1,"/");
        strcat(path1,arg_2[0]);
        if (access( path1 , X_OK) == 0 ){
            arg_2[0] = path1;
            return 1;
        }
        i++;
    }while (system_commands[i]!=NULL);

    return 0;
}

int command_cd(char **args){
    if (args[1] == NULL || args[2] != NULL)  {
        //fprintf(stderr, "wish: expected argument to \"cd\"\n");
        write(STDERR_FILENO, error_message, strlen(error_message));
    } else {
        if (chdir(args[1]) != 0) {
            //perror("wish");
            write(STDERR_FILENO, error_message, strlen(error_message));
        }
    }
    return 1;
}

void command_exit(char **args){
    if (args[1] != NULL)  {
        //fprintf(stderr, "wish: not expected argument to \"exit\"\n");
        write(STDERR_FILENO, error_message, strlen(error_message));
    } else {
        exit(0);
    }  
}


void command_path(char **args, char**system_commands){
  int i=0;
  if (args[1] == NULL)  {
    /*while (system_commands[i]!=NULL){
      printf("%s\n", system_commands[i]);
      i++;
    }*/
    while(system_commands[i]!=NULL)
    {
        system_commands[i]=NULL;
        free(system_commands[i]);
    }
    free(system_commands);
    system_commands = malloc(2 * sizeof(*system_commands)); 
    for (int k = 0; k < 2; k++){
        system_commands[k] = malloc((20) * sizeof(*system_commands[k]));
    }
    strcpy(system_commands[0],"./");
    system_commands[1]=NULL;
    
  } else {
      int j=0;
    while(system_commands[j]!=NULL)
    {
        system_commands[j]=NULL;
        free(system_commands[j]);
    }
    free(system_commands);
    int count =0;
    do{
        count++;
        i++;
    }while (args[i]!=NULL);

    
    system_commands = malloc(count * sizeof(*system_commands)); 
    for (int k = 0; k < count; k++){
        system_commands[k] = malloc((20) * sizeof(*system_commands[k]));
    }
    strcpy(system_commands[0],"./");
    
    i=1;
    do{
      strcpy(system_commands[i],args[i]);
      i++;
    }while (args[i]!=NULL);
    system_commands[i]=NULL;
    
    
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
    for(int i=0;i<=n;i++){
        pids[i]=0;
    }
    int aux=0;
    for(int i = 0; i<=n; i++){
        pid_t pid = fork();
            if (pid == 0) {
            char ** arg = malloc(MAX_SIZE * sizeof(char*));
            int p=0;
            while(strcmp(arg_2[aux],"&")!=0 && arg_2[aux]!=NULL){
                arg[p]=arg_2[aux];
                printf("p value:%d  arg :%s aux value%d\n",p,arg[p],aux);
                p+=1;
                aux+=1;
            }
            arg[p]=NULL;
            printf("comando %s", arg[0]);
            printf("comando en p+1 %s", arg[p+1]);
            printf("llego a execv");
            //int x=0;
            /*while(arg[x]!=NULL){
                    printf("arg %d : %s\n",x,arg[x]);
                    x++;
            }*/
            sleep(1);
            
            execv(arg[0], arg);
            //printf("error execv %d\n", test);
            exit( 0 );
            /*for(int x=0; x<p;x++){
            printf("%s\n",arg[x]);
            }*/
            
            // Child process
            
        } else if (pid < 0) {
            // Error forking
            printf("pid<0");
            //perror("wish");
        } else {
            // Parent process
            //wait(NULL);

            //printf("en padre asigna nuevo pid en pids %d\n",i);
            pids[i]=pid;
            //printf("nuevo hijo %d\n",pids[i]);
         
        }
        if(i<n){
            aux=ampersan_position[i]+1;
        }/*else{
            aux+=1;
        }*/
        if(i==n){
            //printf("aux: %d",aux);
        }
        
    }
    for(int i=0;i<(n+1);i++){
        printf("esperando el hijo : %d\n",pids[i]);
        waitpid(pids[i],&status ,0);
        printf("termino el hijo : %d\n",pids[i]);
    }
    //printf("esta en padre\n");
   
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
        
    } else {
        // Parent process
       wait(NULL);
    }
    return 1;
}